/*
 * ============================================================
 *  SERVIDOR WEB - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "servidor_web.h"
#include "config.h"
#include "red.h"
#include "bascula.h"
#include "ajustes.h"
#include "contador.h"
#include "paginas.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <lwip/sockets.h>
#include "esp_ota_ops.h"

/* El core de Arduino daria por bueno un firmware nuevo en cuanto arranca.
 * Con esto (lo consulta el core antes de setup) la decision queda para
 * web::confirmarFirmware(): si el firmware nuevo no llega hasta ahi, el
 * arranque regresa solo al anterior. */
extern "C" bool verifyRollbackLater() { return true; }

namespace {

WebServer servidor(80);
bool      rutasListas = false;
bool      corriendo   = false;

// ---- actualizacion de firmware ----
EstadoOta ota          = OTA_CERRADA;
bool      recibiendo   = false;    // la subida en curso si se esta escribiendo en la Flash
uint32_t  otaHastaMs   = 0;        // cuando se cierra el permiso
uint32_t  otaTotal     = 0, otaRecibidos = 0;
uint8_t   otaPorciento = 0;
char      otaError[40] = "";
uint32_t  reinicioMs   = 0;        // reinicio pendiente tras actualizar (0 = no hay)
void    (*avisoAvance)() = nullptr;

// ---- calibracion desde la web ----
bool      calAbierta = false;      // permiso abierto desde el menu de la bascula
uint32_t  calHastaMs = 0;
uint8_t   calPaso    = 0;          // 0 = falta el cero; 1 = cero tomado, falta el peso
int32_t   calCero    = 0;          // cuentas con el plato vacio

// ---- peso en vivo ----
/* Cada navegador que muestra el peso deja abierta una conexion
 * (Server-Sent Events) y la bascula escribe en ella cuando hay algo
 * nuevo. Es mas ligero que preguntar cinco veces por segundo. */
struct Oyente {
  NetworkClient cliente;
  bool          activo = false;
};
Oyente   oyentes[WEB_MAX_OYENTES];
uint8_t  turnoOyente  = 0;         // a quien reemplazar si llega uno mas y no hay lugar
uint32_t ultimoPesoMs = 0, ultimoEnvioMs = 0;
char     pesoEnviado[560] = "";

// La Flash tiene DOS particiones de programa. Con una sola (Huge APP),
// "la siguiente" es la misma que esta corriendo: escribir ahi borraria el
// programa en uso.
bool hayLugarOta() {
  const esp_partition_t* siguiente = esp_ota_get_next_update_partition(nullptr);
  return siguiente && siguiente != esp_ota_get_running_partition();
}

/* El portal y la pagina de la bascula comparten direccion "/": se
 * distinguen por donde llego el navegador. Si entro por la red propia de
 * la bascula, esta configurando el WiFi. */
bool vienePorPortal() {
  return (WiFi.getMode() & WIFI_MODE_AP) && servidor.client().localIP() == WiFi.softAPIP();
}

/* Un SSID o el nombre de una pieza pueden traer comillas o diagonales: se
 * escapan para que el JSON no se rompa (ni se cuele nada raro a la pagina). */
String escaparJson(const char* s) {
  String r;
  for (; *s; s++) {
    if (*s == '"' || *s == '\\') { r += '\\'; r += *s; }
    else if ((uint8_t)*s < 0x20) r += ' ';
    else r += *s;
  }
  return r;
}

const char* siNo(bool v) { return v ? "true" : "false"; }

uint32_t calRestanteMs() {
  if (!calAbierta) return 0;
  int32_t restante = (int32_t)(calHastaMs - millis());
  return restante > 0 ? restante : 0;
}

uint32_t calPesoMinimoG() {
  return (uint32_t)ceilf(ajustes::actual().capacidadKg * 1000.0f * CAL_PESO_MIN_PORCIENTO / 100.0f);
}

// ------------------ paginas ------------------

void rutaRaiz() {
  servidor.sendHeader("Cache-Control", "no-store");
  if (vienePorPortal()) {
    red::actividad();
    servidor.send_P(200, "text/html", PAGINA_PORTAL);
  } else {
    servidor.send_P(200, "text/html", PAGINA_BASCULA);
  }
}

// La misma pagina de la bascula, tambien desde la red del portal: asi se
// puede usar desde el telefono aunque no haya un WiFi al cual conectarla.
void rutaBascula() {
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send_P(200, "text/html", PAGINA_BASCULA);
}

void rutaEstilo() {
  servidor.sendHeader("Cache-Control", "no-cache");
  servidor.send_P(200, "text/css", PAGINA_ESTILO);
}

void rutaActualizar() {
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send_P(200, "text/html", PAGINA_ACTUALIZAR);
}

/* Todo lo que no sea del portal se redirige a la raiz. Asi es como
 * Android (generate_204), iOS (hotspot-detect.html) y Windows
 * (connecttest.txt) detectan que hay portal y lo abren solos. */
void rutaDesconocida() {
  if (vienePorPortal()) {
    servidor.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    servidor.send(302, "text/plain", "");
  } else {
    servidor.send(404, "text/plain", "No existe");
  }
}

// ------------------ portal ------------------

/* Escaneo asincrono: nunca bloquea. La pagina pregunta cada segundo
 * hasta que el estado sea "listo". */
void rutaRedes() {
  if (!vienePorPortal()) { servidor.send(404, "text/plain", "No existe"); return; }
  red::actividad();

  int16_t n = WiFi.scanComplete();
  if (n >= 0 && servidor.hasArg("nuevo")) {   // resultados viejos: fuera
    WiFi.scanDelete();
    n = WIFI_SCAN_FAILED;
  }
  if (n == WIFI_SCAN_FAILED && red::info().estado != RED_CONECTANDO) {
    WiFi.scanNetworks(true);
    n = WIFI_SCAN_RUNNING;
  }
  if (n < 0) {
    servidor.send(200, "application/json", "{\"estado\":\"escaneando\"}");
    return;
  }

  String json = "{\"estado\":\"listo\",\"redes\":[";
  for (int16_t i = 0; i < n; i++) {
    if (i) json += ',';
    json += "{\"ssid\":\"" + escaparJson(WiFi.SSID(i).c_str()) + "\"";
    json += ",\"rssi\":" + String(WiFi.RSSI(i));
    json += ",\"segura\":";
    json += siNo(WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    json += ",\"banda5\":";
    json += siNo(WiFi.channel(i) > 14);
    json += '}';
  }
  json += "]}";
  WiFi.scanDelete();
  servidor.send(200, "application/json", json);
}

void rutaConectar() {
  if (red::probar(servidor.arg("ssid").c_str(), servidor.arg("clave").c_str())) {
    servidor.send(200, "application/json", "{\"estado\":\"conectando\"}");
  } else {
    servidor.send(400, "application/json", "{\"error\":\"datos invalidos\"}");
  }
}

// ------------------ estado de la conexion y del firmware ------------------

void rutaEstado() {
  static const char* const REDES[] = { "apagada", "portal", "conectando", "conectada" };
  static const char* const OTAS[]  = { "cerrada", "abierta", "recibiendo", "lista", "error" };

  InfoRed       r = red::info();
  Actualizacion a = web::actualizacion();

  if (vienePorPortal()) {
    red::actividad();
    if (r.estado == RED_CONECTADA) red::ipVista();   // el telefono ya leyo la IP
  }

  String json;
  json.reserve(480);
  json += "{\"placa\":\"" NOMBRE_PLACA "\",\"version\":\"" PROYECTO_VERSION "\"";
  json += ",\"ap\":\"" RED_AP_SSID "\",\"mdns\":\"" RED_NOMBRE_MDNS "\"";
  json += ",\"red\":\"";    json += REDES[r.estado];  json += '"';
  json += ",\"fallo\":";    json += siNo(r.fallo);
  json += ",\"guardada\":"; json += siNo(r.hayGuardada);
  json += ",\"ssid\":\"" + escaparJson(r.ssid) + "\"";
  if (r.estado == RED_CONECTADA) {
    json += ",\"ip\":\"";       json += r.ip;        json += '"';
    json += ",\"rssi\":";       json += String(r.rssi);
    json += ",\"canal\":";      json += String(r.canal);
    json += ",\"banda\":\"";    json += r.banda5 ? "5 GHz" : "2.4 GHz"; json += '"';
    json += ",\"estandar\":\""; json += r.estandar;  json += '"';
    json += ",\"norma\":\"";    json += r.norma;     json += '"';
  }
  json += ",\"ota\":\"";        json += OTAS[a.estado]; json += '"';
  json += ",\"otaLugar\":";     json += siNo(a.hayLugar);
  json += ",\"otaSegundos\":";  json += String(a.restanteMs / 1000);
  json += ",\"otaError\":\"" + escaparJson(a.error) + "\"";
  json += '}';

  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", json);
}

void rutaOlvidar() {
  servidor.send(200, "text/plain", "ok");
  red::olvidarEnBreve();    // despues, para que la respuesta alcance a salir
}

// ------------------ peso en vivo ------------------

/* Todo lo que la pagina necesita para pintarse, en un solo JSON: el peso,
 * el conteo de piezas y como va la calibracion desde la web. Los pesos van
 * en gramos; la pagina los pasa a la unidad elegida. */
size_t armarPeso(char* destino, size_t tam) {
  Peso   p = bascula::peso();
  Conteo c = contador::contar(p);

  int n = snprintf(destino, tam,
    "{\"celda\":%s,\"cal\":%s,\"estable\":%s,\"sobre\":%s,\"conTara\":%s,"
    "\"neto\":%.2f,\"tara\":%.2f,\"bruto\":%.2f,\"div\":%.1f,\"cap\":%.0f,\"unidad\":%u,"
    "\"pz\":{\"activo\":%s,\"unit\":%.4f,\"n\":%ld,\"exacto\":%.2f,\"ambiguo\":%s,\"confiable\":%s,\"perfil\":%d,\"nombre\":\"%s\"},"
    "\"calWeb\":{\"seg\":%lu,\"paso\":%u,\"min\":%lu}}",
    siNo(p.hayCelda), siNo(p.calibrada), siNo(p.estable), siNo(p.sobrecarga), siNo(p.conTara),
    p.netoG, p.taraG, p.brutoG, p.divisionG, p.capacidadG, (unsigned)ajustes::actual().unidad,
    siNo(c.activo), c.unitarioG, (long)c.piezas, c.exacto, siNo(c.ambiguo), siNo(c.confiable), c.perfil,
    escaparJson(c.nombre).c_str(),
    (unsigned long)(calRestanteMs() / 1000), calPaso, (unsigned long)calPesoMinimoG());
  return (n > 0 && (size_t)n < tam) ? (size_t)n : 0;
}

/* Escribe sin esperar. Si no cabe completo (el navegador se durmio o la
 * conexion se cayo) se suelta al oyente: el navegador reconecta solo. Asi
 * un telefono dormido nunca detiene a loop(). */
void mandar(Oyente& o, const char* datos, size_t n) {
  if (::send(o.cliente.fd(), datos, n, MSG_DONTWAIT) == (int)n) return;
  o.cliente.stop();
  o.activo = false;
}

void mandarPeso(Oyente& o, const char* json) {
  char evento[sizeof(pesoEnviado) + 16];
  int n = snprintf(evento, sizeof(evento), "data: %s\n\n", json);
  mandar(o, evento, n);
}

void rutaEventos() {
  // Lugar libre, o el de un oyente que ya se fue; si no hay, el mas viejo.
  int lugar = -1;
  for (uint8_t i = 0; i < WEB_MAX_OYENTES && lugar < 0; i++) {
    if (!oyentes[i].activo || !oyentes[i].cliente.connected()) lugar = i;
  }
  if (lugar < 0) {
    lugar = turnoOyente;
    turnoOyente = (turnoOyente + 1) % WEB_MAX_OYENTES;
  }
  if (oyentes[lugar].activo) oyentes[lugar].cliente.stop();

  // Una copia del cliente: el servidor suelta la suya al salir de aqui,
  // pero la conexion sigue viva mientras exista esta.
  Oyente& o = oyentes[lugar];
  o.cliente = servidor.client();
  o.activo  = true;
  o.cliente.setNoDelay(true);

  static const char CABECERA[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/event-stream\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: keep-alive\r\n\r\n"
    "retry: 2000\n\n";
  mandar(o, CABECERA, sizeof(CABECERA) - 1);

  char json[sizeof(pesoEnviado)];
  if (o.activo && armarPeso(json, sizeof(json))) mandarPeso(o, json);
}

void atenderOyentes() {
  uint32_t ahora = millis();
  if (ahora - ultimoPesoMs < WEB_PESO_MS) return;
  ultimoPesoMs = ahora;

  bool hay = false;
  for (Oyente& o : oyentes) {
    if (o.activo && !o.cliente.connected()) { o.cliente.stop(); o.activo = false; }
    hay |= o.activo;
  }
  if (!hay) return;

  char json[sizeof(pesoEnviado)];
  if (!armarPeso(json, sizeof(json))) return;
  // Solo cuando algo cambio; y una vez por segundo aunque no, para que el
  // navegador sepa que la bascula sigue ahi.
  if (strcmp(json, pesoEnviado) == 0 && ahora - ultimoEnvioMs < 1000) return;
  strlcpy(pesoEnviado, json, sizeof(pesoEnviado));
  ultimoEnvioMs = ahora;

  for (Oyente& o : oyentes) if (o.activo) mandarPeso(o, json);
}

void soltarOyentes() {
  for (Oyente& o : oyentes) {
    if (o.activo) o.cliente.stop();
    o.activo = false;
  }
}

void rutaPeso() {   // el mismo JSON, una sola vez
  char json[sizeof(pesoEnviado)];
  armarPeso(json, sizeof(json));
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", json);
}

// ------------------ ordenes desde la pagina ------------------

/* Todas responden {"ok":true} o {"ok":false,"motivo":"..."}. El motivo
 * "inestable" significa "intenta otra vez en un momento": la pagina
 * reintenta sola unos segundos, igual que la bascula espera a que el peso
 * se asiente cuando se presiona una tecla. */
void responder(bool ok, const char* motivo = "") {
  String json = "{\"ok\":";
  json += siNo(ok);
  if (!ok) { json += ",\"motivo\":\""; json += motivo; json += '"'; }
  json += '}';
  servidor.send(ok ? 200 : 409, "application/json", json);
}

// false (y ya respondio) si no hay con que pesar.
bool listaParaPesar(const Peso& p) {
  if (!p.hayCelda)  { responder(false, "sin_celda");    return false; }
  if (!p.calibrada) { responder(false, "sin_calibrar"); return false; }
  return true;
}

void rutaTara() {
  if (!listaParaPesar(bascula::peso())) return;
  responder(bascula::tarar(), "inestable");
}

void rutaCero() {
  if (!listaParaPesar(bascula::peso())) return;
  responder(bascula::cero(), "inestable");
}

void rutaUnidad() {
  long u = servidor.arg("u").toInt();
  if (u < 0 || u >= NUM_UNIDADES) { responder(false, "dato_invalido"); return; }
  ajustes::actual().unidad = (Unidad)u;
  ajustes::guardar();
  responder(true);
}

// ---- contador de piezas ----

void rutaPiezasMuestra() {
  Peso p = bascula::peso();
  long n = servidor.arg("n").toInt();
  if (!listaParaPesar(p)) return;
  if (n < 1 || n > 9999) { responder(false, "dato_invalido"); return; }
  if (!p.estable)        { responder(false, "inestable");     return; }

  Muestra m = contador::tomarMuestra(p.netoG, n, p.divisionG);
  if (m.calidad == MUESTRA_INVALIDA) { responder(false, "sin_peso"); return; }

  static const char* const CALIDADES[] = { "bien", "chica", "ligera" };
  char json[120];
  snprintf(json, sizeof(json), "{\"ok\":true,\"calidad\":\"%s\",\"unit\":%.4f,\"sugeridas\":%lu}",
           CALIDADES[m.calidad], m.unitarioG, (unsigned long)m.sugeridas);
  servidor.send(200, "application/json", json);
}

void rutaPiezasQuitar() {
  contador::quitar();
  responder(true);
}

void rutaPiezasLista() {
  String json = "{\"max\":" + String(PIEZAS_MAX_PERFILES) + ",\"lista\":[";
  for (uint8_t i = 0; i < contador::perfiles(); i++) {
    const PerfilPieza& pieza = contador::perfil(i);
    if (i) json += ',';
    json += "{\"nombre\":\"" + escaparJson(pieza.nombre) + "\",\"unit\":" + String(pieza.unitarioG, 4) + "}";
  }
  json += "]}";
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", json);
}

// El indice 'i' del formulario, o -1 si no es de un perfil que exista.
int indicePerfil() {
  if (!servidor.hasArg("i")) return -1;
  long i = servidor.arg("i").toInt();
  return (i >= 0 && i < contador::perfiles()) ? (int)i : -1;
}

void rutaPiezasUsar() {
  int i = indicePerfil();
  responder(i >= 0 && contador::usar(i), "dato_invalido");
}

void rutaPiezasGuardar() {
  if (!contador::contar(bascula::peso()).activo) { responder(false, "sin_muestra"); return; }
  responder(contador::guardar(servidor.arg("nombre").c_str()) >= 0, "lleno");
}

void rutaPiezasRenombrar() {
  int i = indicePerfil();
  responder(i >= 0 && contador::renombrar(i, servidor.arg("nombre").c_str()), "dato_invalido");
}

void rutaPiezasBorrar() {
  int i = indicePerfil();
  responder(i >= 0 && contador::borrar(i), "dato_invalido");
}

// ---- calibracion (misma cuenta que el asistente de la pantalla) ----

// false (y ya respondio) si no se puede capturar ahora.
bool listaParaCalibrar(Lectura& l) {
  if (!calRestanteMs()) { responder(false, "sin_permiso"); return false; }
  l = bascula::leer();
  if (l.estado == HX_SIN_RESPUESTA || l.doutAlAire) { responder(false, "sin_celda"); return false; }
  if (!l.estable) { responder(false, "inestable"); return false; }
  return true;
}

void rutaCalibrarCero() {
  Lectura l;
  if (!listaParaCalibrar(l)) return;
  calCero = l.filtrado;
  calPaso = 1;
  responder(true);
}

void rutaCalibrarCarga() {
  Lectura  l;
  Ajustes& a = ajustes::actual();
  long     gramos = servidor.arg("gramos").toInt();

  if (!listaParaCalibrar(l)) return;
  if (calPaso != 1) { responder(false, "sin_cero"); return; }
  if (gramos < (long)calPesoMinimoG() || gramos > a.capacidadKg * 1000L) { responder(false, "peso_invalido"); return; }
  if (abs(l.filtrado - calCero) < CAL_CUENTAS_MINIMAS) { responder(false, "poco_cambio"); return; }

  a.cal.factor = (float)(l.filtrado - calCero) / gramos;
  a.cal.offset = calCero;
  a.cal.valida = true;
  a.pesoCalG   = gramos;
  ajustes::guardar();
  bascula::configurar(a.cal, a.capacidadKg);

  Serial.printf("[calibracion web] cero=%ld  carga=%ld  peso=%ld g  factor=%.3f cuentas/g\n",
                (long)calCero, (long)l.filtrado, gramos, a.cal.factor);
  calPaso    = 0;
  calAbierta = false;      // un permiso, una calibracion
  responder(true);
}

// ------------------ actualizacion de firmware ------------------

void avisar() {
  if (avisoAvance) avisoAvance();
}

// El motivo en corto: tiene que caber en la pantalla redonda.
const char* motivoDelFallo() {
  switch (Update.getError()) {
    case UPDATE_ERROR_MAGIC_BYTE:   return "No es un firmware válido";
    case UPDATE_ERROR_SPACE:
    case UPDATE_ERROR_SIZE:         return "No cabe en la memoria";
    case UPDATE_ERROR_NO_PARTITION: return "Placa sin lugar para OTA";
    case UPDATE_ERROR_ACTIVATE:     return "Dañado o de otro chip";
    default:                        return "Falló la escritura";
  }
}

void falloOta(const char* motivo) {
  Serial.printf("[ota] %s (%s)\n", motivo, Update.errorString());
  recibiendo = false;
  if (Update.isRunning()) Update.abort();
  strlcpy(otaError, motivo, sizeof(otaError));
  ota        = OTA_ERROR;
  otaHastaMs = millis() + OTA_VENTANA_MS;   // el permiso se renueva: se puede reintentar
  avisar();
}

/* Se llama muchas veces por archivo: al empezar, con cada pedazo (unos
 * 1400 bytes) y al terminar. Cada pedazo va directo a la Flash: el
 * firmware nunca esta completo en la RAM. */
void rutaSubida() {
  HTTPUpload& subida = servidor.upload();

  switch (subida.status) {
    case UPLOAD_FILE_START:
      recibiendo = false;
      if (ota != OTA_ABIERTA && ota != OTA_ERROR) return;   // sin permiso: el archivo se ignora completo
      soltarOyentes();                                      // nada mas compite por el radio
      otaTotal     = servidor.clientContentLength();        // el archivo mas unos bytes del formulario
      otaRecibidos = 0;
      otaPorciento = 0;
      otaError[0]  = 0;
      Serial.printf("[ota] Llega \"%s\" (%lu bytes)\n", subida.filename.c_str(), (unsigned long)otaTotal);
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { falloOta(motivoDelFallo()); return; }
      recibiendo = true;
      ota        = OTA_RECIBIENDO;
      avisar();
      break;

    case UPLOAD_FILE_WRITE: {
      if (!recibiendo) return;
      if (Update.write(subida.buf, subida.currentSize) != subida.currentSize) { falloOta(motivoDelFallo()); return; }
      otaRecibidos += subida.currentSize;
      uint8_t porciento = otaTotal ? (uint8_t)min<uint64_t>(99, (uint64_t)otaRecibidos * 100 / otaTotal) : 0;
      if (porciento != otaPorciento) { otaPorciento = porciento; avisar(); }
      break;
    }

    case UPLOAD_FILE_END:
      if (!recibiendo) return;
      recibiendo = false;
      // end() verifica la imagen completa y, solo si esta bien, la deja
      // como la que arranca en el siguiente reinicio.
      if (!Update.end(true)) { falloOta(motivoDelFallo()); return; }
      otaPorciento = 100;
      ota          = OTA_LISTA;
      reinicioMs   = (millis() + 1500) | 1;
      Serial.println("[ota] Firmware recibido y verificado.");
      avisar();
      break;

    case UPLOAD_FILE_ABORTED:
      if (recibiendo) falloOta("Se cortó la subida");
      break;
  }
}

// Se llama una vez, cuando el archivo ya termino de llegar.
void rutaSubidaFin() {
  servidor.sendHeader("Connection", "close");
  if (ota == OTA_LISTA)      servidor.send(200, "text/plain", "ok");
  else if (ota == OTA_ERROR) servidor.send(500, "text/plain; charset=utf-8", otaError);
  else                       servidor.send(403, "text/plain; charset=utf-8", "Primero permite la actualización en la báscula: MENÚ > Conexión > Actualizar.");
}

}  // namespace

namespace web {

void arrancar() {
  if (!rutasListas) {
    servidor.on("/", rutaRaiz);
    servidor.on("/bascula", rutaBascula);
    servidor.on("/estilo.css", rutaEstilo);
    servidor.on("/estado", rutaEstado);

    // portal
    servidor.on("/redes", rutaRedes);
    servidor.on("/conectar", HTTP_POST, rutaConectar);
    servidor.on("/olvidar", HTTP_POST, rutaOlvidar);

    // bascula
    servidor.on("/eventos", rutaEventos);
    servidor.on("/peso", rutaPeso);
    servidor.on("/tara", HTTP_POST, rutaTara);
    servidor.on("/cero", HTTP_POST, rutaCero);
    servidor.on("/unidad", HTTP_POST, rutaUnidad);

    // contador de piezas
    servidor.on("/piezas", HTTP_GET, rutaPiezasLista);
    servidor.on("/piezas/muestra", HTTP_POST, rutaPiezasMuestra);
    servidor.on("/piezas/quitar", HTTP_POST, rutaPiezasQuitar);
    servidor.on("/piezas/usar", HTTP_POST, rutaPiezasUsar);
    servidor.on("/piezas/guardar", HTTP_POST, rutaPiezasGuardar);
    servidor.on("/piezas/renombrar", HTTP_POST, rutaPiezasRenombrar);
    servidor.on("/piezas/borrar", HTTP_POST, rutaPiezasBorrar);

    // calibracion y firmware (los dos piden permiso desde la bascula)
    servidor.on("/calibrar/cero", HTTP_POST, rutaCalibrarCero);
    servidor.on("/calibrar/carga", HTTP_POST, rutaCalibrarCarga);
    servidor.on("/actualizar", HTTP_GET, rutaActualizar);
    servidor.on("/actualizar", HTTP_POST, rutaSubidaFin, rutaSubida);

    servidor.onNotFound(rutaDesconocida);
    servidor.enableDelay(false);   // loop() ya cede el CPU por su cuenta
    rutasListas = true;
  }
  servidor.begin();
  corriendo = true;
}

void detener() {
  if (!corriendo) return;
  soltarOyentes();
  servidor.stop();
  corriendo  = false;
  calAbierta = false;
  if (ota != OTA_LISTA) ota = OTA_CERRADA;
}

void atender() {
  if (!corriendo) return;
  servidor.handleClient();
  atenderOyentes();

  uint32_t ahora = millis();

  if (reinicioMs && (int32_t)(ahora - reinicioMs) >= 0) {
    Serial.println("[ota] Reiniciando con el firmware nuevo.");
    delay(100);
    ESP.restart();
  }
  if ((ota == OTA_ABIERTA || ota == OTA_ERROR) && (int32_t)(ahora - otaHastaMs) >= 0) ota = OTA_CERRADA;
  if (calAbierta && !calRestanteMs()) { calAbierta = false; calPaso = 0; }
}

void permitirActualizacion(bool si) {
  if (ota == OTA_RECIBIENDO || ota == OTA_LISTA) return;
  if (si && corriendo && hayLugarOta()) {
    ota         = OTA_ABIERTA;
    otaHastaMs  = millis() + OTA_VENTANA_MS;
    otaError[0] = 0;
  } else {
    ota = OTA_CERRADA;
  }
}

Actualizacion actualizacion() {
  Actualizacion a;
  a.estado    = ota;
  a.hayLugar  = hayLugarOta();
  a.porciento = otaPorciento;
  strlcpy(a.error, otaError, sizeof(a.error));
  if (ota == OTA_ABIERTA || ota == OTA_ERROR) {
    int32_t restante = (int32_t)(otaHastaMs - millis());
    a.restanteMs = restante > 0 ? restante : 0;
  }
  return a;
}

void permitirCalibracion(bool si) {
  calAbierta = si && corriendo;
  calHastaMs = millis() + CAL_WEB_VENTANA_MS;
  calPaso    = 0;
}

void confirmarFirmware() {
  esp_ota_mark_app_valid_cancel_rollback();
}

void alAvanzar(void (*aviso)()) { avisoAvance = aviso; }

}  // namespace web

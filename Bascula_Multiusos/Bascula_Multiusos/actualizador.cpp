/*
 * ============================================================
 *  ACTUALIZADOR - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "actualizador.h"
#include "config.h"
#include "red.h"
#include "servidor_web.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "esp_ota_ops.h"
#include "mbedtls/sha256.h"

namespace {

enum Trabajo : uint8_t { T_BUSCAR, T_INSTALAR };

// Lo escribe la tarea; loop() y la interfaz solo lo leen.
volatile EstadoActualizador estado    = ACT_INACTIVA;
volatile uint8_t            porciento = 0;
volatile bool               trabajando = false;   // la tarea existe y esta corriendo
volatile uint32_t           reinicioMs = 0;       // reinicio pendiente (0 = no hay)

Trabajo  trabajo;
bool     buscadaEsteArranque = false;
bool     descartada          = false;

// Lo que anuncia el servidor para esta placa.
char     versionServidor[16] = "";
char     archivo[64]         = "";
char     sha256Anunciado[65] = "";
char     notas[48]           = "";
uint32_t tamano              = 0;
char     error[40]           = "";

const size_t PILA_TAREA = 10240;   // bytes: HTTPClient + ArduinoJson + Update

bool hayLugarOta() {
  const esp_partition_t* siguiente = esp_ota_get_next_update_partition(nullptr);
  return siguiente && siguiente != esp_ota_get_running_partition();
}

void fallo(const char* motivo) {
  Serial.printf("[actualizador] %s\n", motivo);
  strlcpy(error, motivo, sizeof(error));
  estado = ACT_ERROR;
}

// Lo que devolvio HTTPClient: negativo = no hubo conexion; positivo = codigo HTTP.
void falloHttp(int codigo) {
  if (codigo <= 0) { fallo("El servidor no responde"); return; }
  char motivo[40];
  snprintf(motivo, sizeof(motivo), "Servidor respondió %d", codigo);
  fallo(motivo);
}

// El motivo en corto: tiene que caber en la pantalla redonda.
const char* motivoDelUpdate() {
  switch (Update.getError()) {
    case UPDATE_ERROR_SPACE:
    case UPDATE_ERROR_SIZE:         return "No cabe en la memoria";
    case UPDATE_ERROR_NO_PARTITION: return "Placa sin lugar para OTA";
    case UPDATE_ERROR_MAGIC_BYTE:   return "No es un firmware válido";
    case UPDATE_ERROR_ACTIVATE:     return "Dañado o de otro chip";
    default:                        return "Falló la escritura";
  }
}

/* "0.5" es mas nueva que "0.4.9"; "1.0" que "0.12". Se compara numero a
 * numero, no letra a letra. Devuelve true si 'a' es mas nueva que 'b'. */
bool esMasNueva(const char* a, const char* b) {
  while (*a || *b) {
    char* finA = nullptr; char* finB = nullptr;
    long na = *a ? strtol(a, &finA, 10) : 0;
    long nb = *b ? strtol(b, &finB, 10) : 0;
    if (na != nb) return na > nb;
    a = *a ? finA : a;
    b = *b ? finB : b;
    if (*a == '.') a++;
    if (*b == '.') b++;
    if ((*a && *a != '.' && !isdigit((unsigned char)*a)) ||
        (*b && *b != '.' && !isdigit((unsigned char)*b))) return false;   // algo raro: no es una version
  }
  return false;
}

bool esHexadecimal(const char* s, size_t largo) {
  if (strlen(s) != largo) return false;
  for (; *s; s++) if (!isxdigit((unsigned char)*s)) return false;
  return true;
}

void prepararHttp(HTTPClient& http, uint32_t esperaMs) {
  http.setConnectTimeout(esperaMs);
  http.setTimeout(esperaMs);
  http.setReuse(false);
  http.setUserAgent("Bascula-Tostatronic/" PROYECTO_VERSION " (" ID_PLACA ")");
}

// ------------------ consultar la lista de versiones ------------------

void consultar() {
  estado = ACT_BUSCANDO;
  versionServidor[0] = archivo[0] = sha256Anunciado[0] = notas[0] = 0;
  tamano = 0;

  WiFiClient cliente;
  HTTPClient http;
  prepararHttp(http, OTA_SERVIDOR_ESPERA_MS);
  Serial.println("[actualizador] Consultando " OTA_SERVIDOR_URL OTA_ARCHIVO_VERSIONES);

  if (!http.begin(cliente, OTA_SERVIDOR_URL OTA_ARCHIVO_VERSIONES)) { fallo("Dirección no válida"); return; }
  int codigo = http.GET();
  if (codigo != HTTP_CODE_OK) { http.end(); falloHttp(codigo); return; }

  // Solo se guarda la entrada de esta placa: la lista trae las cuatro.
  JsonDocument filtro;
  filtro[ID_PLACA] = true;
  JsonDocument doc;
  DeserializationError e = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filtro));
  http.end();

  if (e) { fallo("Lista de versiones dañada"); return; }
  JsonObjectConst placa = doc[ID_PLACA].as<JsonObjectConst>();
  if (placa.isNull()) { fallo("Sin firmware para esta placa"); return; }

  strlcpy(versionServidor, placa["version"] | "", sizeof(versionServidor));
  strlcpy(archivo,         placa["archivo"] | "", sizeof(archivo));
  strlcpy(sha256Anunciado, placa["sha256"]  | "", sizeof(sha256Anunciado));
  strlcpy(notas,           placa["notas"]   | "", sizeof(notas));
  tamano = placa["tamano"] | 0UL;

  if (!versionServidor[0] || !archivo[0] || !tamano || !esHexadecimal(sha256Anunciado, 64) || strchr(archivo, '/')) {
    fallo("Lista de versiones incompleta");
    return;
  }

  if (!esMasNueva(versionServidor, PROYECTO_VERSION)) {
    Serial.printf("[actualizador] Al día: servidor v%s, instalada v" PROYECTO_VERSION "\n", versionServidor);
    estado = ACT_AL_DIA;
    return;
  }
  Serial.printf("[actualizador] Hay versión nueva: v%s (%s, %lu bytes)\n", versionServidor, archivo, (unsigned long)tamano);
  estado = ACT_DISPONIBLE;
}

// ------------------ descargar e instalar ------------------

void abortar(const char* motivo) {
  if (Update.isRunning()) Update.abort();
  fallo(motivo);
}

void descargar() {
  estado    = ACT_DESCARGANDO;
  porciento = 0;

  WiFiClient cliente;
  HTTPClient http;
  prepararHttp(http, OTA_DESCARGA_ESPERA_MS);

  char url[sizeof(OTA_SERVIDOR_URL) + sizeof(archivo)];
  snprintf(url, sizeof(url), OTA_SERVIDOR_URL "%s", archivo);
  Serial.printf("[actualizador] Descargando %s\n", url);

  if (!http.begin(cliente, url)) { fallo("Dirección no válida"); return; }
  int codigo = http.GET();
  if (codigo != HTTP_CODE_OK) { http.end(); falloHttp(codigo); return; }
  if (http.getSize() > 0 && (uint32_t)http.getSize() != tamano) {
    http.end();
    fallo("Tamaño distinto al anunciado");
    return;
  }

  if (!Update.begin(tamano)) { http.end(); fallo(motivoDelUpdate()); return; }

  mbedtls_sha256_context sha;
  mbedtls_sha256_init(&sha);
  mbedtls_sha256_starts(&sha, 0);

  WiFiClient* flujo = http.getStreamPtr();
  uint8_t     bufer[1024];
  uint32_t    recibidos    = 0;
  uint32_t    ultimoDatoMs = millis();

  // Cada pedazo va directo a la Flash: el firmware nunca esta completo en la RAM.
  while (recibidos < tamano) {
    size_t disponibles = flujo->available();
    if (!disponibles) {
      if (!flujo->connected()) break;                                           // el servidor cerro antes de tiempo
      if (millis() - ultimoDatoMs > OTA_DESCARGA_ESPERA_MS) break;              // se quedo callado
      delay(5);
      continue;
    }
    int n = flujo->read(bufer, min(disponibles, sizeof(bufer)));
    if (n <= 0) continue;
    ultimoDatoMs = millis();

    if (Update.write(bufer, n) != (size_t)n) { http.end(); mbedtls_sha256_free(&sha); abortar(motivoDelUpdate()); return; }
    mbedtls_sha256_update(&sha, bufer, n);
    recibidos += n;
    porciento  = (uint8_t)min<uint32_t>(99, recibidos * 100ULL / tamano);
  }
  http.end();

  uint8_t resumen[32];
  mbedtls_sha256_finish(&sha, resumen);
  mbedtls_sha256_free(&sha);

  if (recibidos < tamano) { abortar("Se cortó la descarga"); return; }

  char calculado[65];
  for (int i = 0; i < 32; i++) snprintf(calculado + i * 2, 3, "%02x", resumen[i]);
  if (strcasecmp(calculado, sha256Anunciado) != 0) {
    Serial.printf("[actualizador] SHA-256 esperado %s, llegó %s\n", sha256Anunciado, calculado);
    abortar("No coincide la firma SHA-256");
    return;
  }

  // end() verifica la imagen completa y, solo si esta bien, la deja como
  // la que arranca en el siguiente reinicio.
  if (!Update.end(true)) { fallo(motivoDelUpdate()); return; }

  porciento  = 100;
  estado     = ACT_LISTA;
  reinicioMs = (millis() + 1500) | 1;
  Serial.printf("[actualizador] Firmware v%s instalado y verificado.\n", versionServidor);
}

// ------------------ la tarea ------------------

void tarea(void*) {
  if (trabajo == T_BUSCAR) consultar();
  else                     descargar();
  trabajando = false;
  vTaskDelete(nullptr);
}

void lanzar(Trabajo t) {
  if (trabajando) return;
  trabajo    = t;
  trabajando = true;
  // Misma prioridad que loop(): se reparten el CPU; la celda sigue por encima.
  if (xTaskCreate(tarea, "actualizador", PILA_TAREA, nullptr, 1, nullptr) != pdPASS) {
    trabajando = false;
    fallo("Sin memoria para la tarea");
  }
}

}  // namespace

namespace actualizador {

void iniciar() {
  estado = ACT_INACTIVA;
  buscadaEsteArranque = false;
  descartada = false;
}

void atender() {
  uint32_t ahora = millis();

  if (reinicioMs && (int32_t)(ahora - reinicioMs) >= 0) {
    Serial.println("[actualizador] Reiniciando con el firmware nuevo.");
    delay(100);
    ESP.restart();
  }

  // Una consulta por arranque, en cuanto hay internet.
  if (!buscadaEsteArranque && !trabajando && red::info().estado == RED_CONECTADA) {
    buscadaEsteArranque = true;
    lanzar(T_BUSCAR);
  }
}

void buscar() {
  if (trabajando || estado == ACT_LISTA) return;
  if (red::info().estado != RED_CONECTADA) { fallo("Sin conexión WiFi"); return; }
  buscadaEsteArranque = true;
  descartada = false;
  lanzar(T_BUSCAR);
}

void instalar() {
  if (trabajando || estado == ACT_LISTA) return;
  if (!versionServidor[0] || !archivo[0]) { buscar(); return; }   // no hay que instalar: vuelve a consultar
  if (!hayLugarOta()) { fallo("Placa sin lugar para OTA"); return; }
  if (red::info().estado != RED_CONECTADA) { fallo("Sin conexión WiFi"); return; }
  // Si justo esta llegando un firmware por la pagina web, ese va primero.
  EstadoOta otaWeb = web::actualizacion().estado;
  if (otaWeb == OTA_RECIBIENDO || otaWeb == OTA_LISTA) return;
  descartada = false;
  lanzar(T_INSTALAR);
}

void descartar() {
  if (estado == ACT_DISPONIBLE) {
    descartada = true;
    Serial.println("[actualizador] Actualización pospuesta hasta el siguiente arranque.");
  }
}

bool ocupado() { return trabajando || estado == ACT_LISTA; }

InfoActualizador info() {
  InfoActualizador i;
  i.estado     = estado;
  i.hayLugar   = hayLugarOta();
  i.descartada = descartada;
  i.tamano     = tamano;
  i.porciento  = porciento;
  strlcpy(i.version, versionServidor, sizeof(i.version));
  strlcpy(i.notas, notas, sizeof(i.notas));
  strlcpy(i.error, error, sizeof(i.error));
  return i;
}

}  // namespace actualizador

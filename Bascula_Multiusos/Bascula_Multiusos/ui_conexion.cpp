/*
 * ============================================================
 *  VISTA: CONEXION (WiFi y actualizacion de firmware)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Tres pantallas:
 *
 *    INFO       como va el WiFi. Conectada: la IP, el nombre
 *               tostabascula.local, el ESTANDAR (WiFi 6...) y la
 *               BANDA (2.4 o 5 GHz), que es lo que distingue al
 *               ESP32-C6 y al C5; el anillo es la intensidad de la
 *               senal. Sin red: a que red entrar con el telefono.
 *    OPCIONES   encender / apagar el WiFi, olvidar la red guardada,
 *               actualizar el firmware y permitir que la pagina web
 *               recalibre la bascula (5 minutos).
 *    FIRMWARE   abre por 5 minutos el permiso para recibir un
 *               firmware nuevo desde la pagina web, y muestra el
 *               avance. Al salir de aqui el permiso se cierra.
 *
 *    OK corta ...... INFO: opciones   OPCIONES: elegir
 *    ARRIBA/ABAJO .. moverse en las opciones
 *    MENU corta .... regresa una pantalla (desde INFO, al menu)
 *
 *  Nada de aqui detiene la bascula: la red se conecta, falla o se
 *  olvida mientras la celda se sigue leyendo en su tarea.
 * ============================================================
 */

#include "ui_comun.h"
#include "red.h"
#include "servidor_web.h"
#include "prueba_wifi.h"

namespace ui {
namespace vConexion {

namespace {

enum Pantalla : uint8_t { P_INFO, P_OPCIONES, P_FIRMWARE };
Pantalla pantallaActual = P_INFO;

// Cada pantalla cambia de forma segun el estado (conectada, portal...).
// Aqui se recuerda cual ya esta dibujada, para borrar solo cuando cambia.
int16_t dibujada = -1;

// Aviso que debe salir despues de cambiar de pantalla (el cambio borra todo).
char nota[40] = "";

// Zonas propias de la pantalla INFO (verificadas contra el circulo util).
Zona zIp     = { 120, 108, 196, 30, F_TITULO };
Zona zMdns   = { 120, 134, 190, 16, F_CHICA  };
Zona zEnlace = { 120, 154, 180, 20, F_TEXTO  };

// true si hubo que borrar: toca pintar tambien lo fijo.
bool cambio(uint8_t forma) {
  int16_t clave = pantallaActual * 32 + forma;
  if (clave == dibujada) return false;
  dibujada = clave;
  limpiarVista();
  olvidar(zIp); olvidar(zMdns); olvidar(zEnlace);
  zEnlace.fuente = F_TEXTO;    // pintarAjustado() pudo dejarla con la chica
  return true;
}

void mostrar(Pantalla p) {
  pantallaActual = p;
  dibujada = -1;
}

// ------------------ INFO ------------------

// De -90 dBm (apenas llega) a -50 dBm (excelente).
float calidad(int8_t rssi) { return constrain((rssi + 90) / 40.0f, 0.0f, 1.0f); }

void refrescarInfo() {
  InfoRed r = red::info();
  char    texto[48];

  if (cambio(r.estado)) {
    pintar(zTitulo, "CONEXIÓN", COLOR_AZUL);
    pintar(zPie, "OK: opciones", COLOR_TENUE);
    if (nota[0]) { aviso(nota, COLOR_VERDE, 4000); nota[0] = 0; }
  }

  switch (r.estado) {
    case RED_CONECTADA:
      if (!avisoVigente()) pintarAjustado(zAviso, r.ssid, F_CHICA, F_CHICA, COLOR_TENUE);
      pintarAjustado(zIp, r.ip, F_TITULO, F_TEXTO, COLOR_TEXTO);
      pintar(zMdns, RED_NOMBRE_MDNS ".local", COLOR_TENUE);
      snprintf(texto, sizeof(texto), "%s · %s", r.estandar, r.banda5 ? "5 GHz" : "2.4 GHz");
      pintar(zEnlace, texto, COLOR_AZUL);
      snprintf(texto, sizeof(texto), "señal %d dBm · canal %u", r.rssi, r.canal);
      pintar(zEstado, texto, COLOR_TENUE);
      anillo(calidad(r.rssi), r.rssi > -70 ? COLOR_VERDE : COLOR_AMBAR);
      break;

    case RED_CONECTANDO:
      snprintf(texto, sizeof(texto), "hasta %lu segundos", (unsigned long)(RED_ESPERA_MS / 1000));
      if (!avisoVigente()) pintar(zAviso, texto, COLOR_TENUE);
      pintar(zLinea1, "Conectando a", COLOR_TEXTO);
      pintarAjustado(zLinea2, r.ssid, F_TEXTO, F_CHICA, COLOR_AZUL);
      anillo(r.progreso, COLOR_AMBAR);
      break;

    case RED_PORTAL:
      if (!avisoVigente()) pintar(zAviso, r.fallo ? "No se pudo conectar" : "WiFi sin configurar", r.fallo ? COLOR_NARANJA : COLOR_TENUE);
      pintar(zLinea1, "Con tu teléfono entra", COLOR_TEXTO);
      pintar(zLinea2, "a la red WiFi", COLOR_TEXTO);
      pintarAjustado(zEnlace, RED_AP_SSID, F_TEXTO, F_CHICA, COLOR_AZUL);
      if (r.telefonos) pintar(zEstado, "teléfono conectado", COLOR_VERDE);
      else             pintar(zEstado, "esperando al teléfono…", COLOR_TENUE);
      anillo(r.telefonos ? 1.0f : 0.0f, COLOR_AZUL);
      break;

    default:   // RED_APAGADA
      pintar(zLinea1, "WiFi apagado", COLOR_TEXTO);
      pintar(zDato, "para pesar no hace falta", COLOR_TENUE);
      anillo(0, COLOR_AZUL);
      break;
  }
}

// ------------------ OPCIONES ------------------

enum Opcion : uint8_t { O_WIFI, O_OLVIDAR, O_FIRMWARE, O_CALIBRAR_WEB };

Opcion      opciones[4];
const char* nombres[4];
uint8_t     n = 0, seleccion = 0;
uint32_t    confirmarHastaMs = 0;    // "Olvidar red" pide un segundo OK

void armarOpciones() {
  n = 0;
  opciones[n] = O_WIFI;
  nombres[n++] = red::encendida() ? "Apagar WiFi" : "Encender WiFi";
  if (red::info().hayGuardada) { opciones[n] = O_OLVIDAR;  nombres[n++] = "Olvidar red"; }
  if (red::encendida())        { opciones[n] = O_FIRMWARE; nombres[n++] = "Actualizar"; }
  if (red::encendida())        { opciones[n] = O_CALIBRAR_WEB; nombres[n++] = "Calibrar web"; }
  if (seleccion >= n) seleccion = 0;
}

bool confirmando() {
  if (confirmarHastaMs && (int32_t)(millis() - confirmarHastaMs) >= 0) confirmarHastaMs = 0;
  return confirmarHastaMs != 0;
}

void elegir() {
  switch (opciones[seleccion]) {
    case O_WIFI:
      if (red::encendida()) { pruebaWifi::detener(); red::encender(false); }
      else                  red::encender(true);
      mostrar(P_INFO);
      break;

    case O_OLVIDAR:
      if (!confirmando()) { confirmarHastaMs = (millis() + 3000) | 1; return; }
      confirmarHastaMs = 0;
      red::olvidar();
      mostrar(P_INFO);
      break;

    case O_FIRMWARE:
      web::permitirActualizacion(true);
      mostrar(P_FIRMWARE);
      break;

    case O_CALIBRAR_WEB:
      // A diferencia del firmware, este permiso sigue al salir de aqui:
      // para calibrar hay que ir a poner y quitar pesos.
      web::permitirCalibracion(true);
      strlcpy(nota, "Calibración web: 5 min", sizeof(nota));
      mostrar(P_INFO);
      break;
  }
}

void refrescarOpciones() {
  armarOpciones();
  // Si cambia la cantidad de opciones (p. ej. el telefono acaba de guardar
  // una red), el anillo se reparte distinto: se dibuja todo de nuevo.
  if (cambio(n)) pintar(zTitulo, "CONEXIÓN", COLOR_AZUL);

  lista(nombres, n, seleccion);
  if (confirmando() && opciones[seleccion] == O_OLVIDAR) pintar(zPie, "OK otra vez: olvidar", COLOR_AMBAR);
  else                                                   pintar(zPie, "OK: elegir", COLOR_TENUE);
}

// ------------------ FIRMWARE ------------------

const uint8_t FORMA_SIN_LUGAR = 31;

void refrescarFirmware() {
  Actualizacion a = web::actualizacion();
  InfoRed       r = red::info();
  char          texto[48];

  if (cambio(a.hayLugar ? a.estado : FORMA_SIN_LUGAR)) {
    pintar(zTitulo, "FIRMWARE", COLOR_AZUL);
    pintar(zAviso, "instalado: v" PROYECTO_VERSION, COLOR_TENUE);
  }

  if (!a.hayLugar) {
    pintar(zLinea1, "Esta placa se cargó", COLOR_TEXTO);
    pintar(zLinea2, "sin lugar para OTA", COLOR_TEXTO);
    pintar(zDato, "Partition: Minimal SPIFFS", COLOR_AMBAR);
    pintar(zPie, "MENÚ: regresar", COLOR_TENUE);
    anillo(1.0f, COLOR_AMBAR);
    return;
  }

  uint32_t segundos = a.restanteMs / 1000;

  switch (a.estado) {
    case OTA_ABIERTA:
      pintar(zLinea1, "Abre en el navegador", COLOR_TEXTO);
      pintarAjustado(zLinea2, r.estado == RED_CONECTADA ? r.ip : "192.168.4.1", F_TEXTO, F_CHICA, COLOR_AZUL);
      pintar(zDato, "y elige Actualizar firmware", COLOR_TENUE);
      snprintf(texto, sizeof(texto), "permitido %lu:%02lu", (unsigned long)(segundos / 60), (unsigned long)(segundos % 60));
      pintar(zEstado, texto, COLOR_VERDE);
      pintar(zPie, "MENÚ: cancelar", COLOR_TENUE);
      anillo((float)a.restanteMs / OTA_VENTANA_MS, COLOR_AZUL);
      break;

    case OTA_RECIBIENDO:
      snprintf(texto, sizeof(texto), "%u%%", a.porciento);
      pintarNumero(zNumero, texto, COLOR_TEXTO);
      pintar(zUnidad, "recibiendo…", COLOR_TENUE);
      pintar(zEstado, "no apagues la báscula", COLOR_AMBAR);
      anillo(a.porciento / 100.0f, COLOR_VERDE);
      break;

    case OTA_LISTA:
      pintar(zLinea1, "Firmware recibido", COLOR_VERDE);
      pintar(zLinea2, "Reiniciando…", COLOR_TEXTO);
      anillo(1.0f, COLOR_VERDE);
      break;

    case OTA_ERROR:
      pintar(zLinea1, "No se actualizó", COLOR_NARANJA);
      pintarAjustado(zLinea2, a.error, F_TEXTO, F_CHICA, COLOR_TEXTO);
      pintar(zDato, "nada cambió: reintenta", COLOR_TENUE);
      pintar(zPie, "MENÚ: cancelar", COLOR_TENUE);
      anillo(1.0f, COLOR_NARANJA);
      break;

    default:   // OTA_CERRADA: se acabo el tiempo
      pintar(zLinea1, "Tiempo agotado", COLOR_TEXTO);
      pintar(zDato, "OK: permitir de nuevo", COLOR_TENUE);
      pintar(zPie, "MENÚ: regresar", COLOR_TENUE);
      anillo(0, COLOR_AZUL);
      break;
  }
}

}  // namespace

void entrar() {
  seleccion = 0;
  confirmarHastaMs = 0;
  mostrar(P_INFO);
  refrescar();
}

void salir() {
  web::permitirActualizacion(false);
}

void tecla(const EventoTecla& e) {
  if (e.tipo != PULSACION_CORTA) return;

  switch (pantallaActual) {
    case P_INFO:
      if (e.tecla == TECLA_OK)   mostrar(P_OPCIONES);
      if (e.tecla == TECLA_MENU) irA(V_MENU);
      break;

    case P_OPCIONES:
      armarOpciones();   // la tecla puede llegar antes del primer refresco
      switch (e.tecla) {
        case TECLA_ARRIBA: seleccion = (seleccion + n - 1) % n; confirmarHastaMs = 0; break;
        case TECLA_ABAJO:  seleccion = (seleccion + 1) % n;     confirmarHastaMs = 0; break;
        case TECLA_OK:     elegir();        break;
        case TECLA_MENU:   mostrar(P_INFO); break;
        default: break;
      }
      break;

    case P_FIRMWARE:
      if (e.tecla == TECLA_OK)   web::permitirActualizacion(true);   // renueva el permiso
      if (e.tecla == TECLA_MENU) { web::permitirActualizacion(false); mostrar(P_OPCIONES); }
      break;
  }
}

void refrescar() {
  switch (pantallaActual) {
    case P_INFO:     refrescarInfo();     break;
    case P_OPCIONES: refrescarOpciones(); break;
    case P_FIRMWARE: refrescarFirmware(); break;
  }
}

}  // namespace vConexion
}  // namespace ui

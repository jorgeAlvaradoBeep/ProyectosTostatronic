/*
 * ============================================================
 *  PORTAL - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "portal.h"
#include "config.h"
#include "pagina.h"
#include "pantalla.h"
#include "reproductor.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

// LittleFS aparta bloques para sus tablas: no se le puede llenar hasta el tope.
static const uint32_t RESERVA_FS = 48 * 1024;

static WebServer servidor(80);
static DNSServer dns;
static bool      prendido = false;
static uint32_t  actividad = 0;

static File     fSubida;
static String   errorSubida;
static uint32_t esperado = 0, recibido = 0;
static int      avanceMostrado = -1;

static uint32_t capacidad() {
  uint32_t total = LittleFS.totalBytes();
  return total > RESERVA_FS ? total - RESERVA_FS : 0;
}

static void sinCache() { servidor.sendHeader("Cache-Control", "no-store"); }

static void responder(int codigo, bool ok, const String& error = "") {
  String j = ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"" + error + "\"}";
  sinCache();
  servidor.send(codigo, "application/json", j);
}

// ------------------------------------------------------------
//  Rutas
// ------------------------------------------------------------

static void rutaInicio() {
  actividad = millis();
  sinCache();
  servidor.send_P(200, "text/html; charset=utf-8", PAGINA);
}

static void rutaEstado() {
  actividad = millis();
  InfoVideo v = reproductorInfo();
  char j[320];
  snprintf(j, sizeof(j),
    "{\"fw\":\"%s\",\"video\":{\"hay\":%s,\"bytes\":%u,\"cuadros\":%u,\"fps\":%.2f,\"seg\":%.2f,\"audio\":%s},"
    "\"capacidad\":%u,\"volumen\":%u,\"brillo\":%u,\"brilloOk\":%s,\"pausa\":%s,\"bateria\":%u}",
    VERSION_FW, v.hay ? "true" : "false", (unsigned)v.bytes, v.cuadros, v.fps, v.segundos, v.audio ? "true" : "false",
    (unsigned)capacidad(), ajustes.volumen, ajustes.brillo, PIN_TFT_BL != SIN_PIN ? "true" : "false",
    reproductorEnPausa() ? "true" : "false", bateriaMv());
  sinCache();
  servidor.send(200, "application/json", j);
}

static void rutaAjustes() {
  actividad = millis();
  if (servidor.hasArg("volumen")) ajustes.volumen = constrain(servidor.arg("volumen").toInt(), 0, 100);
  if (servidor.hasArg("brillo"))  ajustes.brillo  = constrain(servidor.arg("brillo").toInt(), 5, 100);
  ajustesAplicar();
  if (servidor.hasArg("guardar")) ajustesGuardar();
  if (servidor.hasArg("pausa"))   reproductorPausa(servidor.arg("pausa").toInt() != 0);
  responder(200, true);
}

static void rutaBorrar() {
  actividad = millis();
  reproductorCerrar();
  LittleFS.remove(RUTA_VIDEO);
  responder(200, true);
}

// Llega el archivo .ltd por partes (multipart). El video anterior se borra antes
// de recibir el nuevo: en 2.5 MB no caben los dos.
static void subidaFalla(const char* motivo) {
  if (errorSubida.length()) return;
  errorSubida = motivo;
  if (fSubida) fSubida.close();
  LittleFS.remove(RUTA_VIDEO);
}

static void rutaSubirTrozo() {
  HTTPUpload& s = servidor.upload();
  actividad = millis();

  if (s.status == UPLOAD_FILE_START) {
    errorSubida = "";
    recibido = 0;
    avanceMostrado = -1;
    esperado = servidor.arg("bytes").toInt();
    reproductorCerrar();
    LittleFS.remove(RUTA_VIDEO);
    if (esperado > capacidad()) { subidaFalla("El archivo no cabe en la memoria"); return; }
    fSubida = LittleFS.open(RUTA_VIDEO, "w");
    if (!fSubida) { subidaFalla("No se pudo crear el archivo"); return; }
    pantallaAvance(0, "Recibiendo");

  } else if (s.status == UPLOAD_FILE_WRITE) {
    if (errorSubida.length()) return;
    if (recibido == 0 && (s.currentSize < 4 || memcmp(s.buf, "LTD1", 4) != 0)) { subidaFalla("El archivo no es .ltd"); return; }
    if (recibido + s.currentSize > capacidad()) { subidaFalla("El archivo no cabe en la memoria"); return; }
    if (fSubida.write(s.buf, s.currentSize) != s.currentSize) { subidaFalla("Memoria llena"); return; }
    recibido += s.currentSize;
    if (esperado) {
      int p = (int)((uint64_t)recibido * 100 / esperado);
      if (p > 100) p = 100;
      if (p != avanceMostrado) { avanceMostrado = p; pantallaAvance(p, "Recibiendo"); }
    }

  } else if (s.status == UPLOAD_FILE_END) {
    if (fSubida) fSubida.close();

  } else if (s.status == UPLOAD_FILE_ABORTED) {
    subidaFalla("Se corto el envio");
  }
}

static void rutaSubirFin() {
  actividad = millis();
  if (!errorSubida.length() && esperado && recibido != esperado) subidaFalla("Llego incompleto");
  if (!errorSubida.length()) {
    const char* e = reproductorAbrir();
    if (e) { errorSubida = e; LittleFS.remove(RUTA_VIDEO); }
  }
  if (errorSubida.length()) responder(400, false, errorSubida);
  else responder(200, true);
}

// Cualquier otra direccion (los celulares prueban varias para saber si hay
// internet) se manda a la pagina.
static void rutaOtra() {
  servidor.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
  servidor.send(302, "text/plain", "");
}

// ------------------------------------------------------------
//  WiFi
// ------------------------------------------------------------

void portalIniciar() {
  servidor.on("/", HTTP_GET, rutaInicio);
  servidor.on("/estado", HTTP_GET, rutaEstado);
  servidor.on("/ajustes", HTTP_POST, rutaAjustes);
  servidor.on("/borrar", HTTP_POST, rutaBorrar);
  servidor.on("/subir", HTTP_POST, rutaSubirFin, rutaSubirTrozo);
  servidor.onNotFound(rutaOtra);
}

void portalWifi(bool prender) {
  if (prender == prendido) return;
  if (prender) {
    WiFi.mode(WIFI_AP);
    const char* clave = strlen(WIFI_CLAVE) >= 8 ? WIFI_CLAVE : nullptr;
    WiFi.softAP(WIFI_NOMBRE, clave, 1, 0, 2);
    // Varias Super Mini traen la antena mal acoplada y a toda potencia ni se
    // ven; a 8.5 dBm conectan bien y de paso gastan menos bateria.
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    dns.start(53, "*", WiFi.softAPIP());
    servidor.begin();
    actividad = millis();
    REGISTRO("[wifi] red \"%s\" en http://%s\n", WIFI_NOMBRE, WiFi.softAPIP().toString().c_str());
  } else {
    servidor.stop();
    dns.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    REGISTRO("[wifi] apagado\n");
  }
  prendido = prender;
}

bool portalWifiPrendido() { return prendido; }

void portalTick() {
  if (!prendido) return;
  dns.processNextRequest();
  servidor.handleClient();
  if (WiFi.softAPgetStationNum() > 0) actividad = millis();
}

uint32_t portalUltimaActividad() { return actividad; }

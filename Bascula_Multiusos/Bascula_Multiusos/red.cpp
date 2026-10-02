/*
 * ============================================================
 *  RED - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "red.h"
#include "config.h"
#include "servidor_web.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "esp_wifi.h"

namespace {

const char*     ESPACIO = "red";               // "carpeta" de la NVS para el WiFi
const IPAddress IP_PORTAL(192, 168, 4, 1);

enum Enlace : uint8_t { SIN_INTENTO, CONECTANDO, CONECTADO, FALLO };

bool      wifiEncendido = true;    // preferencia guardada
bool      radioActivo   = false;
bool      portal        = false;   // la red propia y su DNS estan arriba
Enlace    enlace        = SIN_INTENTO;
bool      intentoNuevo  = false;   // la red en prueba viene del portal: se guarda si conecta
bool      porLanzar     = false;   // hay un intento preparado que aun no llega al radio
EventoRed pendiente     = EVENTO_RED_NINGUNO;

// SSID: hasta 32 caracteres. Clave WPA: de 8 a 63, o 64 digitos hexadecimales.
char ssidGuardado[33] = "", claveGuardada[65] = "";
char ssidIntento[33]  = "", claveIntento[65]  = "";

DNSServer dns;
bool      mdnsActivo = false;

uint32_t inicioIntentoMs = 0;
uint32_t conectadoMs     = 0;
uint32_t ipVistaMs       = 0;    // cuando el telefono leyo la IP (0 = todavia no)
uint32_t actividadMs     = 0;    // ultima vez que alguien uso el portal
uint32_t olvidarMs       = 0;    // olvido pedido desde la web (0 = no hay)

// ------------------ NVS ------------------

void cargar() {
  Preferences nvs;
  nvs.begin(ESPACIO, false);   // false: la crea si es el primer arranque
  wifiEncendido = nvs.getBool("encendido", true);
  if (nvs.isKey("ssid"))  nvs.getString("ssid", ssidGuardado, sizeof(ssidGuardado));
  if (nvs.isKey("clave")) nvs.getString("clave", claveGuardada, sizeof(claveGuardada));
  nvs.end();
}

void guardarRed(const char* ssid, const char* clave) {
  strlcpy(ssidGuardado, ssid, sizeof(ssidGuardado));
  strlcpy(claveGuardada, clave, sizeof(claveGuardada));
  Preferences nvs;
  nvs.begin(ESPACIO, false);
  nvs.putString("ssid", ssidGuardado);
  nvs.putString("clave", claveGuardada);
  nvs.end();
}

void borrarRed() {
  ssidGuardado[0] = claveGuardada[0] = 0;
  Preferences nvs;
  nvs.begin(ESPACIO, false);
  nvs.remove("ssid");
  nvs.remove("clave");
  nvs.end();
}

// ------------------ radio ------------------

void limitarPotencia() {
  esp_wifi_set_max_tx_power(RED_POTENCIA_DBM * 4);   // la unidad es 0.25 dBm
}

void abrirPortal() {
  // AP + STA a la vez: el AP sirve el portal mientras la parte STA
  // escanea y prueba la clave, sin tirar al telefono.
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(false);   // con el AP arriba, reintentar solo lo vuelve inestable
  WiFi.softAPConfig(IP_PORTAL, IP_PORTAL, IPAddress(255, 255, 255, 0));
  WiFi.softAP(RED_AP_SSID);       // abierta: lo unico que hay es el portal
  limitarPotencia();

  // DNS comodin: cualquier dominio resuelve a la bascula. Asi el telefono
  // detecta que hay portal y lo abre solo.
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", IP_PORTAL);

  portal      = true;
  actividadMs = millis();
  WiFi.scanNetworks(true);        // asincrono: la lista ya estara cuando abran el portal

  Serial.printf("[red] Portal abierto: conectate a \"%s\" (http://%s)\n", RED_AP_SSID, IP_PORTAL.toString().c_str());
}

void cerrarPortal() {
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);            // el enlace con el router se conserva
  WiFi.setAutoReconnect(true);
  portal = false;
  Serial.println("[red] Portal cerrado.");
}

/* Deja listo el intento; el radio arranca en lanzarIntento(). Van separados
 * porque al enlazarse el AP del portal puede brincar al canal del router y
 * soltar al telefono: primero tiene que salir la respuesta HTTP. */
void intentar(const char* ssid, const char* clave, bool nuevo) {
  strlcpy(ssidIntento, ssid, sizeof(ssidIntento));
  strlcpy(claveIntento, clave, sizeof(claveIntento));
  intentoNuevo    = nuevo;
  enlace          = CONECTANDO;
  inicioIntentoMs = millis();
  porLanzar       = true;
}

void lanzarIntento() {
  porLanzar = false;
  Serial.printf("[red] Conectando a \"%s\"...\n", ssidIntento);
  WiFi.scanDelete();
  WiFi.begin(ssidIntento, claveIntento[0] ? claveIntento : nullptr);
}

void arrancar() {
  WiFi.persistent(false);              // las claves van en nuestra NVS, no en la del driver
  WiFi.setHostname(RED_NOMBRE_MDNS);
  radioActivo = true;
  enlace      = SIN_INTENTO;

  if (ssidGuardado[0]) {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    limitarPotencia();
    intentar(ssidGuardado, claveGuardada, false);
    lanzarIntento();
  } else {
    Serial.println("[red] No hay WiFi guardado.");
    abrirPortal();
  }
  web::arrancar();
}

void parar() {
  web::detener();
  if (mdnsActivo) { MDNS.end(); mdnsActivo = false; }
  if (portal)     { dns.stop(); portal = false; }
  WiFi.scanDelete();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  radioActivo = false;
  enlace      = SIN_INTENTO;
  porLanzar   = false;
  olvidarMs   = 0;
  Serial.println("[red] WiFi apagado.");
}

// ------------------ lo que pasa con el enlace ------------------

void alConectar() {
  if (intentoNuevo) guardarRed(ssidIntento, claveIntento);
  enlace      = CONECTADO;
  conectadoMs = millis();
  ipVistaMs   = 0;
  pendiente   = intentoNuevo ? EVENTO_RED_CONECTO_NUEVA : EVENTO_RED_CONECTO;

  if (mdnsActivo) MDNS.end();
  mdnsActivo = MDNS.begin(RED_NOMBRE_MDNS);
  if (mdnsActivo) MDNS.addService("http", "tcp", 80);

  Serial.printf("[red] Conectada a \"%s\": http://%s  o  http://%s.local\n",
                ssidIntento, WiFi.localIP().toString().c_str(), RED_NOMBRE_MDNS);

  // El portal solo se queda un momento si la red se acaba de elegir desde
  // el telefono (para que alcance a leer la IP). Si fue un reintento de la
  // red guardada, ya no hace falta.
  if (portal && !intentoNuevo) cerrarPortal();
}

void alFallar() {
  Serial.printf("[red] No se pudo conectar a \"%s\": clave incorrecta o red fuera de alcance.\n", ssidIntento);
  WiFi.disconnect();          // que deje de buscarla
  enlace      = FALLO;
  actividadMs = millis();
  if (!portal) abrirPortal();
}

void alPerderse() {
  Serial.println("[red] Se perdio la conexion: reintentando.");
  if (mdnsActivo) { MDNS.end(); mdnsActivo = false; }
  pendiente = EVENTO_RED_SE_PERDIO;
  if (portal) {
    intentar(ssidGuardado, claveGuardada, false);
  } else {
    // En modo estacion el driver reintenta solo; aqui solo se lleva el tiempo.
    strlcpy(ssidIntento, ssidGuardado, sizeof(ssidIntento));
    intentoNuevo    = false;
    enlace          = CONECTANDO;
    inicioIntentoMs = millis();
  }
}

// Con que estandar quedo el enlace: es lo que distingue al C6 y al C5.
void estandarNegociado(const char*& estandar, const char*& norma) {
  wifi_phy_mode_t modo;
  if (esp_wifi_sta_get_negotiated_phymode(&modo) != ESP_OK) { estandar = "WiFi"; norma = ""; return; }
  switch (modo) {
    case WIFI_PHY_MODE_HE20:  estandar = "WiFi 6";  norma = "802.11ax"; break;
    case WIFI_PHY_MODE_VHT20: estandar = "WiFi 5";  norma = "802.11ac"; break;
    case WIFI_PHY_MODE_HT20:
    case WIFI_PHY_MODE_HT40:  estandar = "WiFi 4";  norma = "802.11n";  break;
    case WIFI_PHY_MODE_11A:   estandar = "802.11a"; norma = "802.11a";  break;
    case WIFI_PHY_MODE_11G:   estandar = "802.11g"; norma = "802.11g";  break;
    case WIFI_PHY_MODE_11B:   estandar = "802.11b"; norma = "802.11b";  break;
    default:                  estandar = "WiFi LR"; norma = "";         break;
  }
}

}  // namespace

namespace red {

void iniciar() {
  cargar();
  if (wifiEncendido) arrancar();
  else Serial.println("[red] WiFi apagado desde el menu.");
}

void atender() {
  if (!radioActivo) return;

  if (portal) dns.processNextRequest();
  web::atender();
  if (porLanzar) lanzarIntento();   // la respuesta al telefono ya salio

  uint32_t ahora = millis();

  if (olvidarMs && (int32_t)(ahora - olvidarMs) >= 0) {
    olvidarMs = 0;
    olvidar();
    return;
  }

  bool hayEnlace = WiFi.status() == WL_CONNECTED;

  switch (enlace) {
    case CONECTANDO:
      if (hayEnlace) alConectar();
      else if (ahora - inicioIntentoMs > RED_ESPERA_MS) alFallar();
      break;

    case CONECTADO:
      if (!hayEnlace) alPerderse();
      else if (portal && ((ipVistaMs && ahora - ipVistaMs > RED_CIERRE_PORTAL_MS) ||
                          ahora - conectadoMs > RED_CIERRE_PORTAL_MAX_MS)) cerrarPortal();
      break;

    default:
      // Portal abierto. Si hay red guardada pero el router no estaba (p. ej.
      // se fue la luz) y nadie esta usando el portal, se vuelve a intentar.
      if (ssidGuardado[0] && WiFi.softAPgetStationNum() == 0 && ahora - actividadMs > RED_REINTENTO_MS) {
        intentar(ssidGuardado, claveGuardada, false);
      }
      break;
  }
}

InfoRed info() {
  InfoRed i;
  i.hayGuardada   = ssidGuardado[0] != 0;
  i.portalAbierto = portal;
  if (!radioActivo) return i;

  i.telefonos = portal ? WiFi.softAPgetStationNum() : 0;

  switch (enlace) {
    case CONECTADO:
      i.estado = RED_CONECTADA;
      strlcpy(i.ssid, ssidIntento, sizeof(i.ssid));
      strlcpy(i.ip, WiFi.localIP().toString().c_str(), sizeof(i.ip));
      i.rssi   = WiFi.RSSI();
      i.canal  = WiFi.channel();
      i.banda5 = i.canal > 14;        // del 1 al 14 es 2.4 GHz; del 36 en adelante, 5 GHz
      estandarNegociado(i.estandar, i.norma);
      break;

    case CONECTANDO:
      i.estado   = RED_CONECTANDO;
      i.progreso = min(1.0f, (float)(millis() - inicioIntentoMs) / RED_ESPERA_MS);
      strlcpy(i.ssid, ssidIntento, sizeof(i.ssid));
      break;

    default:
      i.estado = RED_PORTAL;
      i.fallo  = enlace == FALLO;
      if (i.fallo) strlcpy(i.ssid, ssidIntento, sizeof(i.ssid));
      break;
  }
  return i;
}

EventoRed evento() {
  EventoRed e = pendiente;
  pendiente = EVENTO_RED_NINGUNO;
  return e;
}

bool encendida() { return radioActivo; }

void encender(bool si) {
  if (si == radioActivo) return;
  wifiEncendido = si;
  Preferences nvs;
  nvs.begin(ESPACIO, false);
  nvs.putBool("encendido", si);
  nvs.end();
  if (si) arrancar();
  else    parar();
}

void olvidar() {
  Serial.println("[red] Se borra el WiFi guardado.");
  borrarRed();
  if (!radioActivo) return;

  if (mdnsActivo) { MDNS.end(); mdnsActivo = false; }
  WiFi.disconnect();
  enlace    = SIN_INTENTO;
  porLanzar = false;
  if (portal) actividadMs = millis();
  else        abrirPortal();
}

bool probar(const char* ssid, const char* clave) {
  size_t largoSsid = strlen(ssid), largoClave = strlen(clave);
  bool claveValida = largoClave == 0 || (largoClave >= 8 && largoClave <= 64);
  if (!portal || enlace == CONECTADO || largoSsid == 0 || largoSsid > 32 || !claveValida) return false;

  actividadMs = millis();
  intentar(ssid, clave, true);
  return true;
}

void actividad() { actividadMs = millis(); }

void ipVista() {
  if (ipVistaMs == 0) ipVistaMs = millis() | 1;
}

void olvidarEnBreve() { olvidarMs = (millis() + 600) | 1; }

}  // namespace red

/*
 * ============================================================
 *  LATIDO - el ultrasonido en una pantalla redonda, con su sonido
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 *  www.tostatronic.com
 * ============================================================
 *
 *  ESP32-C3 Super Mini + pantalla redonda GC9A01 + amplificador I2S
 *  MAX98357A + bocina de 8 ohms, con bateria 18650 y cargador TP4056.
 *
 *  Reproduce en bucle un video corto con su audio (el eco del corazon del
 *  bebe). El video se sube desde el celular: el ESP32 crea la red WiFi
 *  "Latido" y sirve una pagina en http://192.168.4.1 que recorta y convierte
 *  el video ahi mismo, en el navegador, y lo guarda en la memoria Flash.
 *
 *  Librerias (Gestor de librerias del Arduino IDE):
 *    - "GFX Library for Arduino" (moononournation) >= 1.6.5
 *    - "JPEGDEC" (bitbank2)
 *  Core: esp32 de Espressif >= 3.3.0
 *
 *  Placa y opciones: ver config.h. Conexiones y armado: ver README.md.
 * ============================================================
 */

#include <LittleFS.h>
#include <Preferences.h>
#include <esp_sleep.h>

#include "config.h"
#include "pantalla.h"
#include "reproductor.h"
#include "portal.h"

Ajustes ajustes = { VOLUMEN_INICIAL, 100 };
static Preferences memoria;

static bool     hayBateria = false;      // se detecta sola al arrancar
static uint16_t bateriaFiltrada = 0;
static bool     esperaDibujada = false;  // pantalla "sin video" ya pintada
static uint32_t avisoHasta = 0;          // un mensaje temporal tapa el video hasta este millis()

// ------------------------------------------------------------
//  Ajustes guardados
// ------------------------------------------------------------

void ajustesAplicar() {
  reproductorVolumen(ajustes.volumen);
  pantallaBrillo(ajustes.brillo);
}

void ajustesGuardar() {
  memoria.putUChar("vol", ajustes.volumen);
  memoria.putUChar("bri", ajustes.brillo);
}

// ------------------------------------------------------------
//  Bateria
// ------------------------------------------------------------

static uint16_t leerBateria() {
  if (PIN_BATERIA == SIN_PIN) return 0;
  uint32_t suma = 0;
  for (int i = 0; i < 8; i++) suma += analogReadMilliVolts(PIN_BATERIA);
  return (uint16_t)(suma / 8 * BATERIA_FACTOR);
}

uint16_t bateriaMv() { return hayBateria ? bateriaFiltrada : 0; }

static void dormir(const char* motivo) {
  REGISTRO("[energia] a dormir: %s\n", motivo);
  reproductorCerrar();
  portalWifi(false);
  pantallaMensaje("Bateria baja", "Conecta el", "cargador");
  delay(4000);
  pantallaApagar();
  esp_deep_sleep_start();                // despierta al apagar y prender el interruptor
}

static void vigilarBateria() {
  static uint32_t proxima = 0, bajaDesde = 0;
  if (!hayBateria || millis() < proxima) return;
  proxima = millis() + 2000;
  bateriaFiltrada = (bateriaFiltrada * 3 + leerBateria()) / 4;
  if (bateriaFiltrada >= BATERIA_CORTE_MV) { bajaDesde = 0; return; }
  if (!bajaDesde) bajaDesde = millis();
  else if (millis() - bajaDesde > 15000) dormir("bateria en el corte");
}

// ------------------------------------------------------------
//  Pantallas de espera
// ------------------------------------------------------------

static void pantallaWifi() {
  pantallaMensaje("WiFi: " WIFI_NOMBRE, strlen(WIFI_CLAVE) >= 8 ? WIFI_CLAVE : "(sin clave)", "192.168.4.1");
}

static void aviso(const char* r1, const char* r2, const char* r3, uint32_t ms) {
  reproductorPausa(true);
  pantallaMensaje(r1, r2, r3);
  avisoHasta = millis() + ms;
  esperaDibujada = false;
}

// ------------------------------------------------------------
//  Boton: toque = pausa / sigue, sostenido = WiFi
// ------------------------------------------------------------

static bool botonAbajo() {
  if (PIN_BOTON != SIN_PIN && digitalRead(PIN_BOTON) == LOW) return true;
  if (PIN_BOTON_BOOT != SIN_PIN && digitalRead(PIN_BOTON_BOOT) == LOW) return true;
  return false;
}

static void atenderBoton() {
  static bool antes = false, largoHecho = false;
  static uint32_t desde = 0, cambio = 0;
  bool ahora = botonAbajo();
  if (ahora != antes && millis() - cambio > 30) {        // antirrebote
    cambio = millis();
    antes = ahora;
    if (ahora) { desde = millis(); largoHecho = false; }
    else if (!largoHecho && reproductorActivo() && !avisoHasta) reproductorPausa(!reproductorEnPausa());
  }
  if (antes && !largoHecho && millis() - desde > 1500) {
    largoHecho = true;
    bool prender = !portalWifiPrendido();
    portalWifi(prender);
    if (prender) { reproductorPausa(true); pantallaWifi(); avisoHasta = millis() + 8000; esperaDibujada = false; }
    else aviso("WiFi apagado", nullptr, nullptr, 2000);
  }
}

// ------------------------------------------------------------
//  setup / loop
// ------------------------------------------------------------

void setup() {
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.begin(115200);
#endif
  if (PIN_BOTON != SIN_PIN) pinMode(PIN_BOTON, INPUT_PULLUP);
  if (PIN_BOTON_BOOT != SIN_PIN) pinMode(PIN_BOTON_BOOT, INPUT_PULLUP);

  pantallaIniciar();
  memoria.begin("latido", false);
  ajustes.volumen = memoria.getUChar("vol", VOLUMEN_INICIAL);
  ajustes.brillo  = memoria.getUChar("bri", 100);

  reproductorIniciar();
  ajustesAplicar();

  // Con el divisor cableado se leen entre 3.0 y 4.3 V. Un pin al aire o la
  // placa alimentada solo por USB dan otra cosa: entonces no se vigila nada.
  uint16_t mv = leerBateria();
  hayBateria = mv > 2900 && mv < 4400;
  bateriaFiltrada = mv;
  REGISTRO("\n=== Latido %s ===\n[bateria] %s (%u mV)\n", VERSION_FW, hayBateria ? "con lectura" : "sin divisor", mv);

  if (!LittleFS.begin(true)) REGISTRO("[fs] no se pudo montar LittleFS\n");

  portalIniciar();
  portalWifi(true);

  // Saludo: el corazon late un par de segundos con los datos del WiFi
  pantallaWifi();
  uint32_t t = millis();
  while (millis() - t < 2500) { pantallaLatir(); portalTick(); delay(10); }

  const char* e = reproductorAbrir();
  if (e) REGISTRO("[video] %s\n", e);
}

void loop() {
  portalTick();
  atenderBoton();
  vigilarBateria();

  if (avisoHasta) {                              // mensaje temporal sobre el video
    pantallaLatir();
    if (millis() > avisoHasta) { avisoHasta = 0; reproductorPausa(false); }
  } else if (reproductorActivo()) {
    esperaDibujada = false;
    reproductorTick();
  } else {                                       // sin video: datos del WiFi y corazon latiendo
    if (!esperaDibujada || !pantallaEsMensaje()) {   // tambien tras una subida fallida
      esperaDibujada = true;
      if (!portalWifiPrendido()) portalWifi(true);
      pantallaWifi();
    }
    pantallaLatir();
  }

  // El WiFi se apaga solo cuando ya hay video y nadie lo esta usando
  if (portalWifiPrendido() && reproductorActivo() &&
      millis() - portalUltimaActividad() > WIFI_VENTANA_S * 1000UL) {
    portalWifi(false);
  }

  if (!reproductorActivo() || reproductorEnPausa()) delay(5);   // sin video no hay prisa
}

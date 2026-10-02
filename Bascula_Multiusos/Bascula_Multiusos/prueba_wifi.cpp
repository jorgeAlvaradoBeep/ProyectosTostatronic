/*
 * ============================================================
 *  PRUEBA DE WIFI - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "prueba_wifi.h"
#include <WiFi.h>
#include <WiFiUdp.h>

namespace {

const uint16_t PUERTO_DIFUSION = 50000;
const uint16_t PUERTO_DESCARTE = 9;       // "discard": quien lo recibe, lo tira
const size_t   TAM_PAQUETE     = 1024;

volatile bool  corriendo = false;
TaskHandle_t   tarea     = nullptr;
volatile uint32_t total = 0, porSegundo = 0;

/* A donde mandar, segun como este el WiFi en este momento. Devuelve
 * false si no hay por donde (apagado, o todavia conectandose). */
bool destino(IPAddress& ip, uint16_t& puerto) {
  wifi_mode_t modo = WiFi.getMode();
  if (modo == WIFI_MODE_NULL) return false;

  if (WiFi.status() == WL_CONNECTED) {
    ip     = WiFi.gatewayIP();
    puerto = PUERTO_DESCARTE;
    return true;
  }
  if (modo & WIFI_MODE_AP) {
    ip     = WiFi.softAPIP();
    ip[3]  = 255;
    puerto = PUERTO_DIFUSION;
    return true;
  }
  return false;
}

void tareaTransmitir(void*) {
  WiFiUDP udp;
  static uint8_t datos[TAM_PAQUETE];
  for (size_t i = 0; i < TAM_PAQUETE; i++) datos[i] = (uint8_t)i;

  IPAddress ip;
  uint16_t  puerto = 0;
  bool      hayDestino = destino(ip, puerto);

  uint32_t inicioSegundo = millis(), enSegundo = 0;
  while (corriendo) {
    if (hayDestino) {
      udp.beginPacket(ip, puerto);
      udp.write(datos, TAM_PAQUETE);
      if (udp.endPacket()) { total = total + 1; enSegundo++; }
    }

    if (millis() - inicioSegundo >= 1000) {
      porSegundo = enSegundo;
      enSegundo = 0;
      inicioSegundo = millis();
      hayDestino = destino(ip, puerto);   // la red pudo conectarse o caerse
    }
    // Cede 1 ms: el radio se mantiene saturado (su cola nunca se vacia)
    // y la pantalla y loop() siguen teniendo CPU.
    vTaskDelay(1);
  }
  porSegundo = 0;
  tarea = nullptr;
  vTaskDelete(nullptr);
}

}  // namespace

namespace pruebaWifi {

void iniciar() {
  if (corriendo) return;
  total = 0;
  porSegundo = 0;
  corriendo = true;
  // Prioridad 3: por debajo de la bascula (6), por encima de loop() (1).
  xTaskCreate(tareaTransmitir, "pruebaWifi", 4096, nullptr, 3, &tarea);
}

void detener() {
  if (!corriendo) return;
  corriendo = false;
  while (tarea) vTaskDelay(pdMS_TO_TICKS(5));   // que termine el paquete en curso
}

bool     activa()             { return corriendo; }
uint32_t paquetesPorSegundo() { return porSegundo; }
uint32_t paquetesTotales()    { return total; }

}  // namespace pruebaWifi

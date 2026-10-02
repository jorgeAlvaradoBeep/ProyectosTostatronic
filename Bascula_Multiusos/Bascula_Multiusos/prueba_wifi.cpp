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

const char*    RED_PRUEBA   = "Tostatronic-Bascula-Prueba";
const uint16_t PUERTO       = 50000;
const size_t   TAM_PAQUETE  = 1024;

volatile bool  corriendo = false;
TaskHandle_t   tarea     = nullptr;
volatile uint32_t total = 0, porSegundo = 0;

void tareaTransmitir(void*) {
  WiFiUDP udp;
  static uint8_t datos[TAM_PAQUETE];
  for (size_t i = 0; i < TAM_PAQUETE; i++) datos[i] = (uint8_t)i;

  IPAddress difusion = WiFi.softAPIP();
  difusion[3] = 255;

  uint32_t inicioSegundo = millis(), enSegundo = 0;
  while (corriendo) {
    udp.beginPacket(difusion, PUERTO);
    udp.write(datos, TAM_PAQUETE);
    if (udp.endPacket()) { total = total + 1; enSegundo++; }

    if (millis() - inicioSegundo >= 1000) {
      porSegundo = enSegundo;
      enSegundo = 0;
      inicioSegundo = millis();
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
  WiFi.mode(WIFI_AP);
  WiFi.softAP(RED_PRUEBA);          // red abierta: no hay nada que proteger
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
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
}

bool     activa()             { return corriendo; }
uint32_t paquetesPorSegundo() { return porSegundo; }
uint32_t paquetesTotales()    { return total; }

}  // namespace pruebaWifi

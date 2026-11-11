/*
 * ============================================================
 *  PORTAL - red WiFi propia y pagina para subir el video
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#pragma once
#include <Arduino.h>

struct Ajustes {
  uint8_t volumen;   // 0..100
  uint8_t brillo;    // 5..100
};
extern Ajustes ajustes;
void ajustesAplicar();
void ajustesGuardar();
uint16_t bateriaMv();            // 0 = no hay divisor cableado

void portalIniciar();            // rutas del servidor (no prende el WiFi)
void portalWifi(bool prender);
bool portalWifiPrendido();
void portalTick();
uint32_t portalUltimaActividad();   // millis() de la ultima peticion o cliente conectado

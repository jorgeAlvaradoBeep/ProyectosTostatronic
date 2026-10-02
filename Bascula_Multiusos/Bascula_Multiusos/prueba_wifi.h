/*
 * ============================================================
 *  PRUEBA DE WIFI - el radio transmitiendo sin parar
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Sirve para DEMOSTRAR que el WiFi no dana las lecturas del
 *  HX711 (ver hx711.h). Con el WiFi apagado cualquier bascula
 *  funciona; la prueba de verdad es con el radio ocupado.
 *
 *  Levanta una red abierta "Tostatronic-Bascula-Prueba" y manda
 *  paquetes UDP de difusion lo mas rapido que el radio acepta:
 *  maxima carga de interrupciones del WiFi. Mientras tanto se
 *  comparan el ruido, los atipicos y las tramas lentas contra la
 *  misma medicion con el WiFi apagado.
 *
 *  No necesita internet ni claves. En la fase 3 el WiFi real
 *  (portal cautivo) reemplaza a esta prueba en el uso diario.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

namespace pruebaWifi {

void     iniciar();
void     detener();
bool     activa();

// Paquetes que el radio acepto en el ultimo segundo, y el total.
uint32_t paquetesPorSegundo();
uint32_t paquetesTotales();

}  // namespace pruebaWifi

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
 *  Usa el WiFi real de la bascula (red.h), como este en ese
 *  momento, y manda paquetes UDP lo mas rapido que el radio
 *  acepta: maxima carga de interrupciones del WiFi.
 *
 *    Conectada a tu red -> al router, al puerto de "descarte":
 *                          el trafico no molesta a nadie mas.
 *    Portal abierto ..... -> difusion en la red propia.
 *
 *  Mientras tanto se comparan el ruido, los atipicos y las tramas
 *  lentas contra la misma medicion con la prueba apagada.
 *
 *  No enciende ni apaga el WiFi: eso es de MENU > Conexion.
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

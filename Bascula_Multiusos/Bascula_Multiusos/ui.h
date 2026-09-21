/*
 * ============================================================
 *  INTERFAZ EN LA PANTALLA REDONDA
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  La GC9A01 es circular: las esquinas del cuadro de 240x240 no
 *  existen. Por eso cada texto se pinta en una "zona" con centro
 *  y tamano, y pantalla::zonaTexto() verifica que la zona quepa
 *  en el circulo util (y lo reporta por Serial si no).
 *
 *  Reparto de la pantalla:
 *      anillo perimetral .. indicador (r 111 a 119)
 *      franja superior .... modo / titulo
 *      centro ............. el numero grande + unidad
 *      franja inferior .... estado
 *
 *  FASE 1: pantalla de arranque y prueba de hardware.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "bascula.h"
#include "teclado.h"

namespace ui {

// Logo + atribucion. Bloquea 'duracionMs' (solo se usa al arrancar).
void arranque(uint16_t duracionMs);

// Prueba de hardware: peso crudo, muestras por segundo, ruido y teclas.
void pruebaHardwareEntrar();
void pruebaHardwareTecla(const EventoTecla& evento);
void pruebaHardwareRefrescar(const Lectura& lectura);

}  // namespace ui

/*
 * ============================================================
 *  PANTALLA - GC9A01 redonda de 240 x 240 con Arduino_GFX
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

extern Arduino_GFX* gfx;

bool pantallaIniciar();
void pantallaBrillo(uint8_t porciento);          // 0..100 (necesita el pin BLK cableado)
void pantallaApagar();

// Corazon arriba y hasta tres renglones de texto abajo (nullptr = renglon vacio).
void pantallaMensaje(const char* r1, const char* r2 = nullptr, const char* r3 = nullptr);
// Hace latir el corazon de pantallaMensaje(). Llamar seguido desde loop().
void pantallaLatir();
bool pantallaEsMensaje();                         // true si lo ultimo que se pinto fue pantallaMensaje()
// Anillo de avance con el porcentaje al centro.
void pantallaAvance(uint8_t porciento, const char* titulo);

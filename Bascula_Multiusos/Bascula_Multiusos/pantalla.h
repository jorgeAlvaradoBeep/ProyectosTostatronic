/*
 * ============================================================
 *  PANTALLA - capa propia sobre la libreria grafica
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  El resto del proyecto dibuja SOLO con estas funciones. La
 *  libreria grafica (Arduino_GFX) se menciona unicamente en
 *  pantalla.cpp: cambiar a LovyanGFX es reescribir ese archivo
 *  y nada mas.
 *
 *  Para que el cambio sea facil de verdad, a la libreria solo se
 *  le piden primitivas que todas tienen: rellenar, arco, circulo,
 *  linea y "copiar un bloque de pixeles". El TEXTO no se le pide:
 *  se compone aqui, en un buffer fuera de pantalla, y se copia de
 *  un golpe. Dos ventajas:
 *    - el numero del peso se actualiza sin parpadeo;
 *    - acentos y digitos de ancho fijo no dependen de la libreria.
 *
 *  Convencion de angulos: grados, 0 = las 12 en punto, sentido
 *  horario (como un reloj; asi se piensa una pantalla redonda).
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ------------------ PALETA (RGB565) ------------------
// Misma paleta que los diagramas y las paginas web de Tostatronic.

const uint16_t COLOR_FONDO    = 0x10A3;  // #101519
const uint16_t COLOR_TARJETA  = 0x1905;  // #1A222A
const uint16_t COLOR_BORDE    = 0x29A7;  // #2A353F
const uint16_t COLOR_TEXTO    = 0xF7BE;  // #F3F6F4
const uint16_t COLOR_TENUE    = 0x9515;  // #93A1AA
const uint16_t COLOR_AZUL     = 0x4D5E;  // #4BA8F5  azul de marca: acento
const uint16_t COLOR_VERDE    = 0x2EB4;  // #2DD4A7  estable / correcto
const uint16_t COLOR_AMBAR    = 0xF5A5;  // #F0B429  avisos
const uint16_t COLOR_NARANJA  = 0xFAE7;  // #FF5C38  error / sobrecarga

// ------------------ GEOMETRIA DEL CIRCULO ------------------

const int16_t CENTRO_X     = 120;
const int16_t CENTRO_Y     = 120;
const int16_t RADIO_TOTAL  = 120;
const int16_t ANILLO_EXT   = 119;   // anillo perimetral indicador
const int16_t ANILLO_INT   = 111;
const int16_t RADIO_UTIL   = 106;   // el contenido vive dentro de este radio

enum Fuente : uint8_t { F_NUMERO, F_TITULO, F_TEXTO, F_CHICA };

namespace pantalla {

bool iniciar();

bool tieneBrillo();               // false si BL va cableado fijo a 3V3
void brillo(uint8_t nivel);       // 0..255

void limpiar(uint16_t color = COLOR_FONDO);

// --- texto ---

// Ancho en pixeles de un texto UTF-8 con la fuente dada.
int16_t anchoTexto(const char* utf8, Fuente fuente);
int16_t altoMayusculas(Fuente fuente);

// Pinta una zona de ancho x alto centrada en (cx, cy): fondo + texto
// centrado, compuesto fuera de pantalla y copiado de un golpe (sin
// parpadeo). Devuelve false si la zona se sale del circulo util o si el
// texto no cabe en la zona: en ambos casos dibuja igual, pero lo reporta
// por Serial para que el error de diseno no pase desapercibido.
bool zonaTexto(int16_t cx, int16_t cy, int16_t ancho, int16_t alto,
               const char* utf8, Fuente fuente,
               uint16_t color = COLOR_TEXTO, uint16_t fondo = COLOR_FONDO);

// --- figuras ---

void arco(int16_t rExterior, int16_t rInterior, float gradosInicio, float gradosFin, uint16_t color);
void circulo(int16_t x, int16_t y, int16_t radio, uint16_t color);
void aro(int16_t x, int16_t y, int16_t radio, uint16_t color);
void rectangulo(int16_t x, int16_t y, int16_t ancho, int16_t alto, uint16_t color);
void linea(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void imagen(int16_t x, int16_t y, int16_t ancho, int16_t alto, const uint16_t* pixelesProgmem);

// --- ayudas para disenar en redondo ---

// true si el rectangulo centrado en (cx, cy) cabe entero dentro del radio.
bool cabeEnCirculo(int16_t cx, int16_t cy, int16_t ancho, int16_t alto, int16_t radio = RADIO_UTIL);

// Ancho maximo de una zona de cierto alto centrada en (CENTRO_X, cy).
int16_t anchoDisponible(int16_t cy, int16_t alto, int16_t radio = RADIO_UTIL);

// Dibuja el circulo util y, encima, el contorno de cada zona pintada
// desde que se activo. Es para revisar el diseno; no para el usuario.
void depuracion(bool activa);
bool depuracionActiva();

}  // namespace pantalla

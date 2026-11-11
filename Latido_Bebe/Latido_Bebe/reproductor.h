/*
 * ============================================================
 *  REPRODUCTOR - video MJPEG a la pantalla y audio ADPCM al MAX98357A
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  El audio manda: una tarea aparte lo va sacando por I2S sin pausas y el
 *  video dibuja el cuadro que toca segun lo que ya se oyo. Si la pantalla se
 *  atrasa se salta cuadros, pero el latido nunca se corta ni se desfasa.
 */

#pragma once
#include <Arduino.h>

struct InfoVideo {
  bool     hay;
  uint32_t bytes;
  uint16_t cuadros;
  float    fps;
  float    segundos;
  bool     audio;
};

void reproductorIniciar();                 // una sola vez, en setup()
const char* reproductorAbrir();            // nullptr = listo; si no, el motivo
void reproductorCerrar();                  // suelta el archivo (antes de borrarlo o reemplazarlo)
void reproductorTick();                    // llamar en cada vuelta de loop()

void reproductorPausa(bool pausar);
bool reproductorEnPausa();
bool reproductorActivo();
void reproductorVolumen(uint8_t porciento);   // 0..100
InfoVideo reproductorInfo();

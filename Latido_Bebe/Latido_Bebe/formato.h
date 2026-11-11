/*
 * ============================================================
 *  FORMATO .LTD - video MJPEG + audio ADPCM en un solo archivo
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Lo arma la pagina web, en el navegador (ver pagina.h).
 *  Todo en little-endian:
 *
 *    [cabecera 32 bytes]
 *    [indice: (cuadros + 1) x uint32]  posicion de cada JPEG; el ultimo es el final
 *    [JPEG 0][JPEG 1]...               240x240, baseline
 *    [audio]                           IMA ADPCM mono, 4 bits por muestra,
 *                                      nibble bajo primero, estado inicial (0, 0)
 *
 *  Video y audio duran lo mismo, asi que el archivo se repite sin cortes.
 */

#pragma once
#include <Arduino.h>

struct __attribute__((packed)) CabeceraLtd {
  char     magia[4];        // "LTD1"
  uint16_t ancho, alto;     // 240 x 240
  uint16_t fpsX100;         // 1200 = 12 fps
  uint16_t cuadros;
  uint32_t audioHz;         // 16000
  uint32_t audioMuestras;
  uint32_t audioPos;        // donde empieza el audio dentro del archivo
  uint32_t audioBytes;
  uint8_t  audioCodec;      // 0 = sin audio, 1 = IMA ADPCM
  uint8_t  version;         // 1
  uint16_t reservado;
};
static_assert(sizeof(CabeceraLtd) == 32, "la cabecera .ltd mide 32 bytes");

// Decodificador IMA ADPCM: 4 bits -> 16 bits.
class DecoAdpcm {
public:
  void reiniciar() { _pred = 0; _idx = 0; }

  inline int16_t muestra(uint8_t c) {
    static const int16_t PASO[89] = {
      7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66,
      73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
      449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
      2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
      9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };
    static const int8_t SALTO[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

    int32_t paso = PASO[_idx];
    int32_t dif = paso >> 3;
    if (c & 4) dif += paso;
    if (c & 2) dif += paso >> 1;
    if (c & 1) dif += paso >> 2;
    _pred += (c & 8) ? -dif : dif;
    if (_pred > 32767) _pred = 32767; else if (_pred < -32768) _pred = -32768;
    _idx += SALTO[c & 7];
    if (_idx < 0) _idx = 0; else if (_idx > 88) _idx = 88;
    return (int16_t)_pred;
  }

private:
  int32_t _pred = 0;
  int8_t  _idx = 0;
};

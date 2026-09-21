/*
 * ============================================================
 *  PANTALLA - implementacion sobre Arduino_GFX (GC9A01)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  UNICO archivo del proyecto que conoce la libreria grafica.
 *  No se usa TFT_eSPI a proposito: en el ESP32-C6 con el core 3.x
 *  de Arduino no compila (VSPI no declarado) o reinicia en bucle.
 *  Arduino_GFX (moononournation) >= 1.6.5 compila para C6 y C5.
 * ============================================================
 */

#include "pantalla.h"
#include "config.h"
#include "fuentes.h"
#include <Arduino_GFX_Library.h>

namespace {

Arduino_DataBus* bus = nullptr;
Arduino_GFX*     gfx = nullptr;

// Buffer fuera de pantalla para componer texto. 200 x 60 cubre la zona
// mas grande (el numero del peso). Son 24 KB: un framebuffer completo
// serian 115 KB, demasiado para convivir con el WiFi en estos chips.
const uint32_t ZONA_MAX_PIXELES = 200UL * 60UL;
uint16_t* lienzo = nullptr;

bool modoDepuracion = false;

const GFXfont* fuenteDe(Fuente f) {
  switch (f) {
    case F_NUMERO: return &FuenteNumero;
    case F_TITULO: return &FuenteTitulo;
    case F_TEXTO:  return &FuenteTexto;
    default:       return &FuenteChica;
  }
}

/* Lee un caracter UTF-8 y lo traduce al codigo de nuestras fuentes:
 * Latin-1 directo, mas los extras del hueco 0x80-0x9F (la misma tabla
 * que usa generar_recursos.py). Avanza el puntero. */
uint8_t siguienteCaracter(const char*& p) {
  uint8_t  c = (uint8_t)*p++;
  uint32_t cp = c;

  if (c >= 0xF0)      { cp = c & 0x07; for (uint8_t i = 0; i < 3 && *p; i++) cp = (cp << 6) | ((uint8_t)*p++ & 0x3F); }
  else if (c >= 0xE0) { cp = c & 0x0F; for (uint8_t i = 0; i < 2 && *p; i++) cp = (cp << 6) | ((uint8_t)*p++ & 0x3F); }
  else if (c >= 0xC0) { cp = c & 0x1F; if (*p) cp = (cp << 6) | ((uint8_t)*p++ & 0x3F); }

  if (cp == 0x2248) return 0x80;   // aproximadamente igual
  if (cp == 0x2022) return 0x81;   // bala
  if (cp == 0x2026) return 0x82;   // puntos suspensivos
  if (cp < 0x100 && !(cp >= 0x7F && cp <= 0x9F)) return (uint8_t)cp;
  return '?';
}

const GFXglyph* glifoDe(const GFXfont* fuente, uint8_t codigo) {
  if (codigo < fuente->first || codigo > fuente->last) return nullptr;
  return &fuente->glyph[codigo - fuente->first];
}

/* Dibuja un glifo de 1 bit dentro del lienzo, recortando a sus bordes. */
void glifoAlLienzo(const GFXfont* fuente, const GFXglyph* g, int16_t x, int16_t yBase,
                   int16_t ancho, int16_t alto, uint16_t color) {
  const uint8_t* mapa = fuente->bitmap + g->bitmapOffset;
  uint16_t bit = 0;

  for (int16_t fila = 0; fila < g->height; fila++) {
    int16_t py = yBase + g->yOffset + fila;
    for (int16_t col = 0; col < g->width; col++, bit++) {
      if (!(mapa[bit >> 3] & (0x80 >> (bit & 7)))) continue;
      int16_t px = x + g->xOffset + col;
      if (px >= 0 && px < ancho && py >= 0 && py < alto) lienzo[(int32_t)py * ancho + px] = color;
    }
  }
}

}  // namespace

namespace pantalla {

bool iniciar() {
  bus = new Arduino_ESP32SPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_SCK, PIN_TFT_MOSI, GFX_NOT_DEFINED);
  gfx = new Arduino_GC9A01(bus, PIN_TFT_RST == SIN_PIN ? GFX_NOT_DEFINED : PIN_TFT_RST,
                           TFT_ROTACION, TFT_IPS);

  lienzo = (uint16_t*)malloc(ZONA_MAX_PIXELES * sizeof(uint16_t));
  if (!lienzo || !gfx->begin(TFT_FRECUENCIA)) return false;

  gfx->fillScreen(COLOR_FONDO);   // antes de encender la luz: sin destello blanco

  if (tieneBrillo()) {
    ledcAttach(PIN_TFT_BL, 5000, 8);
    brillo(TFT_BRILLO);
  }
  return true;
}

bool tieneBrillo() { return PIN_TFT_BL != SIN_PIN; }

void brillo(uint8_t nivel) {
  if (tieneBrillo()) ledcWrite(PIN_TFT_BL, nivel);
}

void limpiar(uint16_t color) {
  gfx->fillScreen(color);
  if (modoDepuracion) gfx->drawCircle(CENTRO_X, CENTRO_Y, RADIO_UTIL, COLOR_AMBAR);
}

// ------------------ texto ------------------

int16_t anchoTexto(const char* utf8, Fuente f) {
  const GFXfont* fuente = fuenteDe(f);
  int16_t ancho = 0;
  while (*utf8) {
    const GFXglyph* g = glifoDe(fuente, siguienteCaracter(utf8));
    if (g) ancho += g->xAdvance;
  }
  return ancho;
}

int16_t altoMayusculas(Fuente f) {
  switch (f) {
    case F_NUMERO: return FuenteNumeroAltoMayus;
    case F_TITULO: return FuenteTituloAltoMayus;
    case F_TEXTO:  return FuenteTextoAltoMayus;
    default:       return FuenteChicaAltoMayus;
  }
}

bool zonaTexto(int16_t cx, int16_t cy, int16_t ancho, int16_t alto,
               const char* utf8, Fuente f, uint16_t color, uint16_t fondo) {
  if (ancho <= 0 || alto <= 0 || (uint32_t)ancho * alto > ZONA_MAX_PIXELES) {
    Serial.printf("[pantalla] zona %dx%d no cabe en el lienzo\n", ancho, alto);
    return false;
  }

  const GFXfont* fuente = fuenteDe(f);
  int16_t anchoTxt = anchoTexto(utf8, f);
  bool correcto = true;

  if (!cabeEnCirculo(cx, cy, ancho, alto)) {
    Serial.printf("[pantalla] zona \"%s\" se sale del circulo util\n", utf8);
    correcto = false;
  }
  if (anchoTxt > ancho) {
    Serial.printf("[pantalla] \"%s\" mide %d px y la zona %d px\n", utf8, anchoTxt, ancho);
    correcto = false;
  }

  for (int32_t i = 0; i < (int32_t)ancho * alto; i++) lienzo[i] = fondo;

  // Centrado: en horizontal por el ancho del texto; en vertical por el
  // alto de las mayusculas (si se usara la caja de cada texto, "g" y "A"
  // quedarian a alturas distintas y el renglon brincaria).
  int16_t x     = (ancho - anchoTxt) / 2;
  int16_t yBase = (alto + altoMayusculas(f)) / 2;

  while (*utf8) {
    const GFXglyph* g = glifoDe(fuente, siguienteCaracter(utf8));
    if (!g) continue;
    glifoAlLienzo(fuente, g, x, yBase, ancho, alto, color);
    x += g->xAdvance;
  }

  gfx->draw16bitRGBBitmap(cx - ancho / 2, cy - alto / 2, lienzo, ancho, alto);
  if (modoDepuracion) {
    gfx->drawRect(cx - ancho / 2, cy - alto / 2, ancho, alto, correcto ? COLOR_AMBAR : COLOR_NARANJA);
  }
  return correcto;
}

// ------------------ figuras ------------------

void arco(int16_t rExterior, int16_t rInterior, float gradosInicio, float gradosFin, uint16_t color) {
  if (gradosFin - gradosInicio <= 0.0f) return;
  if (gradosFin - gradosInicio >= 360.0f) { gradosInicio = 0; gradosFin = 359.9f; }
  // La libreria mide desde las 3 en punto; aqui se mide desde las 12.
  gfx->fillArc(CENTRO_X, CENTRO_Y, rExterior, rInterior, gradosInicio - 90.0f, gradosFin - 90.0f, color);
}

void circulo(int16_t x, int16_t y, int16_t radio, uint16_t color)  { gfx->fillCircle(x, y, radio, color); }
void aro(int16_t x, int16_t y, int16_t radio, uint16_t color)      { gfx->drawCircle(x, y, radio, color); }
void rectangulo(int16_t x, int16_t y, int16_t ancho, int16_t alto, uint16_t color) { gfx->fillRect(x, y, ancho, alto, color); }
void linea(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)         { gfx->drawLine(x0, y0, x1, y1, color); }

void imagen(int16_t x, int16_t y, int16_t ancho, int16_t alto, const uint16_t* pixelesProgmem) {
  gfx->draw16bitRGBBitmap(x, y, pixelesProgmem, ancho, alto);
}

// ------------------ disenar en redondo ------------------

bool cabeEnCirculo(int16_t cx, int16_t cy, int16_t ancho, int16_t alto, int16_t radio) {
  // Basta con revisar la esquina mas lejana del centro.
  int32_t dx = abs(cx - CENTRO_X) + ancho / 2;
  int32_t dy = abs(cy - CENTRO_Y) + alto / 2;
  return dx * dx + dy * dy <= (int32_t)radio * radio;
}

int16_t anchoDisponible(int16_t cy, int16_t alto, int16_t radio) {
  int32_t dy = abs(cy - CENTRO_Y) + alto / 2;
  if (dy >= radio) return 0;
  return 2 * (int16_t)sqrtf((float)((int32_t)radio * radio - dy * dy));
}

void depuracion(bool activa) { modoDepuracion = activa; }
bool depuracionActiva()      { return modoDepuracion; }

}  // namespace pantalla

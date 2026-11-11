/*
 * ============================================================
 *  PANTALLA - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "pantalla.h"
#include "config.h"
#include <driver/gpio.h>

Arduino_GFX* gfx = nullptr;
static Arduino_DataBus* bus = nullptr;

static const uint16_t NEGRO  = 0x0000;
static const uint16_t BLANCO = 0xFFFF;
static const uint16_t ROSA   = 0xFA6D;   // (255, 77, 109)
static const uint16_t GRIS   = 0x4208;
static const uint16_t SUAVE  = 0xBDF7;

static const int16_t CX = 120, CY = 120;
static const int16_t COR_Y = 78;         // centro del corazon
static const int16_t COR_CAJA = 50;      // media caja que ocupa al latir

static bool    hayCorazon = false;
static int16_t avanceAnterior = -1;

bool pantallaIniciar() {
  bus = new Arduino_ESP32SPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_SCK, PIN_TFT_MOSI, GFX_NOT_DEFINED);
  gfx = new Arduino_GC9A01(bus, PIN_TFT_RST == SIN_PIN ? GFX_NOT_DEFINED : PIN_TFT_RST,
                           TFT_ROTACION, true /* IPS */);
  if (!gfx->begin(TFT_FRECUENCIA)) return false;
  gfx->fillScreen(NEGRO);
  if (PIN_TFT_BL != SIN_PIN) ledcAttach(PIN_TFT_BL, 5000, 8);
  return true;
}

void pantallaBrillo(uint8_t porciento) {
  if (PIN_TFT_BL == SIN_PIN) return;
  if (porciento > 100) porciento = 100;
  // curva cuadratica: el ojo no ve el brillo de forma lineal
  ledcWrite(PIN_TFT_BL, (uint32_t)porciento * porciento * 255 / 10000);
}

void pantallaApagar() {
  gfx->fillScreen(NEGRO);
  gfx->displayOff();
  if (PIN_TFT_BL != SIN_PIN) {            // la luz de fondo se queda apagada tambien dormido
    ledcDetach(PIN_TFT_BL);
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, LOW);
    gpio_hold_en((gpio_num_t)PIN_TFT_BL);
    gpio_deep_sleep_hold_en();
  }
}

static void centrado(const char* txt, int16_t y, uint8_t tam, uint16_t color) {
  if (!txt || !*txt) return;
  int16_t bx, by; uint16_t bw, bh;
  gfx->setTextSize(tam);
  gfx->setTextColor(color);
  gfx->getTextBounds(txt, 0, 0, &bx, &by, &bw, &bh);
  gfx->setCursor(CX - (int16_t)bw / 2 - bx, y);
  gfx->print(txt);
}

// Corazon por su curva implicita (x^2 + y^2 - 1)^3 - x^2 y^3 <= 0, renglon por
// renglon: cada renglon se pinta completo (fondo + tramo rosa), asi no parpadea.
static void corazon(float medio) {
  const float k = 1.25f / medio;          // la curva cabe en +-1.25
  for (int16_t py = -COR_CAJA; py <= COR_CAJA; py++) {
    float y = -(py - 6) * k;              // el corazon tiene mas cuerpo arriba
    int16_t ini = 0, fin = -1;
    for (int16_t px = 0; px <= COR_CAJA; px++) {
      float x = px * k;
      float a = x * x + y * y - 1.0f;
      if (a * a * a - x * x * y * y * y <= 0.0f) { if (fin < 0) ini = px; fin = px; }
    }
    int16_t ry = COR_Y + py;
    if (fin < 0) { gfx->drawFastHLine(CX - COR_CAJA, ry, 2 * COR_CAJA + 1, NEGRO); continue; }
    gfx->drawFastHLine(CX - COR_CAJA, ry, COR_CAJA - fin, NEGRO);
    gfx->drawFastHLine(CX + fin + 1, ry, COR_CAJA - fin, NEGRO);
    if (ini > 0) {                        // la hendidura de arriba
      gfx->drawFastHLine(CX - fin, ry, fin - ini + 1, ROSA);
      gfx->drawFastHLine(CX - ini + 1, ry, 2 * ini - 1, NEGRO);
      gfx->drawFastHLine(CX + ini, ry, fin - ini + 1, ROSA);
    } else {
      gfx->drawFastHLine(CX - fin, ry, 2 * fin + 1, ROSA);
    }
  }
}

void pantallaMensaje(const char* r1, const char* r2, const char* r3) {
  gfx->fillScreen(NEGRO);
  corazon(40);
  centrado(r1, 142, 2, BLANCO);
  centrado(r2, 168, 2, SUAVE);
  centrado(r3, 194, 2, SUAVE);
  hayCorazon = true;
  avanceAnterior = -1;
}

bool pantallaEsMensaje() { return hayCorazon; }

void pantallaLatir() {
  if (!hayCorazon) return;
  // Doble golpe (lub-dub) cada 0.9 s
  static uint8_t faseAnterior = 255;
  uint32_t t = millis() % 900;
  uint8_t fase = (t < 110) ? 1 : (t < 220) ? 0 : (t < 330) ? 2 : 0;
  if (fase == faseAnterior) return;
  faseAnterior = fase;
  corazon(fase == 1 ? 47 : fase == 2 ? 44 : 40);
}

void pantallaAvance(uint8_t porciento, const char* titulo) {
  if (porciento > 100) porciento = 100;
  if (hayCorazon || avanceAnterior < 0) {
    gfx->fillScreen(NEGRO);
    gfx->fillArc(CX, CY, 116, 106, 0, 360, GRIS);
    centrado(titulo, 84, 2, SUAVE);
    hayCorazon = false;
    avanceAnterior = 0;
  }
  int16_t fin = (int16_t)porciento * 360 / 100;
  if (fin > avanceAnterior) {
    // el anillo arranca arriba (270 grados) y avanza como reloj
    float a0 = 270.0f + avanceAnterior, a1 = 270.0f + fin;
    if (a0 >= 360.0f) { a0 -= 360.0f; a1 -= 360.0f; }
    if (a1 <= 360.0f) gfx->fillArc(CX, CY, 116, 106, a0, a1, ROSA);
    else { gfx->fillArc(CX, CY, 116, 106, a0, 360, ROSA); gfx->fillArc(CX, CY, 116, 106, 0, a1 - 360.0f, ROSA); }
    avanceAnterior = fin;
  }
  char txt[8];
  snprintf(txt, sizeof(txt), "%u%%", porciento);
  gfx->fillRect(CX - 50, 112, 100, 30, NEGRO);
  centrado(txt, 116, 3, BLANCO);
}

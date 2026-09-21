// GC9A01 FALSA: un framebuffer de 240x240 en memoria con las mismas
// primitivas que pantalla.cpp le pide a Arduino_GFX.
// Desarrollado por Tostatronic - Ing. Jorge Alvarado
#pragma once
#include <Arduino.h>
#define GFX_NOT_DEFINED -1

extern uint16_t marcoFalso[240 * 240];

struct Arduino_DataBus {};
struct Arduino_ESP32SPI : Arduino_DataBus { Arduino_ESP32SPI(int, int, int, int, int) {} };

struct Arduino_GFX {
  virtual ~Arduino_GFX() {}
  virtual bool begin(int32_t = -1) { return true; }
  void punto(int x, int y, uint16_t c) { if (x >= 0 && x < 240 && y >= 0 && y < 240) marcoFalso[y * 240 + x] = c; }
  void fillScreen(uint16_t c) { for (auto& p : marcoFalso) p = c; }
  void fillRect(int x, int y, int w, int h, uint16_t c) { for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) punto(x + i, y + j, c); }
  void drawRect(int x, int y, int w, int h, uint16_t c) { for (int i = 0; i < w; i++) { punto(x + i, y, c); punto(x + i, y + h - 1, c); } for (int j = 0; j < h; j++) { punto(x, y + j, c); punto(x + w - 1, y + j, c); } }
  void draw16bitRGBBitmap(int x, int y, const uint16_t* b, int w, int h) { for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) punto(x + i, y + j, b[j * w + i]); }
  void fillCircle(int cx, int cy, int r, uint16_t c) { for (int j = -r; j <= r; j++) for (int i = -r; i <= r; i++) if (i * i + j * j <= r * r) punto(cx + i, cy + j, c); }
  void drawCircle(int cx, int cy, int r, uint16_t c) { for (int a = 0; a < 1440; a++) punto(cx + lround(r * cos(a * M_PI / 720)), cy + lround(r * sin(a * M_PI / 720)), c); }
  void drawLine(int x0, int y0, int x1, int y1, uint16_t c) { int n = std::max(abs(x1 - x0), abs(y1 - y0)); for (int k = 0; k <= n; k++) punto(x0 + (n ? (x1 - x0) * k / n : 0), y0 + (n ? (y1 - y0) * k / n : 0), c); }
  // Misma convencion que la libreria real: 0 grados = las 3 en punto, sentido horario.
  void fillArc(int cx, int cy, int r1, int r2, float ini, float fin, uint16_t c) {
    if (r1 < r2) std::swap(r1, r2);
    ini = fmodf(ini, 360); if (ini < 0) ini += 360; fin = fmodf(fin, 360); if (fin < 0) fin += 360;
    for (int j = -r1; j <= r1; j++) for (int i = -r1; i <= r1; i++) {
      float r = sqrtf(i * i + j * j); if (r > r1 + 0.5f || r < r2 - 0.5f) continue;
      float a = atan2f(j, i) * 180 / M_PI; if (a < 0) a += 360;
      bool dentro = ini <= fin ? (a >= ini && a <= fin) : (a >= ini || a <= fin);
      if (dentro) punto(cx + i, cy + j, c);
    }
  }
};
struct Arduino_GC9A01 : Arduino_GFX { Arduino_GC9A01(Arduino_DataBus*, int, int, bool) {} };

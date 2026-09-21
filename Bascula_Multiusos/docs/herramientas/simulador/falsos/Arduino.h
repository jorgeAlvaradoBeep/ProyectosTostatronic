// Arduino.h FALSO: lo minimo para compilar ui.cpp y pantalla.cpp en la computadora.
// Desarrollado por Tostatronic - Ing. Jorge Alvarado
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdarg>
#include <algorithm>
#include <initializer_list>

#define PROGMEM
#define IRAM_ATTR
#define constrain(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
using std::max; using std::min;

extern uint32_t relojFalsoMs;
inline uint32_t millis() { return relojFalsoMs; }
inline void delay(uint32_t ms) { relojFalsoMs += ms; }
inline bool ledcAttach(int, int, int) { return true; }
inline void ledcWrite(int, int) {}

struct SerialFalso {
  void begin(int) {}
  void println(const char* s = "") { std::printf("%s\n", s); }
  void printf(const char* f, ...) { va_list a; va_start(a, f); std::vprintf(f, a); va_end(a); }
};
extern SerialFalso Serial;

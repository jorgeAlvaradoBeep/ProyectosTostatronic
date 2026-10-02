// NVS FALSA: no guarda nada. Alcanza para que contador.cpp (el real) corra
// en la computadora empezando sin perfiles.
// Desarrollado por Tostatronic - Ing. Jorge Alvarado
#pragma once
#include <Arduino.h>

struct Preferences {
  bool    begin(const char*, bool = false) { return true; }
  void    end() {}
  uint8_t getUChar(const char*, uint8_t porDefecto = 0) { return porDefecto; }
  size_t  putUChar(const char*, uint8_t) { return 1; }
  size_t  getBytes(const char*, void*, size_t) { return 0; }
  size_t  putBytes(const char*, const void*, size_t n) { return n; }
};

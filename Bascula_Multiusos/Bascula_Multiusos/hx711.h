/*
 * ============================================================
 *  HX711 - lectura a prueba de WiFi en chips de UN solo nucleo
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  EL PROBLEMA
 *  El HX711 no es SPI ni I2C: se lee "a mano", 24 pulsos de reloj
 *  y un bit por pulso. Y tiene una trampa: si SCK se queda en ALTO
 *  mas de 60 microsegundos, el chip lo toma como orden de apagado
 *  y se reinicia a media trama.
 *
 *  El ESP32-C6 y el C5 tienen un solo nucleo. Si a medio pulso
 *  llega una interrupcion del WiFi, el reloj se queda en alto lo
 *  que dure esa interrupcion. Resultado: la bascula funciona
 *  perfecto sin WiFi y mete lecturas basura en cuanto se conecta.
 *
 *  LA SOLUCION (primera mitad; la otra mitad es el filtro)
 *  La trama completa se lee dentro de una seccion critica: desde
 *  el primer pulso hasta el ultimo no entra ninguna interrupcion.
 *  Dura ~100 microsegundos, asi que el WiFi ni se entera.
 *
 *  Ademas cada trama se CRONOMETRA. Si alguna vez una tarda de
 *  mas, se cuenta como "trama lenta" y se descarta: es la evidencia
 *  que usa el modo Diagnostico para demostrar que el WiFi no esta
 *  danando las lecturas.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

enum ResultadoHX : uint8_t {
  HX_OK,
  HX_SIN_RESPUESTA,   // DOUT nunca bajo: celda/modulo desconectado o sin alimentacion
  HX_SATURADO,        // lectura en el tope del ADC: celda mal cableada o sobrecargada
  HX_TRAMA_LENTA,     // la trama tardo de mas: lectura descartada
};

class HX711 {
 public:
  void iniciar(int8_t pinDout, int8_t pinSck);

  bool hayDato() const;   // DOUT en bajo = conversion lista

  // Espera (cediendo el CPU) a que haya dato y lo lee. Canal A, ganancia 128.
  ResultadoHX leer(int32_t& crudo, uint16_t esperaMs);

  // Estadistica acumulada, para el modo Diagnostico.
  uint32_t tramasLeidas() const  { return _leidas; }
  uint32_t tramasLentas() const  { return _lentas; }
  uint32_t tramaMaxMicros() const { return _maxMicros; }
  void     reiniciarEstadistica() { _leidas = _lentas = _maxMicros = 0; }

 private:
  int8_t   _dout = -1, _sck = -1;
  uint32_t _leidas = 0, _lentas = 0, _maxMicros = 0;
};

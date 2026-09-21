/*
 * ============================================================
 *  TECLADO DE MEMBRANA 1x4
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Conexion: el comun del teclado a GND y cada tecla a su GPIO.
 *  Las entradas usan INPUT_PULLUP, asi que no hacen falta
 *  resistencias: tecla suelta = ALTO, tecla presionada = BAJO.
 *
 *      1 MENU   |   2 ARRIBA   |   3 ABAJO   |   4 OK
 *
 *  Cada tecla entrega tres tipos de evento:
 *    CORTA    al SOLTAR, si no llego a ser larga
 *    LARGA    una vez, al cumplir TECLA_LARGA_MS presionada
 *    REPETIR  cada TECLA_REPETIR_MS mientras siga presionada
 *             (para el avance rapido de ARRIBA / ABAJO)
 *
 *  Todo es por sondeo con millis(): nada de delay() ni de
 *  interrupciones, para no estorbarle al HX711 ni al WiFi.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

enum Tecla : uint8_t { TECLA_MENU, TECLA_ARRIBA, TECLA_ABAJO, TECLA_OK, NUM_TECLAS, TECLA_NINGUNA };
enum TipoPulsacion : uint8_t { PULSACION_CORTA, PULSACION_LARGA, PULSACION_REPETIR };

struct EventoTecla {
  Tecla         tecla;
  TipoPulsacion tipo;
};

namespace teclado {

void iniciar();

// Llamar seguido desde loop(). Devuelve true y llena 'evento' si hubo uno.
bool leer(EventoTecla& evento);

// Estado fisico ya sin rebote (para la pantalla de prueba de hardware).
bool presionada(Tecla t);

}  // namespace teclado

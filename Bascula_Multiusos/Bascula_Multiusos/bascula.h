/*
 * ============================================================
 *  BASCULA - muestreo de la celda de carga
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Una tarea propia de FreeRTOS lee el HX711 sin parar y deja el
 *  resultado en una "foto" (struct Lectura) que el resto del
 *  programa consulta cuando quiere. Nadie mas toca el HX711.
 *
 *  Por que una tarea y no leer desde loop(): dibujar en la
 *  pantalla tarda varios milisegundos y el HX711 entrega un dato
 *  cada 12.5 ms (a 80 SPS). Desde loop() se perderian muestras
 *  cada vez que se redibuja.
 *
 *  FASE 1: valores crudos y estadistica por segundo.
 *  FASE 2: aqui se agregan filtro, estabilidad, tara y calibracion.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "hx711.h"

struct Lectura {
  ResultadoHX estado        = HX_SIN_RESPUESTA;
  int32_t     crudo         = 0;   // ultima muestra valida
  int32_t     promedio      = 0;   // media de las ultimas muestras (~100 ms)

  // Estadistica del ultimo segundo completo
  float       muestrasPorSegundo = 0;
  int32_t     ruidoPicoPico      = 0;

  // Acumulado desde el arranque (evidencia para el Diagnostico)
  uint32_t    tramasLeidas   = 0;
  uint32_t    tramasLentas   = 0;
  uint32_t    tramaMaxMicros = 0;
};

namespace bascula {

void iniciar();

// Copia consistente de la ultima lectura (segura entre tareas).
Lectura leer();

}  // namespace bascula

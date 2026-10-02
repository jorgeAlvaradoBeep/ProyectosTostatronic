/*
 * ============================================================
 *  BASCULA - muestreo, filtro, estabilidad, cero y tara
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Una tarea propia de FreeRTOS lee el HX711 sin parar y deja el
 *  resultado en una "foto" (struct Lectura) que el resto del
 *  programa consulta cuando quiere. Nadie mas toca el HX711.
 *
 *  Por que una tarea y no leer desde loop(): dibujar en la
 *  pantalla tarda varios milisegundos y el HX711 entrega un dato
 *  cada 100 ms (a 10 SPS; 12.5 ms a 80). Desde loop() se perderian
 *  muestras cada vez que se redibuja.
 *
 *  Camino de cada muestra:
 *    HX711 -> filtro robusto (mediana + descarte de atipicos)
 *          -> deteccion de estabilidad
 *          -> si esta estable, promedio largo (el ultimo digito
 *             deja de bailar) y seguimiento de cero
 *
 *  Todo se guarda en CUENTAS del ADC. Los gramos se calculan solo
 *  al consultar, con la calibracion vigente:
 *
 *      gramos = (cuentas - cero) / factor
 *
 *  El factor puede ser negativo (celda montada al reves o A+/A-
 *  cruzados): la calibracion lo detecta sola.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "hx711.h"

struct Lectura {
  ResultadoHX estado        = HX_SIN_RESPUESTA;
  int32_t     crudo         = 0;   // ultima muestra valida, sin filtrar
  int32_t     filtrado      = 0;   // despues del filtro robusto (y del promedio largo si esta estable)
  bool        estable       = false;
  float       progresoEstable = 0; // 0..1: cuanto falta para darlo por estable
  bool        doutAlAire    = false;  // DOUT suelto: se lee siempre "listo"

  // Estadistica del ultimo segundo completo
  float       muestrasPorSegundo = 0;
  int32_t     ruidoPicoPico      = 0;  // de las muestras crudas

  // Acumulado desde el arranque (evidencia para el Diagnostico)
  uint32_t    atipicos       = 0;  // picos aislados: la firma de una trama danada
  uint32_t    tramasLeidas   = 0;
  uint32_t    tramasLentas   = 0;
  uint32_t    tramaMaxMicros = 0;
};

struct Calibracion {
  float   factor = 0;    // cuentas por gramo (con signo)
  int32_t offset = 0;    // cuentas con el plato vacio al calibrar
  bool    valida = false;
};

// El peso ya en gramos, listo para mostrarse.
struct Peso {
  ResultadoHX estado     = HX_SIN_RESPUESTA;
  bool   hayCelda        = false;   // responde y DOUT no esta al aire
  bool   calibrada       = false;
  bool   estable         = false;
  float  progresoEstable = 0;
  bool   sobrecarga      = false;
  bool   conTara         = false;
  float  brutoG          = 0;       // desde el cero
  float  taraG           = 0;
  float  netoG           = 0;       // bruto - tara: lo que se muestra
  float  divisionG       = 0.1f;
  float  capacidadG      = 1000;
};

namespace bascula {

void iniciar();

// Calibracion y celda en uso. Reinicia el cero (al valor calibrado) y
// quita la tara; el cero de arranque se vuelve a tomar al asentarse.
void configurar(const Calibracion& cal, uint8_t capacidadKg);

// Copia consistente de la ultima lectura (segura entre tareas).
Lectura leer();

Peso peso();

// Solo funcionan con el peso estable: devuelven false si se mueve.
bool tarar();       // lo que hay en el plato pasa a ser la tara
bool cero();        // el plato actual pasa a ser el cero; quita la tara
void quitarTara();

}  // namespace bascula

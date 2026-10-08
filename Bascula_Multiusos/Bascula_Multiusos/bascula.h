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
#include "ruido.h"

struct Lectura {
  ResultadoHX estado        = HX_SIN_RESPUESTA;
  int32_t     crudo         = 0;   // ultima muestra valida, sin filtrar
  int32_t     filtrado      = 0;   // despues del filtro robusto (y del promedio largo si esta estable)
  int32_t     mostrado      = 0;   // filtrado con retencion del ultimo digito: lo que va a la pantalla
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

// Avance y resultado de una escucha (ver bascula::escuchar).
struct Escucha {
  bool     activa   = false;
  float    progreso = 0;     // 0..1
  uint32_t muestras = 0;
  float    media    = 0;     // cuentas
  float    sigma    = 0;     // desviacion estandar de las muestras crudas, en cuentas
  int32_t  picoPico = 0;
};

namespace bascula {

void iniciar();

// Calibracion y celda en uso. Reinicia el cero (al valor calibrado) y
// quita la tara; el cero de arranque se vuelve a tomar al asentarse.
void configurar(const Calibracion& cal, uint8_t capacidadKg);

// Perfil de ruido aprendido (MENU > Afinar ruido): fija la ventana del
// filtro, la banda de estabilidad y el promedio largo. Uno no valido
// regresa a los valores de config.h.
void        perfil(const PerfilRuido& p);
PerfilRuido perfil();

// Escucha la celda durante 'ms': acumula media, desviacion y pico a pico
// de las muestras crudas. escucha() dice como va y, al terminar
// (activa = false con muestras > 0), el resultado. escuchar(0) cancela.
void    escuchar(uint32_t ms);
Escucha escucha();

// Copia consistente de la ultima lectura (segura entre tareas).
Lectura leer();

Peso peso();

// Solo funcionan con el peso estable: devuelven false si se mueve.
bool tarar();       // lo que hay en el plato pasa a ser la tara
bool cero();        // el plato actual pasa a ser el cero; quita la tara
void quitarTara();

}  // namespace bascula

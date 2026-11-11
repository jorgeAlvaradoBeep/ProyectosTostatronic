/*
 * ============================================================
 *  CALIBRACION - la cuenta con tres pesos
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  La misma aritmetica para el asistente de la pantalla y para la
 *  pagina web: aqui se decide que pesos se proponen, cuales se
 *  aceptan, y como salen el factor y el desvio de cada peso.
 *
 *    d_i      = cuentas_i - cero              (lo que movio cada peso)
 *    factor   = sum(g_i * d_i) / sum(g_i^2)   recta por el origen
 *    desvio_i = d_i / factor - g_i            (gramos que se aparta cada peso)
 *
 *  Para senalar al peso que no concuerda no sirve la recta ajustada
 *  (un peso pesado equivocado jala la recta y el desvio se reparte);
 *  se compara cada peso con la MEDIANA de los factores individuales
 *  d_i / g_i: el que mas se aleja de ella es el sospechoso.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"

struct PuntoCal {
  uint32_t gramos  = 0;
  int32_t  cuentas = 0;
};

struct ResultadoCal {
  bool    ok          = false;   // los pesos concuerdan: se puede guardar
  float   factor      = 0;       // cuentas por gramo (con signo)
  float   desvioMaxG  = 0;       // el peor desvio respecto a la recta final
  uint8_t puntoMalo   = 0;       // si !ok: cual repetir (0..CAL_PUNTOS-1)
  float   desvioMaloG = 0;       // y cuanto se aparta de los demas
};

namespace calibracion {

uint32_t capacidadG(uint8_t capacidadKg);

// Peso que se propone para el punto i: el ultimo que se uso si sigue
// siendo valido, si no el porcentaje de config.h. Siempre mayor que
// el punto previo (anteriorG) y deja lugar para los que faltan.
uint32_t propuestaG(uint8_t i, uint8_t capacidadKg, uint32_t anteriorG);

// Menor y mayor peso que se acepta para el punto i, dado el anterior.
uint32_t minimoG(uint8_t i, uint8_t capacidadKg, uint32_t anteriorG);
uint32_t maximoG(uint8_t i, uint8_t capacidadKg);

// Por que no sirve un peso o una lectura (nullptr = sirve). Los textos
// son los "motivos" de la pagina web; la pantalla los traduce.
const char* revisarPeso(uint8_t i, uint32_t gramos, uint8_t capacidadKg, uint32_t anteriorG);
const char* revisarCuentas(uint8_t i, int32_t cuentas, int32_t cero, int32_t anteriorCuentas);

// La cuenta con los CAL_PUNTOS puntos tomados.
ResultadoCal calcular(int32_t cero, const PuntoCal* puntos, float divisionG);

// Escribe la calibracion en los ajustes, la guarda en la NVS y la pone
// a trabajar en la bascula. Recuerda los pesos para la proxima vez.
void guardar(int32_t cero, const PuntoCal* puntos, const ResultadoCal& r);

}  // namespace calibracion

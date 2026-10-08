/*
 * ============================================================
 *  RUIDO - del ruido medido a los parametros del filtro
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  El asistente "Afinar ruido" escucha la celda en reposo y mide
 *  su desviacion estandar (sigma) en cuentas. De ahi salen:
 *
 *    VENTANA del filtro robusto. Promediar n muestras baja el
 *      ruido en raiz de n. Se busca que a la salida del filtro
 *      quede en RUIDO_SIGMA_OBJETIVO_DIV divisiones:
 *          n = (sigma / objetivo)^2, entre 5 y 15, impar.
 *
 *    BANDA de estabilidad. Con un ruido de sigma/raiz(n) a la
 *      salida, pedir +/- 1 division puede ser imposible: la banda
 *      es lo mayor entre 1 division y 3 sigmas del filtrado, asi
 *      la bascula si llega a "estable" con la celda que tiene.
 *
 *    ALFA del promedio largo (ya estable). Un promedio exponencial
 *      baja el ruido en raiz(alfa/2). Se pide que lo que se muestra
 *      quede en RUIDO_PANTALLA_OBJETIVO_DIV divisiones.
 *
 *  Funciones puras: se prueban en la computadora y las usa tanto
 *  bascula.cpp como el simulador.
 * ============================================================
 */

#pragma once
#include <stdint.h>
#include <math.h>
#include "config.h"

// Lo que se aprende escuchando la celda en reposo. Se guarda en la NVS.
struct PerfilRuido {
  bool     valido         = false;
  float    sigmaCuentas   = 0;   // desviacion estandar de las muestras crudas, en reposo
  uint8_t  ventana        = 0;   // muestras del filtro robusto (impar, 5..15)
  uint16_t asentamientoMs = 0;   // lo que tardo en asentarse al poner / quitar un objeto
};

namespace ruido {

const uint8_t VENTANA_MIN = 5, VENTANA_MAX = 15;

inline uint8_t ventanaPara(float sigmaCuentas, float cuentasPorDivision) {
  float sigmaDiv = cuentasPorDivision > 0 ? sigmaCuentas / cuentasPorDivision : 0;
  float n = ceilf((sigmaDiv / RUIDO_SIGMA_OBJETIVO_DIV) * (sigmaDiv / RUIDO_SIGMA_OBJETIVO_DIV));
  if (n < VENTANA_MIN) n = VENTANA_MIN;
  if (n > VENTANA_MAX) n = VENTANA_MAX;
  return (uint8_t)n | 1;
}

inline PerfilRuido proponer(float sigmaCuentas, float cuentasPorDivision, uint16_t asentamientoMs) {
  PerfilRuido p;
  p.valido         = sigmaCuentas >= 0;
  p.sigmaCuentas   = sigmaCuentas;
  p.ventana        = ventanaPara(sigmaCuentas, cuentasPorDivision);
  p.asentamientoMs = asentamientoMs;
  return p;
}

// Ruido que queda a la salida del filtro (en cuentas).
inline float sigmaFiltrada(const PerfilRuido& p) {
  return p.ventana ? p.sigmaCuentas / sqrtf((float)p.ventana) : p.sigmaCuentas;
}

// Banda de estabilidad en cuentas: lo mayor entre 'bandaMinima' (1 division)
// y 3 sigmas de lo que sale del filtro.
inline float banda(const PerfilRuido& p, float bandaMinima) {
  float tresSigma = 3.0f * sigmaFiltrada(p);
  return tresSigma > bandaMinima ? tresSigma : bandaMinima;
}

// Alfa del promedio largo, para que lo mostrado quede en
// RUIDO_PANTALLA_OBJETIVO_DIV divisiones. Entre 0.03 y 'alfaMaxima'.
inline float alfa(const PerfilRuido& p, float cuentasPorDivision, float alfaMaxima) {
  float objetivo = RUIDO_PANTALLA_OBJETIVO_DIV * cuentasPorDivision;
  float sf = sigmaFiltrada(p);
  if (sf <= 0 || objetivo <= 0) return alfaMaxima;
  float a = 2.0f * (objetivo / sf) * (objetivo / sf);
  if (a > alfaMaxima) a = alfaMaxima;
  if (a < 0.03f) a = 0.03f;
  return a;
}

}  // namespace ruido

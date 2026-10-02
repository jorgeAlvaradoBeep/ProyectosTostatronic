/*
 * ============================================================
 *  FILTRO ROBUSTO - mediana + descarte de atipicos
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Un promedio simple se deja arrastrar por UNA sola lectura mala:
 *  si de 5 muestras una llega disparada, el promedio se mueve un
 *  quinto de ese disparo. Aqui se hace en tres pasos:
 *
 *    1. MEDIANA de la ventana: el valor de en medio. Una muestra
 *       disparada queda en un extremo y no la mueve.
 *    2. MAD (desviacion absoluta mediana): que tanto se alejan las
 *       muestras de la mediana, medido tambien con mediana. Es la
 *       "escala del ruido" que no se deja enganar por los atipicos.
 *    3. Se DESCARTAN las muestras a mas de 6 MAD de la mediana y se
 *       PROMEDIAN las demas. Con ruido normal, 6 MAD son unas 4
 *       desviaciones estandar. Con ventanas tan chicas (5 muestras) el
 *       MAD sale muy variable, por eso el margen es amplio: probado con
 *       ruido simulado, tira ~4% de muestras buenas (sin efecto en el
 *       resultado) y cualquier pico real.
 *
 *  El promedio final baja el ruido; la mediana y el descarte evitan
 *  que un pico (una trama mala, un golpe) llegue a la pantalla.
 *
 *  Son funciones puras, sin nada de Arduino: se pueden probar en la
 *  computadora.
 * ============================================================
 */

#pragma once
#include <stdint.h>
#include <stdlib.h>

namespace filtro {

const uint8_t VENTANA_MAX = 16;

struct Resultado {
  int32_t valor;         // promedio de las muestras que sobrevivieron
  int32_t mediana;
  int32_t limite;        // distancia maxima a la mediana para no ser atipico
  uint8_t descartadas;
};

// Insercion: para 16 elementos o menos es mas rapido que qsort.
inline void ordenar(int32_t* v, uint8_t n) {
  for (uint8_t i = 1; i < n; i++) {
    int32_t x = v[i];
    int8_t  j = i - 1;
    while (j >= 0 && v[j] > x) { v[j + 1] = v[j]; j--; }
    v[j + 1] = x;
  }
}

// Ordena 'v' en el lugar y devuelve su mediana.
inline int32_t mediana(int32_t* v, uint8_t n) {
  ordenar(v, n);
  if (n & 1) return v[n / 2];
  return (int32_t)(((int64_t)v[n / 2 - 1] + v[n / 2]) / 2);
}

inline Resultado robusto(const int32_t* muestras, uint8_t n) {
  Resultado r = { 0, 0, 0, 0 };
  if (n == 0) return r;
  if (n > VENTANA_MAX) n = VENTANA_MAX;

  int32_t copia[VENTANA_MAX];
  for (uint8_t i = 0; i < n; i++) copia[i] = muestras[i];
  r.mediana = mediana(copia, n);

  int32_t desvio[VENTANA_MAX];
  for (uint8_t i = 0; i < n; i++) desvio[i] = abs(muestras[i] - r.mediana);
  int32_t mad = mediana(desvio, n);

  // El +2 evita que, con muestras casi identicas (MAD = 0), se tire una
  // muestra solo por diferir en una cuenta.
  r.limite = 6 * mad + 2;

  int64_t suma = 0;
  uint8_t usadas = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (abs(muestras[i] - r.mediana) <= r.limite) { suma += muestras[i]; usadas++; }
    else r.descartadas++;
  }
  // Con n impar la muestra mediana tiene desvio 0, asi que 'usadas' >= 1.
  r.valor = usadas ? (int32_t)(suma / usadas) : r.mediana;
  return r;
}

}  // namespace filtro

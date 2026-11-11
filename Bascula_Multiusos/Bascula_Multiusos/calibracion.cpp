/*
 * ============================================================
 *  CALIBRACION - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "calibracion.h"
#include "ajustes.h"
#include "bascula.h"

namespace calibracion {

uint32_t capacidadG(uint8_t capacidadKg) { return capacidadKg * 1000UL; }

uint32_t maximoG(uint8_t i, uint8_t capacidadKg) {
  // Los puntos que faltan necesitan al menos un gramo mas cada uno.
  return capacidadG(capacidadKg) - (CAL_PUNTOS - 1 - i);
}

uint32_t minimoG(uint8_t i, uint8_t capacidadKg, uint32_t anteriorG) {
  uint32_t cap    = capacidadG(capacidadKg);
  uint32_t minimo = (uint32_t)ceilf(cap * CAL_PESO_MIN_PORCIENTO / 100.0f);
  if (i > 0 && anteriorG + 1 > minimo) minimo = anteriorG + 1;
  if (i == CAL_PUNTOS - 1) {
    uint32_t alto = (uint32_t)ceilf(cap * CAL_PESO_ALTO_PORCIENTO / 100.0f);
    if (alto > minimo) minimo = alto;
  }
  return minimo;
}

uint32_t propuestaG(uint8_t i, uint8_t capacidadKg, uint32_t anteriorG) {
  uint32_t cap = capacidadG(capacidadKg);
  uint32_t g   = ajustes::actual().pesoCalG[i];
  if (g == 0 || g > cap) g = (uint32_t)roundf(cap * CAL_PROPUESTA_PORCIENTO[i] / 100.0f);
  uint32_t minimo = minimoG(i, capacidadKg, anteriorG), maximo = maximoG(i, capacidadKg);
  if (g < minimo) g = minimo;
  if (g > maximo) g = maximo;
  return g;
}

const char* revisarPeso(uint8_t i, uint32_t gramos, uint8_t capacidadKg, uint32_t anteriorG) {
  if (gramos > maximoG(i, capacidadKg)) return "peso_invalido";
  if (i > 0 && gramos <= anteriorG)     return "orden_pesos";
  if (gramos < minimoG(i, capacidadKg, anteriorG)) return i == CAL_PUNTOS - 1 ? "peso_alto" : "peso_invalido";
  return nullptr;
}

const char* revisarCuentas(uint8_t i, int32_t cuentas, int32_t cero, int32_t anteriorCuentas) {
  if (abs(cuentas - cero) < CAL_CUENTAS_MINIMAS)                     return "poco_cambio";
  if (i > 0 && abs(cuentas - anteriorCuentas) < CAL_CUENTAS_MINIMAS) return "poco_cambio";
  return nullptr;
}

ResultadoCal calcular(int32_t cero, const PuntoCal* p, float divisionG) {
  ResultadoCal r;

  // Factor de cada peso por separado y su mediana.
  float f[CAL_PUNTOS];
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) f[i] = (float)(p[i].cuentas - cero) / p[i].gramos;
  float orden[CAL_PUNTOS];
  memcpy(orden, f, sizeof(orden));
  for (uint8_t i = 1; i < CAL_PUNTOS; i++)
    for (uint8_t j = i; j > 0 && orden[j - 1] > orden[j]; j--) { float t = orden[j]; orden[j] = orden[j - 1]; orden[j - 1] = t; }
  float mediana = orden[CAL_PUNTOS / 2];
  if (!(fabsf(mediana) > 0.01f)) { r.ok = false; r.puntoMalo = 0; return r; }

  // El que mas se aparta de la mediana, en gramos, si se pasa de la tolerancia.
  float peor = 0;
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) {
    float desvio     = (p[i].cuentas - cero) / mediana - (float)p[i].gramos;
    float tolerancia = max(p[i].gramos * CAL_DESVIO_MAX_PORCIENTO / 100.0f, CAL_DESVIO_MAX_DIV * divisionG);
    if (fabsf(desvio) > tolerancia && fabsf(desvio) > fabsf(peor)) {
      peor = desvio;
      r.puntoMalo   = i;
      r.desvioMaloG = desvio;
    }
  }
  if (peor != 0) { r.ok = false; return r; }

  // Recta por el origen: factor = sum(g*d) / sum(g^2). En doble para que
  // no se pierdan cifras (las cuentas llegan a millones).
  double sgd = 0, sgg = 0;
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) {
    double g = p[i].gramos, d = (double)(p[i].cuentas - cero);
    sgd += g * d;
    sgg += g * g;
  }
  r.factor = (float)(sgd / sgg);
  r.ok     = fabsf(r.factor) > 0.01f;
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) {
    float desvio = fabsf((p[i].cuentas - cero) / r.factor - (float)p[i].gramos);
    if (desvio > r.desvioMaxG) r.desvioMaxG = desvio;
  }
  return r;
}

void guardar(int32_t cero, const PuntoCal* p, const ResultadoCal& r) {
  Ajustes& a = ajustes::actual();
  a.cal.factor  = r.factor;
  a.cal.offset  = cero;
  a.cal.desvioG = r.desvioMaxG;
  a.cal.valida  = true;
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) a.pesoCalG[i] = p[i].gramos;
  ajustes::guardar();
  bascula::configurar(a.cal, a.capacidadKg);

  Serial.printf("[calibracion] cero=%ld", (long)cero);
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) Serial.printf("  %lu g=%ld", (unsigned long)p[i].gramos, (long)p[i].cuentas);
  Serial.printf("  factor=%.3f cuentas/g  desvio max=%.2f g\n", r.factor, r.desvioMaxG);
}

}  // namespace calibracion

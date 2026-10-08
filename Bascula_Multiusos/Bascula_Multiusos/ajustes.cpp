/*
 * ============================================================
 *  AJUSTES - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "ajustes.h"
#include "config.h"
#include <Preferences.h>

namespace {

const char* ESPACIO = "bascula";   // "carpeta" de la NVS para este proyecto
Ajustes datos;

bool capacidadValida(uint8_t kg) {
  for (uint8_t i = 0; i < NUM_CELDAS; i++) if (CELDAS[i].capacidadKg == kg) return true;
  return false;
}

}  // namespace

namespace ajustes {

void cargar() {
  Preferences nvs;
  nvs.begin(ESPACIO, false);   // false: la crea si es el primer arranque
  datos.cal.valida  = nvs.getBool("cal", false);
  datos.cal.factor  = nvs.getFloat("factor", 0);
  datos.cal.offset  = nvs.getInt("offset", 0);
  datos.capacidadKg = nvs.getUChar("capKg", CELDAS[0].capacidadKg);
  datos.unidad      = (Unidad)nvs.getUChar("unidad", UNIDAD_G);
  datos.pesoCalG    = nvs.getUInt("pesoCal", 500);
  datos.brillo      = nvs.getUChar("brillo", TFT_BRILLO);
  datos.ruido.valido         = nvs.getBool("ruidoOk", false);
  datos.ruido.sigmaCuentas   = nvs.getFloat("ruidoSig", 0);
  datos.ruido.ventana        = nvs.getUChar("ruidoVen", 0);
  datos.ruido.asentamientoMs = nvs.getUShort("ruidoAs", 0);
  nvs.end();

  // Nada de lo leido se usa a ciegas: un factor 0 o NaN dividiria entre cero.
  if (!(fabsf(datos.cal.factor) > 0.01f) || isnan(datos.cal.factor)) datos.cal.valida = false;
  if (isnan(datos.ruido.sigmaCuentas) || datos.ruido.sigmaCuentas < 0 ||
      datos.ruido.ventana < ruido::VENTANA_MIN || datos.ruido.ventana > ruido::VENTANA_MAX) datos.ruido.valido = false;
  if (!capacidadValida(datos.capacidadKg)) datos.capacidadKg = CELDAS[0].capacidadKg;
  if (datos.unidad >= NUM_UNIDADES) datos.unidad = UNIDAD_G;
}

void guardar() {
  Preferences nvs;
  nvs.begin(ESPACIO, false);
  nvs.putBool("cal", datos.cal.valida);
  nvs.putFloat("factor", datos.cal.factor);
  nvs.putInt("offset", datos.cal.offset);
  nvs.putUChar("capKg", datos.capacidadKg);
  nvs.putUChar("unidad", datos.unidad);
  nvs.putUInt("pesoCal", datos.pesoCalG);
  nvs.putUChar("brillo", datos.brillo);
  nvs.putBool("ruidoOk", datos.ruido.valido);
  nvs.putFloat("ruidoSig", datos.ruido.sigmaCuentas);
  nvs.putUChar("ruidoVen", datos.ruido.ventana);
  nvs.putUShort("ruidoAs", datos.ruido.asentamientoMs);
  nvs.end();
}

Ajustes& actual() { return datos; }

}  // namespace ajustes

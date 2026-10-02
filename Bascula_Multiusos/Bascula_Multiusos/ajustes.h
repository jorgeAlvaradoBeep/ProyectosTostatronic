/*
 * ============================================================
 *  AJUSTES - lo que sobrevive a un apagado (NVS)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Se guarda en la NVS del ESP32 con la libreria Preferences: una
 *  zona de la Flash pensada para pares clave/valor, con reparto de
 *  desgaste. Aguanta cientos de miles de escrituras, pero aun asi
 *  solo se escribe cuando algo cambia, nunca en cada lectura.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "bascula.h"

enum Unidad : uint8_t { UNIDAD_G, UNIDAD_KG, UNIDAD_OZ, NUM_UNIDADES };

struct Ajustes {
  Calibracion cal;
  uint8_t     capacidadKg = 1;
  Unidad      unidad      = UNIDAD_G;
  uint32_t    pesoCalG    = 500;   // ultimo peso de calibracion: se propone la siguiente vez
  uint8_t     brillo      = 200;
};

namespace ajustes {

void     cargar();
void     guardar();
Ajustes& actual();

}  // namespace ajustes

/*
 * ============================================================
 *  TECLADO DE MEMBRANA 1x4 - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "teclado.h"
#include "config.h"

namespace {

const int8_t PINES[NUM_TECLAS] = { PIN_TECLA_1, PIN_TECLA_2, PIN_TECLA_3, PIN_TECLA_4 };

struct EstadoTecla {
  bool     lectura     = false;   // ultima lectura cruda
  bool     estable     = false;   // estado ya sin rebote
  bool     fueLarga    = false;
  uint32_t cambioMs    = 0;       // cuando cambio la lectura cruda
  uint32_t presionMs   = 0;       // cuando se dio por presionada
  uint32_t repeticionMs = 0;
};

EstadoTecla teclas[NUM_TECLAS];

}  // namespace

namespace teclado {

void iniciar() {
  for (uint8_t i = 0; i < NUM_TECLAS; i++) pinMode(PINES[i], INPUT_PULLUP);
}

bool presionada(Tecla t) {
  return t < NUM_TECLAS && teclas[t].estable;
}

bool leer(EventoTecla& evento) {
  uint32_t ahora = millis();

  for (uint8_t i = 0; i < NUM_TECLAS; i++) {
    EstadoTecla& k = teclas[i];
    bool lectura = digitalRead(PINES[i]) == LOW;

    // Antirrebote: el cambio solo cuenta si la lectura se sostiene.
    if (lectura != k.lectura) {
      k.lectura  = lectura;
      k.cambioMs = ahora;
    }
    if (lectura != k.estable && ahora - k.cambioMs >= TECLA_REBOTE_MS) {
      k.estable = lectura;

      if (k.estable) {                       // se acaba de presionar
        k.presionMs = ahora;
        k.fueLarga  = false;
      } else if (!k.fueLarga) {              // se solto sin llegar a larga
        evento = { (Tecla)i, PULSACION_CORTA };
        return true;
      }
    }

    if (!k.estable) continue;

    if (!k.fueLarga && ahora - k.presionMs >= TECLA_LARGA_MS) {
      k.fueLarga     = true;
      k.repeticionMs = ahora;
      evento = { (Tecla)i, PULSACION_LARGA };
      return true;
    }
    if (k.fueLarga && ahora - k.repeticionMs >= TECLA_REPETIR_MS) {
      k.repeticionMs = ahora;
      evento = { (Tecla)i, PULSACION_REPETIR };
      return true;
    }
  }
  return false;
}

}  // namespace teclado

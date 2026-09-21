/*
 * ============================================================
 *  BASCULA - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "bascula.h"
#include "config.h"

namespace {

HX711   celda;
Lectura foto;
portMUX_TYPE candadoFoto = portMUX_INITIALIZER_UNLOCKED;

// Media corta para que el numero en pantalla no tiemble: ~100 ms de muestras.
const uint8_t VENTANA_PROMEDIO = max(1, HX_MUESTRAS_POR_SEGUNDO / 10);

void publicar(const Lectura& nueva) {
  portENTER_CRITICAL(&candadoFoto);
  foto = nueva;
  portEXIT_CRITICAL(&candadoFoto);
}

void tareaBascula(void*) {
  Lectura  actual;
  int32_t  ventana[16] = {0};
  uint8_t  indice = 0, llenas = 0;

  uint32_t inicioSegundo = millis();
  uint16_t muestrasSegundo = 0;
  int32_t  minimo = INT32_MAX, maximo = INT32_MIN;

  while (true) {
    int32_t crudo = 0;
    actual.estado = celda.leer(crudo, HX_ESPERA_MS);

    if (actual.estado == HX_OK) {
      actual.crudo = crudo;

      ventana[indice] = crudo;
      indice = (indice + 1) % VENTANA_PROMEDIO;
      if (llenas < VENTANA_PROMEDIO) llenas++;

      int64_t suma = 0;
      for (uint8_t i = 0; i < llenas; i++) suma += ventana[i];
      actual.promedio = (int32_t)(suma / llenas);

      muestrasSegundo++;
      if (crudo < minimo) minimo = crudo;
      if (crudo > maximo) maximo = crudo;
    } else if (actual.estado == HX_SIN_RESPUESTA) {
      llenas = indice = 0;          // al reconectar no se mezclan muestras viejas
    }

    uint32_t transcurrido = millis() - inicioSegundo;
    if (transcurrido >= 1000) {
      actual.muestrasPorSegundo = muestrasSegundo * 1000.0f / transcurrido;
      actual.ruidoPicoPico      = muestrasSegundo ? (maximo - minimo) : 0;
      inicioSegundo   = millis();
      muestrasSegundo = 0;
      minimo = INT32_MAX;
      maximo = INT32_MIN;
    }

    actual.tramasLeidas   = celda.tramasLeidas();
    actual.tramasLentas   = celda.tramasLentas();
    actual.tramaMaxMicros = celda.tramaMaxMicros();
    publicar(actual);

    // Respiro obligatorio. Con el HX711 bien conectado no cuesta nada (el
    // siguiente dato tarda 12.5 ms), pero si DOUT queda al aire y se lee
    // siempre en bajo, sin esto la tarea acapararia el CPU y dejaria sin
    // tiempo a la pantalla y al WiFi.
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

}  // namespace

namespace bascula {

void iniciar() {
  static_assert(HX_MUESTRAS_POR_SEGUNDO / 10 <= 16, "la ventana del promedio no cabe: sube el arreglo");

  celda.iniciar(PIN_HX_DOUT, PIN_HX_SCK);

  // Prioridad por encima de loop() (1) y del servidor web (5): cuando hay
  // dato listo se atiende ya. No acapara el CPU: casi todo el tiempo esta
  // dormida esperando al HX711.
  xTaskCreate(tareaBascula, "bascula", 4096, nullptr, 6, nullptr);
}

Lectura leer() {
  portENTER_CRITICAL(&candadoFoto);
  Lectura copia = foto;
  portEXIT_CRITICAL(&candadoFoto);
  return copia;
}

}  // namespace bascula

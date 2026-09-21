/*
 * ============================================================
 *  HX711 - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "hx711.h"
#include "driver/gpio.h"
#include "esp_cpu.h"
#include "esp_rom_sys.h"

namespace {

// El HX711 se apaga si SCK pasa de 60 us en alto. Se deja margen.
const uint32_t PULSO_ALTO_MAX_US = 50;

portMUX_TYPE candado = portMUX_INITIALIZER_UNLOCKED;

}  // namespace

void HX711::iniciar(int8_t pinDout, int8_t pinSck) {
  _dout = pinDout;
  _sck  = pinSck;

  pinMode(_dout, INPUT);
  pinMode(_sck, OUTPUT);
  digitalWrite(_sck, LOW);   // SCK en bajo = HX711 despierto y convirtiendo
  reiniciarEstadistica();
}

bool HX711::hayDato() const {
  return gpio_get_level((gpio_num_t)_dout) == 0;
}

// IRAM_ATTR: el lazo de la trama corre desde RAM, no desde la Flash.
ResultadoHX IRAM_ATTR HX711::leer(int32_t& crudo, uint16_t esperaMs) {
  // La espera va FUERA de la seccion critica y cediendo el CPU: aqui es
  // donde el WiFi y la pantalla aprovechan para trabajar.
  uint32_t inicio = millis();
  while (!hayDato()) {
    if (millis() - inicio > esperaMs) return HX_SIN_RESPUESTA;
    vTaskDelay(1);
  }

  const gpio_num_t dout = (gpio_num_t)_dout;
  const gpio_num_t sck  = (gpio_num_t)_sck;
  const uint32_t ciclosPorMicro = getCpuFrequencyMhz();

  uint32_t valor = 0;
  uint32_t altoMaxCiclos = 0;

  // "Calentamiento": las funciones de GPIO viven en la Flash. Llamarlas una
  // vez aqui (sin efecto: SCK ya esta en bajo) las deja en la cache, para
  // que el primer pulso de la trama no pague una lectura de Flash con SCK
  // en alto.
  gpio_set_level(sck, 0);
  (void)gpio_get_level(dout);

  // ---- seccion critica: ~100 us sin interrupciones ----
  // Se usa gpio_set_level / gpio_get_level (y no digitalWrite) porque son
  // mas rapidas y de duracion pareja: la seccion critica queda mas corta.
  portENTER_CRITICAL(&candado);
  uint32_t inicioTrama = esp_cpu_get_cycle_count();

  for (uint8_t i = 0; i < 25; i++) {        // 24 bits de dato + 1 pulso = canal A, ganancia 128
    uint32_t sube = esp_cpu_get_cycle_count();
    gpio_set_level(sck, 1);
    esp_rom_delay_us(1);
    if (i < 24) valor = (valor << 1) | (uint32_t)gpio_get_level(dout);
    gpio_set_level(sck, 0);
    uint32_t enAlto = esp_cpu_get_cycle_count() - sube;
    if (enAlto > altoMaxCiclos) altoMaxCiclos = enAlto;
    esp_rom_delay_us(1);
  }

  uint32_t ciclosTrama = esp_cpu_get_cycle_count() - inicioTrama;
  portEXIT_CRITICAL(&candado);
  // ---- fin de la seccion critica ----

  _leidas++;
  uint32_t microsTrama = ciclosTrama / ciclosPorMicro;
  if (microsTrama > _maxMicros) _maxMicros = microsTrama;

  // Si algun pulso estuvo demasiado tiempo en alto, el HX711 pudo haberse
  // reiniciado a media trama: el valor no es de fiar.
  if (altoMaxCiclos / ciclosPorMicro > PULSO_ALTO_MAX_US) {
    _lentas++;
    return HX_TRAMA_LENTA;
  }

  // 24 bits en complemento a dos -> int32 con signo
  if (valor & 0x800000UL) valor |= 0xFF000000UL;
  crudo = (int32_t)valor;

  if (crudo == 0x7FFFFF || crudo == -0x800000) return HX_SATURADO;
  return HX_OK;
}

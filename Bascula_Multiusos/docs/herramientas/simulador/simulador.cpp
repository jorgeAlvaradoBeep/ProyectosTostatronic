// ============================================================
//  SIMULADOR DE LA PANTALLA REDONDA
//  Desarrollado por Tostatronic - Ing. Jorge Alvarado
// ============================================================
//  Compila el ui.cpp y el pantalla.cpp REALES del sketch contra una
//  GC9A01 falsa y guarda cada pantalla como imagen. Sirve para revisar
//  el diseno (que todo quepa en el circulo) sin tener la placa.
//  Se ejecuta con:  bash docs/herramientas/simulador/simular.sh
// ============================================================
#include <Arduino.h>
#include "config.h"
#include "pantalla.h"
#include "ui.h"

uint32_t    relojFalsoMs = 1000;
SerialFalso Serial;
uint16_t    marcoFalso[240 * 240];

// --- dobles de prueba de los modulos de hardware ---
static Lectura lecturaFalsa;
static bool    teclaFalsa[4];
namespace bascula { void iniciar() {} Lectura leer() { return lecturaFalsa; } }
namespace teclado { void iniciar() {} bool leer(EventoTecla&) { return false; } bool presionada(Tecla t) { return teclaFalsa[t]; } }

static void guardar(const char* nombre) {
  char ruta[128]; snprintf(ruta, sizeof(ruta), "salida/%s.ppm", nombre);
  FILE* f = fopen(ruta, "wb"); fprintf(f, "P6 240 240 255\n");
  for (uint16_t p : marcoFalso) { uint8_t rgb[3] = { uint8_t((p >> 11) << 3), uint8_t(((p >> 5) & 0x3F) << 2), uint8_t((p & 0x1F) << 3) }; fwrite(rgb, 1, 3, f); }
  fclose(f); printf("-> %s\n", ruta);
}

static Lectura lectura(ResultadoHX e, int32_t crudo, float sps, int32_t ruido, uint32_t lentas = 0) {
  Lectura l; l.estado = e; l.crudo = l.promedio = crudo; l.muestrasPorSegundo = sps; l.ruidoPicoPico = ruido; l.tramasLentas = lentas; return l;
}

int main() {
  pantalla::iniciar();

  ui::arranque(2500);                                   guardar("01_arranque");

  ui::pruebaHardwareEntrar();
  lecturaFalsa = lectura(HX_OK, 8312345, 80.1f, 412);   // crudo completo: no cabe en grande
  ui::pruebaHardwareRefrescar(lecturaFalsa);            guardar("02_prueba_crudo");

  ui::pruebaHardwareTecla({ TECLA_OK, PULSACION_CORTA });   // toma el cero
  lecturaFalsa = lectura(HX_OK, 8312345 + 734512, 79.9f, 388); teclaFalsa[3] = true;
  ui::pruebaHardwareRefrescar(lecturaFalsa);            guardar("03_prueba_desde_cero");

  teclaFalsa[3] = false; lecturaFalsa = lectura(HX_OK, 8312345 - 1888888, 80.0f, 40000, 3);
  ui::pruebaHardwareRefrescar(lecturaFalsa);            guardar("04_prueba_negativo_ancho");

  lecturaFalsa = lectura(HX_SIN_RESPUESTA, 0, 0, 0);
  ui::pruebaHardwareRefrescar(lecturaFalsa);            guardar("05_sin_celda");

  ui::pruebaHardwareTecla({ TECLA_MENU, PULSACION_LARGA });  // contornos de depuracion
  relojFalsoMs += 5000;
  lecturaFalsa = lectura(HX_OK, 8312345 + 123456, 80.0f, 401);
  ui::pruebaHardwareRefrescar(lecturaFalsa);            guardar("06_zonas_depuracion");
  return 0;
}

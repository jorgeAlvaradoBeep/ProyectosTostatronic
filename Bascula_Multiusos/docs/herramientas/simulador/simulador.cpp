// ============================================================
//  SIMULADOR DE LA PANTALLA REDONDA
//  Desarrollado por Tostatronic - Ing. Jorge Alvarado
// ============================================================
//  Compila las vistas (ui*.cpp) y el pantalla.cpp REALES del sketch
//  contra una GC9A01 falsa y guarda cada pantalla como imagen. Sirve
//  para revisar el diseno (que todo quepa en el circulo) sin tener la
//  placa. Cualquier zona que se salga lo reporta pantalla.cpp con un
//  renglon "[pantalla] ..." en la salida.
//  Se ejecuta con:  bash docs/herramientas/simulador/simular.sh
// ============================================================
#include <Arduino.h>
#include "config.h"
#include "pantalla.h"
#include "bascula.h"
#include "ajustes.h"
#include "prueba_wifi.h"
#include "ui.h"

uint32_t    relojFalsoMs = 1000;
SerialFalso Serial;
uint16_t    marcoFalso[240 * 240];

// --- dobles de prueba de los modulos de hardware ---
static Lectura lecturaFalsa;
static Peso    pesoFalso;
static bool    teclaFalsa[4];
static bool    wifiFalso = false;
static Ajustes ajustesFalsos;

namespace bascula {
void iniciar() {}
void configurar(const Calibracion&, uint8_t) {}
Lectura leer() { return lecturaFalsa; }
Peso peso() { return pesoFalso; }
bool tarar() { return true; }
bool cero() { return true; }
void quitarTara() {}
}
namespace ajustes { void cargar() {} void guardar() {} Ajustes& actual() { return ajustesFalsos; } }
namespace teclado { void iniciar() {} bool leer(EventoTecla&) { return false; } bool presionada(Tecla t) { return teclaFalsa[t]; } }
namespace pruebaWifi {
void iniciar() { wifiFalso = true; } void detener() { wifiFalso = false; } bool activa() { return wifiFalso; }
uint32_t paquetesPorSegundo() { return 842; } uint32_t paquetesTotales() { return 0; }
}

static void guardar(const char* nombre) {
  char ruta[128]; snprintf(ruta, sizeof(ruta), "salida/%s.ppm", nombre);
  FILE* f = fopen(ruta, "wb"); fprintf(f, "P6 240 240 255\n");
  for (uint16_t p : marcoFalso) { uint8_t rgb[3] = { uint8_t((p >> 11) << 3), uint8_t(((p >> 5) & 0x3F) << 2), uint8_t((p & 0x1F) << 3) }; fwrite(rgb, 1, 3, f); }
  fclose(f); printf("-> %s\n", ruta);
}

static Lectura lectura(ResultadoHX e, int32_t filtrado, float sps, int32_t ruido, bool estable, float progreso = 1) {
  Lectura l; l.estado = e; l.crudo = l.filtrado = filtrado; l.muestrasPorSegundo = sps; l.ruidoPicoPico = ruido;
  l.estable = estable; l.progresoEstable = progreso; return l;
}

static Peso peso(float netoG, bool estable, float taraG = 0) {
  Peso p; p.estado = HX_OK; p.hayCelda = true; p.calibrada = true; p.estable = estable; p.progresoEstable = estable ? 1 : 0.4f;
  p.taraG = taraG; p.conTara = taraG != 0; p.netoG = netoG; p.brutoG = netoG + taraG; p.divisionG = 0.1f; p.capacidadG = 1000; return p;
}

static void tecla(Tecla t, TipoPulsacion tipo = PULSACION_CORTA) { EventoTecla e = { t, tipo }; ui::tecla(e); }

int main() {
  pantalla::iniciar();
  ajustesFalsos.cal.valida = true; ajustesFalsos.cal.factor = -681.2f; ajustesFalsos.pesoCalG = 441;

  ui::arranque(2500);                                                     guardar("01_arranque");

  // ---- pesar ----
  ui::iniciar();
  lecturaFalsa = lectura(HX_OK, -352530, 10.4f, 120, true);
  pesoFalso = peso(441.0f, true);                 ui::refrescar();        guardar("02_pesar_estable");
  pesoFalso = peso(-12.3f, false);                ui::refrescar();        guardar("03_pesar_midiendo_negativo");
  pesoFalso = peso(318.6f, true, 122.4f);         ui::refrescar();        guardar("04_pesar_neto_con_tara");
  ajustesFalsos.unidad = UNIDAD_KG; pesoFalso = peso(999.9f, true);  ui::refrescar(); guardar("05_pesar_kg");
  ajustesFalsos.unidad = UNIDAD_OZ;                                  ui::refrescar(); guardar("06_pesar_oz");
  ajustesFalsos.unidad = UNIDAD_G;
  wifiFalso = true; tecla(TECLA_OK); pesoFalso.estable = false;      ui::refrescar(); guardar("07_pesar_tara_pendiente_wifi");
  wifiFalso = false;
  ui::tecla({ TECLA_MENU, PULSACION_LARGA });   // reinicia la vista (quita el pendiente)
  pesoFalso = peso(0, true); pesoFalso.sobrecarga = true;            ui::refrescar(); guardar("08_pesar_sobrecarga");
  pesoFalso = Peso(); pesoFalso.hayCelda = true;                     ui::refrescar(); guardar("09_pesar_sin_calibrar");
  pesoFalso = Peso();                                                ui::refrescar(); guardar("10_pesar_sin_celda");

  // ---- menu y ajustes ----
  pesoFalso = peso(441.0f, true);
  tecla(TECLA_MENU);                                                 ui::refrescar(); guardar("11_menu");
  tecla(TECLA_ABAJO); tecla(TECLA_ABAJO); tecla(TECLA_ABAJO);         ui::refrescar(); guardar("12_menu_diagnostico");
  tecla(TECLA_ARRIBA); tecla(TECLA_OK);                              ui::refrescar(); guardar("13_ajustes");
  tecla(TECLA_ABAJO); tecla(TECLA_OK);                               ui::refrescar(); guardar("14_ajustes_celda_cambiada");

  // ---- asistente de calibracion ----
  ajustesFalsos.capacidadKg = 1;
  ui::tecla({ TECLA_MENU, PULSACION_LARGA }); tecla(TECLA_MENU); tecla(TECLA_ARRIBA); tecla(TECLA_OK);  // menu (recuerda Ajustes) -> Calibrar
  lecturaFalsa = lectura(HX_OK, -52145, 10.4f, 120, false, 0.6f);    ui::refrescar(); guardar("15_calibrar_paso1_asentandose");
  lecturaFalsa.estable = true; lecturaFalsa.progresoEstable = 1;     ui::refrescar(); guardar("16_calibrar_paso1_estable");
  tecla(TECLA_OK);                                                   // toma el cero
  lecturaFalsa = lectura(HX_OK, -352530, 10.4f, 150, true);          ui::refrescar(); guardar("17_calibrar_paso2");
  tecla(TECLA_OK);                                                   ui::refrescar(); guardar("18_calibrar_paso3_valor");
  tecla(TECLA_OK);                                                   ui::refrescar(); guardar("19_calibrar_listo");

  // Primer arranque, sin calibracion
  ajustesFalsos.cal.valida = false;
  lecturaFalsa = lectura(HX_OK, -52145, 10.4f, 120, false, 0.3f);
  ui::iniciar();                                                     ui::refrescar(); guardar("20_primer_arranque");
  ajustesFalsos.cal.valida = true;

  // ---- diagnostico ----
  ui::tecla({ TECLA_MENU, PULSACION_LARGA }); tecla(TECLA_MENU); tecla(TECLA_ABAJO); tecla(TECLA_ABAJO); tecla(TECLA_OK);  // Calibrar -> Diagnostico
  relojFalsoMs += 5000;
  lecturaFalsa = lectura(HX_OK, -52145, 10.4f, 142, true); teclaFalsa[1] = true;
  ui::refrescar();                                                   guardar("21_diagnostico");
  teclaFalsa[1] = false; tecla(TECLA_ARRIBA); relojFalsoMs += 5000;  // enciende la prueba de WiFi
  ui::refrescar();                                                   guardar("22_diagnostico_wifi");
  tecla(TECLA_ABAJO); relojFalsoMs += 5000;                          // contornos de las zonas
  ui::refrescar();                                                   guardar("23_diagnostico_zonas");
  return 0;
}

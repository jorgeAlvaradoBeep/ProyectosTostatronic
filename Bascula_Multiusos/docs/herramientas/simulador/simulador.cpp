// ============================================================
//  SIMULADOR DE LA PANTALLA REDONDA
//  Desarrollado por Tostatronic - Ing. Jorge Alvarado
// ============================================================
//  Compila las vistas (ui*.cpp), contador.cpp y el pantalla.cpp REALES del sketch
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
#include "red.h"
#include "servidor_web.h"
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
static InfoRed       redFalsa;
static bool          redEncendida = true;
static EventoRed     eventoFalso = EVENTO_RED_NINGUNO;
static Actualizacion otaFalsa;

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
namespace red {
void iniciar() {} void atender() {}
InfoRed info() { return redFalsa; }
EventoRed evento() { EventoRed e = eventoFalso; eventoFalso = EVENTO_RED_NINGUNO; return e; }
bool encendida() { return redEncendida; } void encender(bool si) { redEncendida = si; } void olvidar() {}
bool probar(const char*, const char*) { return false; } void actividad() {} void ipVista() {} void olvidarEnBreve() {}
}
namespace web {
void arrancar() {} void detener() {} void atender() {}
void permitirActualizacion(bool) {} Actualizacion actualizacion() { return otaFalsa; } void alAvanzar(void (*)()) {}
void permitirCalibracion(bool) {} void confirmarFirmware() {}
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

static InfoRed conectada(const char* ssid, const char* ip, int8_t rssi, uint8_t canal, const char* estandar) {
  InfoRed r; r.estado = RED_CONECTADA; r.hayGuardada = true; r.rssi = rssi; r.canal = canal; r.banda5 = canal > 14; r.estandar = estandar;
  snprintf(r.ssid, sizeof(r.ssid), "%s", ssid); snprintf(r.ip, sizeof(r.ip), "%s", ip); return r;
}

static Actualizacion ota(EstadoOta e, uint32_t restanteMs = 0, uint8_t porciento = 0, const char* error = "") {
  Actualizacion a; a.estado = e; a.hayLugar = true; a.restanteMs = restanteMs; a.porciento = porciento;
  snprintf(a.error, sizeof(a.error), "%s", error); return a;
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
  for (int i = 0; i < 5; i++) tecla(TECLA_ABAJO);                    ui::refrescar(); guardar("12_menu_diagnostico");
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
  tecla(TECLA_ABAJO); wifiFalso = false;                             // quita los contornos

  // ---- conexion ----
  ui::tecla({ TECLA_MENU, PULSACION_LARGA }); tecla(TECLA_MENU);
  for (int i = 0; i < 3; i++) tecla(TECLA_ARRIBA);                   // Diagnostico -> Conexion
  redFalsa = InfoRed(); redFalsa.estado = RED_PORTAL; redFalsa.portalAbierto = true;
  tecla(TECLA_OK);                                                   ui::refrescar(); guardar("24_conexion_portal");
  redFalsa.fallo = true; redFalsa.hayGuardada = true; redFalsa.telefonos = 1;
                                                                     ui::refrescar(); guardar("25_conexion_portal_fallo_con_telefono");
  redFalsa = InfoRed(); redFalsa.estado = RED_CONECTANDO; redFalsa.progreso = 0.45f; snprintf(redFalsa.ssid, sizeof(redFalsa.ssid), "INFINITUM4F2A_2.4");
                                                                     ui::refrescar(); guardar("26_conexion_conectando");
  redFalsa = conectada("Casa Álvarez", "192.168.100.123", -52, 6, "WiFi 6");
                                                                     ui::refrescar(); guardar("27_conexion_conectada_c6");
  redFalsa = conectada("Red con un nombre larguísimo 5G", "10.0.0.7", -76, 149, "WiFi 6");
                                                                     ui::refrescar(); guardar("28_conexion_conectada_c5_5ghz");
  redFalsa = InfoRed(); redEncendida = false;                        ui::refrescar(); guardar("29_conexion_apagada");
  redEncendida = true; redFalsa = conectada("Casa Álvarez", "192.168.100.123", -52, 6, "WiFi 6");

  tecla(TECLA_OK);                                                   ui::refrescar(); guardar("30_conexion_opciones");
  tecla(TECLA_ABAJO); tecla(TECLA_OK);                               ui::refrescar(); guardar("31_conexion_olvidar_confirmar");
  otaFalsa = ota(OTA_ABIERTA, 272000);
  tecla(TECLA_ABAJO); tecla(TECLA_OK);                               ui::refrescar(); guardar("32_firmware_permitido");
  otaFalsa = ota(OTA_RECIBIENDO, 0, 47);                             ui::refrescar(); guardar("33_firmware_recibiendo");
  otaFalsa = ota(OTA_ERROR, 300000, 0, "Dañado o de otro chip");
                                                                     ui::refrescar(); guardar("34_firmware_error");
  otaFalsa = ota(OTA_LISTA, 0, 100);                                 ui::refrescar(); guardar("35_firmware_listo");
  otaFalsa = ota(OTA_CERRADA);                                       ui::refrescar(); guardar("36_firmware_tiempo_agotado");
  otaFalsa.hayLugar = false;                                         ui::refrescar(); guardar("37_firmware_sin_lugar");
  otaFalsa = ota(OTA_CERRADA);

  // ---- avisos de la red mientras se pesa ----
  ui::tecla({ TECLA_MENU, PULSACION_LARGA });
  eventoFalso = EVENTO_RED_CONECTO;                                  ui::refrescar(); guardar("38_pesar_aviso_wifi");

  // ---- contador de piezas (contador.cpp real) ----
  tecla(TECLA_MENU); tecla(TECLA_ARRIBA);                            // el menu recuerda Conexion -> Contar piezas
  pesoFalso = peso(0, true);
  tecla(TECLA_OK);                                                   ui::refrescar(); guardar("39_contar_paso1_tara");
  tecla(TECLA_OK); pesoFalso = peso(24.6f, true, 35.2f);             ui::refrescar(); guardar("40_contar_paso2_muestra");
  tecla(TECLA_OK);                                                   ui::refrescar(); guardar("41_contar_paso3_cuantas");
  tecla(TECLA_OK); pesoFalso = peso(123.0f, true, 35.2f);            ui::refrescar(); guardar("42_contando");
  relojFalsoMs += 5000; pesoFalso = peso(30.75f, true, 35.2f);       ui::refrescar(); guardar("43_contando_entre_dos");
  tecla(TECLA_ARRIBA);                                               ui::refrescar(); guardar("44_contar_opciones");
  tecla(TECLA_OK); pesoFalso = peso(123.0f, true, 35.2f);            ui::refrescar(); guardar("45_contar_pieza_guardada");
  relojFalsoMs += 5000;

  // muestra chica: 10 piezas que pesan 1.2 g en total
  tecla(TECLA_ARRIBA); tecla(TECLA_OK);                              // opciones -> Otra muestra
  tecla(TECLA_OK); pesoFalso = peso(1.2f, true); tecla(TECLA_OK); tecla(TECLA_OK);
                                                                     ui::refrescar(); guardar("46_contar_muestra_chica");
  relojFalsoMs += 5000;
  // pieza mas ligera que la division: 10 piezas en 0.5 g
  tecla(TECLA_ARRIBA); tecla(TECLA_ABAJO); tecla(TECLA_OK);          // opciones -> Otra muestra (ahora hay "Guardar pieza" antes)
  tecla(TECLA_OK); pesoFalso = peso(0.5f, true); tecla(TECLA_OK); tecla(TECLA_OK);
  relojFalsoMs += 5000;                                              ui::refrescar(); guardar("47_contar_no_confiable");

  tecla(TECLA_ARRIBA); tecla(TECLA_ABAJO); tecla(TECLA_ABAJO); tecla(TECLA_OK);   // opciones -> Elegir pieza
                                                                     ui::refrescar(); guardar("48_contar_elegir_pieza");
  pesoFalso = Peso(); pesoFalso.hayCelda = true;                     ui::refrescar(); guardar("49_contar_sin_calibrar");
  return 0;
}

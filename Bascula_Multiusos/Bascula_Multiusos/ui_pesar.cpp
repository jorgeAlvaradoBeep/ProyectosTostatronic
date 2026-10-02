/*
 * ============================================================
 *  VISTA: PESAR (pantalla principal)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *    OK corta ....... TARA (con el plato vacio, quita la tara)
 *    OK larga ....... CERO (quita tambien la tara)
 *    ARRIBA/ABAJO ... cambia la unidad: g, kg, oz
 *    MENU corta ..... menu
 *
 *  Tara y cero solo se toman con el peso ESTABLE. Si se piden
 *  mientras se mueve, quedan pendientes y se ejecutan en cuanto
 *  se asienta (o se avisa si no se asento a tiempo).
 *
 *  El anillo muestra que tanto de la capacidad de la celda se
 *  esta usando.
 * ============================================================
 */

#include "ui_comun.h"
#include "prueba_wifi.h"

namespace ui {
namespace vPesar {

namespace {

enum Pendiente : uint8_t { NADA, TARA, CERO };
Pendiente pendiente = NADA;
uint32_t  pendienteHastaMs = 0;

void ejecutar(Pendiente p) {
  if (p == TARA) {
    bascula::tarar();
    aviso(bascula::peso().conTara ? "Tara tomada" : "Tara quitada", COLOR_VERDE);
  } else {
    bascula::cero();
    aviso("Cero tomado", COLOR_VERDE);
  }
}

void pedir(Pendiente p) {
  if (!bascula::peso().calibrada) return;
  if (bascula::peso().estable) { ejecutar(p); return; }
  pendiente = p;
  pendienteHastaMs = millis() + ESPERA_ESTABLE_MS;
}

// Texto fijo de zAviso: la tara, o la capacidad y la division.
void textoAviso(const Peso& p, Unidad u, char* texto, size_t tam) {
  if (p.conTara) {
    char tara[16];
    formatearPeso(p.taraG, u, p.divisionG, tara, sizeof(tara));
    snprintf(texto, tam, "Tara %s %s", tara, nombreUnidad(u));
  } else if (p.divisionG < 1.0f) {
    snprintf(texto, tam, "Máx %u kg · d %.1f g", (unsigned)(p.capacidadG / 1000), p.divisionG);
  } else {
    snprintf(texto, tam, "Máx %u kg · d %u g", (unsigned)(p.capacidadG / 1000), (unsigned)p.divisionG);
  }
}

}  // namespace

void entrar() {
  limpiarVista();
  pendiente = NADA;
}

void tecla(const EventoTecla& e) {
  Ajustes& a = ajustes::actual();

  switch (e.tecla) {
    case TECLA_OK:
      if (!a.cal.valida) { irA(V_CALIBRAR); return; }
      if (e.tipo == PULSACION_CORTA) pedir(TARA);
      if (e.tipo == PULSACION_LARGA) pedir(CERO);
      break;

    case TECLA_ARRIBA:
    case TECLA_ABAJO:
      if (e.tipo != PULSACION_CORTA) return;
      a.unidad = (Unidad)((a.unidad + (e.tecla == TECLA_ARRIBA ? 1 : NUM_UNIDADES - 1)) % NUM_UNIDADES);
      ajustes::guardar();
      break;

    case TECLA_MENU:
      if (e.tipo == PULSACION_CORTA) irA(V_MENU);
      break;

    default: break;
  }
}

void refrescar() {
  Peso    p = bascula::peso();
  Unidad  u = ajustes::actual().unidad;
  char    texto[40];

  // ---- pendientes de tara / cero ----
  if (pendiente != NADA) {
    if (p.estable)                                          { ejecutar(pendiente); pendiente = NADA; }
    else if ((int32_t)(millis() - pendienteHastaMs) >= 0)   { aviso("No se asentó: intenta de nuevo", COLOR_NARANJA); pendiente = NADA; }
  }

  // ---- numero, unidad y anillo ----
  if (!p.hayCelda) {
    pintar(zTitulo, "PESO", COLOR_AZUL);
    pintarNumero(zNumero, "SIN CELDA", COLOR_NARANJA);
    pintar(zUnidad, bascula::leer().doutAlAire ? "revisa el cable DOUT" : "revisa el HX711", COLOR_TENUE);
    anillo(1.0f, COLOR_NARANJA);
  } else if (!p.calibrada) {
    pintar(zTitulo, "PESO", COLOR_AZUL);
    pintarNumero(zNumero, "SIN CALIBRAR", COLOR_AMBAR);
    pintar(zUnidad, "OK: calibrar", COLOR_TENUE);
    anillo(0, COLOR_AZUL);
  } else if (p.sobrecarga) {
    pintar(zTitulo, "PESO", COLOR_NARANJA);
    pintarNumero(zNumero, "SOBRECARGA", COLOR_NARANJA);
    snprintf(texto, sizeof(texto), "máximo %u kg", (unsigned)(p.capacidadG / 1000));
    pintar(zUnidad, texto, COLOR_TENUE);
    anillo(1.0f, COLOR_NARANJA);
  } else {
    pintar(zTitulo, p.conTara ? "NETO" : "PESO", p.conTara ? COLOR_AMBAR : COLOR_AZUL);
    formatearPeso(p.netoG, u, p.divisionG, texto, sizeof(texto));
    pintarNumero(zNumero, texto, COLOR_TEXTO);
    pintar(zUnidad, nombreUnidad(u), COLOR_TENUE);
    anillo(p.brutoG / p.capacidadG, COLOR_AZUL);
  }

  // ---- renglones de apoyo ----
  if (!avisoVigente()) {
    textoAviso(p, u, texto, sizeof(texto));
    pintar(zAviso, texto, COLOR_TENUE);
  }

  if (pendiente != NADA)             pintar(zEstado, "esperando a que se asiente…", COLOR_AMBAR);
  else if (!p.hayCelda || !p.calibrada) pintar(zEstado, "", COLOR_TENUE);
  else if (p.estable)                pintar(zEstado, "• ESTABLE", COLOR_VERDE);
  else                               pintar(zEstado, "midiendo…", COLOR_TENUE);

  if (pruebaWifi::activa())            pintar(zPie, "prueba de WiFi", COLOR_AZUL);
  else if (p.hayCelda && p.calibrada) pintar(zPie, "OK: tara", COLOR_TENUE);
  else                                pintar(zPie, "", COLOR_TENUE);
}

}  // namespace vPesar
}  // namespace ui

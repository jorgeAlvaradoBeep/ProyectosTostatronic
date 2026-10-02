/*
 * ============================================================
 *  VISTAS: MENU PRINCIPAL y AJUSTES
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Las dos son una lista: ARRIBA / ABAJO para moverse, OK para
 *  entrar (menu) o cambiar el valor (ajustes), MENU corta para
 *  regresar.
 *
 *  Contar piezas, Calorias y Conexion se agregan al menu en las
 *  fases 3 a 5.
 * ============================================================
 */

#include "ui_comun.h"

namespace ui {

// ------------------ MENU PRINCIPAL ------------------

namespace vMenu {

namespace {

struct Opcion { const char* nombre; Vista vista; };
const Opcion OPCIONES[] = {
  { "Pesar",       V_PESAR },
  { "Calibrar",    V_CALIBRAR },
  { "Ajustes",     V_AJUSTES },
  { "Diagnóstico", V_DIAGNOSTICO },
};
const uint8_t N = sizeof(OPCIONES) / sizeof(OPCIONES[0]);
const char*   NOMBRES[N];
uint8_t       seleccion = 0;

}  // namespace

void entrar() {
  limpiarVista();
  for (uint8_t i = 0; i < N; i++) NOMBRES[i] = OPCIONES[i].nombre;
  pintar(zTitulo, "MENÚ", COLOR_AZUL);
  pintar(zPie, "OK: entrar", COLOR_TENUE);
}

void tecla(const EventoTecla& e) {
  switch (e.tecla) {
    case TECLA_ARRIBA: seleccion = (seleccion + N - 1) % N; break;
    case TECLA_ABAJO:  seleccion = (seleccion + 1) % N;     break;
    case TECLA_OK:     if (e.tipo == PULSACION_CORTA) irA(OPCIONES[seleccion].vista); break;
    case TECLA_MENU:   if (e.tipo == PULSACION_CORTA) irA(V_PESAR); break;
    default: break;
  }
}

void refrescar() { lista(NOMBRES, N, seleccion); }

}  // namespace vMenu

// ------------------ AJUSTES ------------------

namespace vAjustes {

namespace {

enum Ajuste : uint8_t { A_UNIDAD, A_CELDA, A_BRILLO };
const uint8_t NIVELES_BRILLO[] = { 64, 128, 191, 255 };   // 25, 50, 75 y 100 %

char     textos[3][32];
const char* opciones[3];
uint8_t  n = 0;
uint8_t  seleccion = 0;
uint32_t avisoCeldaHastaMs = 0;

void armarTextos() {
  Ajustes& a = ajustes::actual();
  snprintf(textos[A_UNIDAD], sizeof(textos[0]), "Unidad: %s", nombreUnidad(a.unidad));
  snprintf(textos[A_CELDA],  sizeof(textos[0]), "Celda: %u kg", a.capacidadKg);
  snprintf(textos[A_BRILLO], sizeof(textos[0]), "Brillo: %u %%", (unsigned)((a.brillo * 100 + 127) / 255));
  // Sin BL en un GPIO (C6 Super Mini) no hay brillo que ajustar.
  n = pantalla::tieneBrillo() ? 3 : 2;
  for (uint8_t i = 0; i < n; i++) opciones[i] = textos[i];
}

void cambiar(uint8_t cual) {
  Ajustes& a = ajustes::actual();
  switch (cual) {
    case A_UNIDAD:
      a.unidad = (Unidad)((a.unidad + 1) % NUM_UNIDADES);
      break;

    case A_CELDA: {
      uint8_t i = 0;
      while (i < NUM_CELDAS && CELDAS[i].capacidadKg != a.capacidadKg) i++;
      a.capacidadKg = CELDAS[(i + 1) % NUM_CELDAS].capacidadKg;
      bascula::configurar(a.cal, a.capacidadKg);
      // Otra celda es otro factor: la calibracion anterior ya no aplica.
      avisoCeldaHastaMs = millis() + 3000;
      break;
    }

    case A_BRILLO: {
      uint8_t i = 0;
      while (i < 4 && NIVELES_BRILLO[i] < a.brillo) i++;
      // Si estaba justo en un nivel, pasa al siguiente; si estaba entre dos, al de arriba.
      if (i < 4 && NIVELES_BRILLO[i] == a.brillo) i++;
      a.brillo = NIVELES_BRILLO[i % 4];
      pantalla::brillo(a.brillo);
      break;
    }
  }
  ajustes::guardar();
  armarTextos();
}

}  // namespace

void entrar() {
  limpiarVista();
  seleccion = 0;
  avisoCeldaHastaMs = 0;
  armarTextos();
  pintar(zTitulo, "AJUSTES", COLOR_AZUL);
}

void tecla(const EventoTecla& e) {
  switch (e.tecla) {
    case TECLA_ARRIBA: seleccion = (seleccion + n - 1) % n; break;
    case TECLA_ABAJO:  seleccion = (seleccion + 1) % n;     break;
    case TECLA_OK:     if (e.tipo == PULSACION_CORTA) cambiar(seleccion); break;
    case TECLA_MENU:   if (e.tipo == PULSACION_CORTA) irA(V_MENU); break;
    default: break;
  }
}

void refrescar() {
  lista(opciones, n, seleccion);
  if (avisoCeldaHastaMs && (int32_t)(millis() - avisoCeldaHastaMs) < 0) pintar(zPie, "Recalibra la celda", COLOR_AMBAR);
  else                                                                   pintar(zPie, "OK: cambiar", COLOR_TENUE);
}

}  // namespace vAjustes

}  // namespace ui

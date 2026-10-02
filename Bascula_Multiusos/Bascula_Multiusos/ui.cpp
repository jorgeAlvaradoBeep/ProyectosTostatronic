/*
 * ============================================================
 *  INTERFAZ - arranque, piezas comunes y cambio de vista
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "ui.h"
#include "ui_comun.h"
#include "logo.h"

namespace ui {

// ------------------ zonas compartidas ------------------

Zona zTitulo = { 120,  50, 120, 26, F_TITULO };
Zona zAviso  = { 120,  78, 170, 16, F_CHICA  };
Zona zNumero = { 120, 114, 196, 52, F_NUMERO };
Zona zUnidad = { 120, 152, 150, 18, F_TEXTO  };
Zona zEstado = { 120, 174, 168, 16, F_CHICA  };
Zona zPie    = { 120, 194, 132, 14, F_CHICA  };
// Instrucciones a dos renglones (asistente de calibracion)
Zona zLinea1 = { 120, 104, 190, 22, F_TEXTO  };
Zona zLinea2 = { 120, 128, 190, 22, F_TEXTO  };
Zona zDato   = { 120, 152, 170, 16, F_CHICA  };

void pintar(Zona& z, const char* texto, uint16_t color, uint16_t fondo) {
  if (z.pintada && z.color == color && z.fondo == fondo && strncmp(z.texto, texto, sizeof(z.texto)) == 0) return;
  strlcpy(z.texto, texto, sizeof(z.texto));
  z.color   = color;
  z.fondo   = fondo;
  z.pintada = true;
  pantalla::zonaTexto(z.cx, z.cy, z.ancho, z.alto, texto, z.fuente, color, fondo);
}

void olvidar(Zona& z) { z.pintada = false; }

void pintarNumero(Zona& z, const char* texto, uint16_t color) {
  // La fuente del numero solo trae espacio, cifras, signo, punto y dos puntos.
  bool soloCifras = true;
  for (const char* p = texto; *p; p++) if ((uint8_t)*p < 0x20 || (uint8_t)*p > 0x3A) soloCifras = false;
  Fuente f = soloCifras && pantalla::anchoTexto(texto, F_NUMERO) <= z.ancho ? F_NUMERO : F_TITULO;
  if (f != z.fuente) { z.fuente = f; olvidar(z); }
  pintar(z, texto, color);
}

// ------------------ anillo perimetral ------------------

int16_t  anilloPintado = -1;     // grados ya dibujados; -1 = hay que dibujar todo
uint16_t anilloColor   = 0;
int8_t   segmentoLista = -1;     // opcion resaltada en el anillo de la lista

void anillo(float fraccion, uint16_t color) {
  int16_t grados = (int16_t)(constrain(fraccion, 0.0f, 1.0f) * 360.0f);
  if (grados == anilloPintado && color == anilloColor) return;

  if (anilloPintado < 0 || color != anilloColor) {
    pantalla::arco(ANILLO_EXT, ANILLO_INT, 0, grados, color);
    pantalla::arco(ANILLO_EXT, ANILLO_INT, grados, 360, COLOR_BORDE);
  } else if (grados > anilloPintado) {                 // crecio: solo el tramo nuevo
    pantalla::arco(ANILLO_EXT, ANILLO_INT, anilloPintado, grados, color);
  } else {                                             // bajo: se apaga el tramo sobrante
    pantalla::arco(ANILLO_EXT, ANILLO_INT, grados, anilloPintado, COLOR_BORDE);
  }
  anilloPintado = grados;
  anilloColor   = color;
}

// ------------------ avisos temporales ------------------

uint32_t avisoHastaMs = 0;

void aviso(const char* texto, uint16_t color, uint16_t ms) {
  pintar(zAviso, texto, color);
  avisoHastaMs = millis() + ms;
  if (avisoHastaMs == 0) avisoHastaMs = 1;
}

bool avisoVigente() {
  if (avisoHastaMs && (int32_t)(millis() - avisoHastaMs) >= 0) avisoHastaMs = 0;
  return avisoHastaMs != 0;
}

void limpiarVista() {
  pantalla::limpiar();
  for (Zona* z : { &zTitulo, &zAviso, &zNumero, &zUnidad, &zEstado, &zPie, &zLinea1, &zLinea2, &zDato }) olvidar(*z);
  zNumero.fuente = F_NUMERO;
  anilloPintado  = -1;
  segmentoLista  = -1;
  avisoHastaMs   = 0;
}

// ------------------ lista vertical ------------------

Zona zAnterior  = { 120,  84, 170, 22, F_TEXTO  };
Zona zActual    = { 120, 120, 190, 32, F_TITULO };
Zona zSiguiente = { 120, 156, 170, 22, F_TEXTO  };

void lista(const char* const* opciones, uint8_t n, uint8_t sel) {
  if (segmentoLista < 0) {
    olvidar(zAnterior); olvidar(zActual); olvidar(zSiguiente);
    pantalla::arco(ANILLO_EXT, ANILLO_INT, 0, 360, COLOR_BORDE);
  }
  // Circular: desde la primera, ARRIBA lleva a la ultima.
  pintar(zAnterior,  n > 2 ? opciones[(sel + n - 1) % n] : "", COLOR_TENUE);
  pintar(zActual,    opciones[sel], COLOR_TEXTO, COLOR_TARJETA);
  pintar(zSiguiente, n > 1 ? opciones[(sel + 1) % n] : "", COLOR_TENUE);

  if (segmentoLista != sel) {
    const float hueco = 4;   // grados entre segmentos
    float tramo = 360.0f / n;
    if (segmentoLista >= 0) pantalla::arco(ANILLO_EXT, ANILLO_INT, segmentoLista * tramo + hueco / 2, (segmentoLista + 1) * tramo - hueco / 2, COLOR_BORDE);
    pantalla::arco(ANILLO_EXT, ANILLO_INT, sel * tramo + hueco / 2, (sel + 1) * tramo - hueco / 2, COLOR_AZUL);
    segmentoLista = sel;
  }
}

// ------------------ unidades ------------------

const float GRAMOS_POR_ONZA = 28.349523125f;

const char* nombreUnidad(Unidad u) {
  switch (u) {
    case UNIDAD_KG: return "kg";
    case UNIDAD_OZ: return "oz";
    default:        return "g";
  }
}

void formatearPeso(float gramos, Unidad u, float divisionG, char* texto, size_t tam) {
  // Primero se redondea a la division: la bascula no promete mas que eso.
  float redondo = roundf(gramos / divisionG) * divisionG;
  int   decG    = divisionG < 1.0f ? 1 : 0;

  float valor;
  int   dec;
  switch (u) {
    case UNIDAD_KG: valor = redondo / 1000.0f;       dec = decG + 3;                  break;
    case UNIDAD_OZ: valor = redondo / GRAMOS_POR_ONZA; dec = divisionG < 0.3f ? 3 : 2; break;
    default:        valor = redondo;                 dec = decG;                      break;
  }
  if (fabsf(valor) < 0.5f * powf(10, -dec)) valor = 0;   // sin "-0.0"
  snprintf(texto, tam, "%.*f", dec, valor);
}

// ------------------ cambio de vista ------------------

Vista vista = V_PESAR;

void irA(Vista v) {
  vista = v;
  switch (v) {
    case V_PESAR:       vPesar::entrar();          break;
    case V_MENU:        vMenu::entrar();           break;
    case V_AJUSTES:     vAjustes::entrar();        break;
    case V_DIAGNOSTICO: vDiagnostico::entrar();    break;
    case V_CALIBRAR:    vCalibrar::entrar(false);  break;
  }
}

// ------------------ publico ------------------

void arranque(uint16_t duracionMs) {
  pantalla::limpiar();
  pantalla::arco(ANILLO_EXT, ANILLO_INT, 0, 360, COLOR_AZUL);

  pantalla::imagen(CENTRO_X - LOGO_ANCHO / 2, 92 - LOGO_ALTO / 2, LOGO_ANCHO, LOGO_ALTO, LOGO_PIXELES);
  pantalla::zonaTexto(120, 148, 184, 20, PROYECTO_NOMBRE, F_TEXTO, COLOR_TEXTO);
  pantalla::zonaTexto(120, 168, 180, 16, "Desarrollado por " PROYECTO_AUTOR, F_CHICA, COLOR_AZUL);
  pantalla::zonaTexto(120, 186, 150, 16, PROYECTO_ING, F_CHICA, COLOR_TENUE);
  pantalla::zonaTexto(120, 204, 104, 14, PROYECTO_WEB, F_CHICA, COLOR_TENUE);   // aqui abajo el circulo ya es angosto

  delay(duracionMs);
}

void iniciar() {
  if (ajustes::actual().cal.valida) {
    irA(V_PESAR);
  } else {
    vista = V_CALIBRAR;
    vCalibrar::entrar(true);
  }
}

void tecla(const EventoTecla& e) {
  // MENU larga: de cualquier lado a la pantalla de pesar.
  if (e.tecla == TECLA_MENU && e.tipo == PULSACION_LARGA) { irA(V_PESAR); return; }

  switch (vista) {
    case V_PESAR:       vPesar::tecla(e);       break;
    case V_MENU:        vMenu::tecla(e);        break;
    case V_AJUSTES:     vAjustes::tecla(e);     break;
    case V_DIAGNOSTICO: vDiagnostico::tecla(e); break;
    case V_CALIBRAR:    vCalibrar::tecla(e);    break;
  }
}

void refrescar() {
  switch (vista) {
    case V_PESAR:       vPesar::refrescar();       break;
    case V_MENU:        vMenu::refrescar();        break;
    case V_AJUSTES:     vAjustes::refrescar();     break;
    case V_DIAGNOSTICO: vDiagnostico::refrescar(); break;
    case V_CALIBRAR:    vCalibrar::refrescar();    break;
  }
}

}  // namespace ui

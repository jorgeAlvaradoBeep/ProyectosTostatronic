/*
 * ============================================================
 *  INTERFAZ - piezas compartidas entre las vistas
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Uso interno de ui*.cpp. El resto del programa solo ve ui.h.
 *
 *  Cada vista (pesar, menu, conexion...) tiene tres funciones:
 *    entrar()     pinta la pantalla desde cero
 *    tecla(e)     reacciona a una tecla
 *    refrescar()  se llama cada 100 ms; solo repinta lo que cambio
 *
 *  Reparto de la pantalla redonda (centros y tamanos verificados
 *  contra el circulo util de radio 106 con el simulador):
 *
 *      y  50  titulo / modo              zTitulo
 *      y  78  aviso o dato secundario    zAviso
 *      y 114  NUMERO GRANDE              zNumero
 *      y 152  unidad                     zUnidad
 *      y 174  estado (estabilidad)       zEstado
 *      y 194  ayuda de teclas            zPie
 *      anillo perimetral: capacidad, estabilidad o posicion en menu
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "pantalla.h"
#include "teclado.h"
#include "bascula.h"
#include "ajustes.h"

namespace ui {

enum Vista : uint8_t { V_PESAR, V_MENU, V_CONTAR, V_CONEXION, V_CALIBRAR, V_AJUSTES, V_DIAGNOSTICO };
void irA(Vista v);

// ------------------ zonas de texto ------------------

/* Una zona de texto que recuerda lo ultimo que pinto: solo se vuelve a
 * mandar a la pantalla cuando el texto o el color cambian. Asi el SPI
 * queda libre casi todo el tiempo y nada parpadea. */
struct Zona {
  int16_t  cx, cy, ancho, alto;
  Fuente   fuente;
  char     texto[40];
  uint16_t color, fondo;
  bool     pintada;
};

void pintar(Zona& z, const char* texto, uint16_t color = COLOR_TEXTO, uint16_t fondo = COLOR_FONDO);
void olvidar(Zona& z);

// Para cantidades: fuente grande si el texto son solo cifras y cabe; si
// no (letras, o demasiado ancho), la de titulo.
void pintarNumero(Zona& z, const char* texto, uint16_t color = COLOR_TEXTO);

// Para textos que no controlamos (el nombre de una red, una IP): fuente
// 'grande' si cabe en la zona; si no, la 'chica'; y si ni asi, recortado.
void pintarAjustado(Zona& z, const char* texto, Fuente grande, Fuente chica, uint16_t color = COLOR_TEXTO);

// Copia 'texto' a 'salida' quitandole caracteres del final (y poniendo "…")
// hasta que mida 'ancho' pixeles o menos. Respeta los acentos (UTF-8).
void recortar(const char* texto, Fuente fuente, int16_t ancho, char* salida, size_t tam);

extern Zona zTitulo, zAviso, zNumero, zUnidad, zEstado, zPie;
extern Zona zLinea1, zLinea2, zDato;

// Borra la pantalla y olvida zonas, anillo y avisos: al entrar a una vista.
void limpiarVista();

// Anillo perimetral lleno en 'fraccion' (0..1). Repinta solo el tramo que cambia.
void anillo(float fraccion, uint16_t color);

// Mensaje temporal en zAviso. Mientras este vigente, las vistas no
// pintan ahi su texto fijo.
void aviso(const char* texto, uint16_t color = COLOR_AZUL, uint16_t ms = 1800);
bool avisoVigente();

// ------------------ lista vertical (menu y ajustes) ------------------
// Anterior y siguiente en chico, la actual en grande sobre una tarjeta,
// y el anillo dividido en un segmento por opcion.
void lista(const char* const* opciones, uint8_t n, uint8_t seleccion);

// ------------------ unidades ------------------

const char* nombreUnidad(Unidad u);

// Redondea a la division de la celda y escribe con los decimales que
// corresponden a la unidad. Nunca escribe "-0.0".
void formatearPeso(float gramos, Unidad u, float divisionG, char* texto, size_t tam);

// ------------------ vistas ------------------

namespace vPesar       { void entrar(); void tecla(const EventoTecla& e); void refrescar(); }
namespace vMenu        { void entrar(); void tecla(const EventoTecla& e); void refrescar(); }
namespace vContar      { void entrar(); void tecla(const EventoTecla& e); void refrescar(); }
namespace vAjustes     { void entrar(); void tecla(const EventoTecla& e); void refrescar(); }
namespace vDiagnostico { void entrar(); void tecla(const EventoTecla& e); void refrescar(); }
namespace vCalibrar    { void entrar(bool primerArranque); void tecla(const EventoTecla& e); void refrescar(); }
// salir(): la unica vista que deja algo abierto (el permiso para recibir firmware).
namespace vConexion    { void entrar(); void tecla(const EventoTecla& e); void refrescar(); void salir(); }

}  // namespace ui

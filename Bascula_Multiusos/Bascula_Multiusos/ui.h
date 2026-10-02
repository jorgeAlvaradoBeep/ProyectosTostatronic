/*
 * ============================================================
 *  INTERFAZ EN LA PANTALLA REDONDA
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  La GC9A01 es circular: las esquinas del cuadro de 240x240 no
 *  existen. Por eso cada texto se pinta en una "zona" con centro
 *  y tamano, y pantalla::zonaTexto() verifica que la zona quepa
 *  en el circulo util (y lo reporta por Serial si no).
 *
 *  Vistas (una por archivo):
 *    ui_pesar.cpp        pantalla principal: peso, tara, cero
 *    ui_menu.cpp         menu principal y ajustes
 *    ui_calibrar.cpp     asistente de calibracion
 *    ui_diagnostico.cpp  prueba de hardware y prueba de WiFi
 *
 *  Teclas en todas las vistas:
 *    MENU corta ... menu / atras        MENU larga ... volver a pesar
 *    ARRIBA/ABAJO . moverse o cambiar   OK ........... confirmar
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "teclado.h"

namespace ui {

// Logo + atribucion. Bloquea 'duracionMs' (solo se usa al arrancar).
void arranque(uint16_t duracionMs);

// Primera vista: el asistente de calibracion si nunca se ha calibrado;
// si no, la de pesar.
void iniciar();

void tecla(const EventoTecla& evento);
void refrescar();

}  // namespace ui

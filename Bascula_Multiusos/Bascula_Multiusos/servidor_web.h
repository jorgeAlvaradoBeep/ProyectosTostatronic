/*
 * ============================================================
 *  SERVIDOR WEB - portal, bascula en vivo y actualizacion de firmware
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Un solo servidor en el puerto 80, para las dos caras del WiFi:
 *
 *    Por la red del portal (192.168.4.1)  -> elegir el WiFi
 *    Por tu red (IP o tostabascula.local) -> la bascula: peso en
 *                                            vivo, piezas y ajustes
 *    Por cualquiera de las dos            -> /bascula y /actualizar
 *
 *  El peso en vivo va por Server-Sent Events (/eventos): el
 *  navegador deja una conexion abierta y la bascula escribe en ella
 *  cuando algo cambia. Tara, cero, unidad y el contador de piezas se
 *  mandan como ordenes sueltas (POST) y cualquiera en la red puede
 *  usarlas; recalibrar y cambiar el firmware piden antes un permiso
 *  desde las teclas de la bascula.
 *
 *  Se atiende desde loop() (lo llama red::atender), en la misma
 *  tarea que la pantalla: asi nunca hay dos tareas dibujando ni
 *  tocando el mismo estado. La celda no se entera: se lee en su
 *  propia tarea, de mayor prioridad (ver bascula.h).
 *
 *  ------------------------------------------------------------
 *  ACTUALIZACION DE FIRMWARE POR WiFi
 *  ------------------------------------------------------------
 *  La pagina /actualizar recibe el .bin que exporta el Arduino IDE
 *  (Programa > Exportar binario compilado) y lo escribe en la otra
 *  mitad de la Flash. Tres seguros:
 *
 *    1. Solo acepta el archivo mientras la bascula lo permite
 *       (MENU > Conexion > Actualizar, 5 minutos). Hay que estar
 *       frente a ella: nadie en la red puede cambiarle el programa.
 *    2. El chip verifica el firmware antes de activarlo: uno
 *       danado, o compilado para otro chip, se rechaza y la
 *       bascula sigue con el que tenia.
 *    3. Si el firmware nuevo no logra arrancar, la placa regresa
 *       sola al anterior (ver OTA_CONFIRMAR_MS en config.h).
 *
 *  Lo que NO se puede verificar: que el .bin sea de la misma PLACA
 *  de config.h. Un firmware de DevKit en una mini arranca, pero con
 *  los pines cambiados; se corrige cargando el correcto (por WiFi
 *  si la pantalla aun se ve, o por USB).
 *
 *  Requiere el Partition Scheme "Minimal SPIFFS (1.9MB APP with
 *  OTA)". Con "Huge APP" no hay donde poner el firmware nuevo.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

enum EstadoOta : uint8_t {
  OTA_CERRADA,      // la bascula no acepta firmware
  OTA_ABIERTA,      // permitida desde el menu, esperando el archivo
  OTA_RECIBIENDO,
  OTA_LISTA,        // recibido y verificado: se reinicia en un momento
  OTA_ERROR,        // fallo; se puede intentar de nuevo
};

struct Actualizacion {
  EstadoOta estado     = OTA_CERRADA;
  bool      hayLugar   = false;   // la Flash tiene una segunda particion de programa
  uint32_t  restanteMs = 0;       // lo que queda del permiso
  uint8_t   porciento  = 0;
  char      error[40]  = "";
};

namespace web {

void arrancar();     // al encender el WiFi
void detener();
void atender();

// Abre (o cierra) el permiso para recibir firmware. No hace nada si la
// Flash no tiene lugar o si ya hay una actualizacion en curso.
void permitirActualizacion(bool si);

Actualizacion actualizacion();

// Abre por CAL_WEB_VENTANA_MS el permiso para recalibrar desde la pagina.
// Se cierra solo al guardar una calibracion o al acabarse el tiempo.
void permitirCalibracion(bool si);

// Un firmware llegado por WiFi arranca "a prueba": si la placa se reinicia
// antes de que se llame a esto, regresa sola al firmware anterior. Se llama
// desde loop() a los OTA_CONFIRMAR_MS. Sin efecto en un arranque normal.
void confirmarFirmware();

// Mientras llega el firmware, loop() esta detenido dentro del servidor.
// Esta funcion se llama a cada cambio de porcentaje para que la pantalla
// se siga actualizando.
void alAvanzar(void (*aviso)());

}  // namespace web

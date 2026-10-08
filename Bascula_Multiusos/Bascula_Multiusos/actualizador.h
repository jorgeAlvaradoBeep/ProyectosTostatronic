/*
 * ============================================================
 *  ACTUALIZADOR - firmware nuevo desde el servidor de Tostatronic
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Complementa a la actualizacion por pagina web (servidor_web.h),
 *  donde uno sube el .bin a mano. Aqui es la bascula la que va a
 *  buscarlo:
 *
 *    1. Al arrancar, en cuanto se conecta al WiFi, pide al servidor
 *       (OTA_SERVIDOR_URL en config.h) la lista de versiones y mira
 *       la entrada de SU placa (ID_PLACA).
 *    2. Si la version publicada es mas nueva que la que corre, lo
 *       OFRECE en la pantalla: "Nueva version v0.5 - OK: instalar".
 *       Nunca instala solo; si se dice "ahora no", no insiste hasta
 *       el siguiente arranque (queda en MENU > Conexion > Instalar).
 *    3. Con OK descarga el .bin directo a la otra particion de la
 *       Flash, calculando el SHA-256 al vuelo. Si no coincide con el
 *       anunciado, o el chip no da por buena la imagen, se descarta
 *       y la bascula sigue con lo que tenia.
 *    4. Reinicia con el firmware nuevo "a prueba": si no llega a
 *       correr OTA_CONFIRMAR_MS, la placa regresa sola al anterior
 *       (el mismo mecanismo que la actualizacion por web).
 *
 *  La consulta y la descarga corren en una tarea aparte, con la
 *  misma prioridad que loop(): la pantalla sigue viva, las teclas
 *  responden y la celda (en su propia tarea, de mayor prioridad) no
 *  se entera. Esa tarea solo escribe su propio estado; la interfaz
 *  lo lee con info(). Nada de aqui detiene la bascula si el servidor
 *  no contesta: un arranque sin internet es un arranque normal.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

enum EstadoActualizador : uint8_t {
  ACT_INACTIVA,      // todavia no se ha consultado (sin WiFi, o aun no conecta)
  ACT_BUSCANDO,      // pidiendo la lista de versiones al servidor
  ACT_AL_DIA,        // el servidor no tiene nada mas nuevo para esta placa
  ACT_DISPONIBLE,    // hay version nueva: esperando que el usuario diga OK
  ACT_DESCARGANDO,
  ACT_LISTA,         // escrita y verificada: se reinicia en un momento
  ACT_ERROR,         // fallo al consultar o al descargar; se puede reintentar
};

struct InfoActualizador {
  EstadoActualizador estado     = ACT_INACTIVA;
  bool     hayLugar   = false;   // la Flash tiene una segunda particion de programa
  bool     descartada = false;   // el usuario dijo "ahora no" en este arranque
  char     version[16] = "";     // la version publicada en el servidor
  char     notas[48]   = "";     // que trae de nuevo (una linea)
  uint32_t tamano      = 0;      // bytes del .bin
  uint8_t  porciento   = 0;      // avance de la descarga
  char     error[40]   = "";
};

namespace actualizador {

void iniciar();
void atender();        // desde loop(): lanza la consulta al conectar y reinicia al terminar

void buscar();         // consultar el servidor ahora (desde el menu Conexion)
void instalar();       // descargar e instalar la version disponible
void descartar();      // "ahora no": no se vuelve a ofrecer en este arranque

bool ocupado();        // consultando o descargando: la pagina web no debe recibir firmware
InfoActualizador info();

}  // namespace actualizador

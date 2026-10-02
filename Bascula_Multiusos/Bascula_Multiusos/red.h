/*
 * ============================================================
 *  RED - WiFi con portal cautivo, sin claves en el codigo
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  La bascula funciona completa sin WiFi. El WiFi es un extra, y
 *  por eso aqui NADA bloquea ni reinicia la placa: conectarse,
 *  fallar, abrir el portal o cambiar de red pasa "por debajo"
 *  mientras se sigue pesando, sin perder la tara ni el cero.
 *
 *  Como arranca:
 *
 *    1. NO hay red guardada -> abre la red "Tostatronic-Bascula".
 *       Te conectas con el telefono y se abre solo un portal que
 *       lista las redes; eliges la tuya y escribes la clave.
 *
 *    2. YA hay red guardada -> se enlaza a ella. Si no la
 *       encuentra (se fue la luz, cambio la clave), abre el
 *       portal y la reintenta cada 3 minutos mientras nadie lo
 *       este usando.
 *
 *  Conectada, la bascula se encuentra por su IP (sale en la
 *  pantalla de Conexion) o en http://tostabascula.local
 *
 *  Todo se atiende por sondeo desde loop() con red::atender():
 *  sin tareas propias y sin callbacks del WiFi, asi el estado solo
 *  se toca desde un lugar.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

enum EstadoRed : uint8_t {
  RED_APAGADA,      // WiFi apagado desde el menu
  RED_PORTAL,       // red propia abierta, esperando a que elijan un WiFi
  RED_CONECTANDO,
  RED_CONECTADA,
};

enum EventoRed : uint8_t {
  EVENTO_RED_NINGUNO,
  EVENTO_RED_CONECTO,        // se enlazo a la red guardada
  EVENTO_RED_CONECTO_NUEVA,  // se enlazo a una red recien elegida en el portal
  EVENTO_RED_SE_PERDIO,
};

// Solo tipos simples (nada de la libreria WiFi): la interfaz y el
// simulador usan esta "foto" sin saber como esta hecha la red.
struct InfoRed {
  EstadoRed   estado        = RED_APAGADA;
  bool        portalAbierto = false;   // la red propia esta arriba
  bool        hayGuardada   = false;
  bool        fallo         = false;   // el ultimo intento no logro conectar
  uint8_t     telefonos     = 0;       // conectados a la red del portal
  float       progreso      = 0;       // 0..1 de la espera, mientras conecta
  char        ssid[33]      = "";      // red a la que se conecta o esta conectada
  char        ip[16]        = "";
  int8_t      rssi          = 0;       // dBm
  uint8_t     canal         = 0;
  bool        banda5        = false;   // 5 GHz (solo el ESP32-C5)
  const char* estandar      = "";      // "WiFi 6", "WiFi 4"...
  const char* norma         = "";      // "802.11ax", "802.11n"...
};

namespace red {

void iniciar();              // lee la NVS y, si el WiFi esta encendido, arranca
void atender();              // llamar seguido desde loop()

InfoRed   info();
EventoRed evento();          // lo ultimo que paso; se entrega una sola vez

bool encendida();
void encender(bool si);      // se guarda: sobrevive a un apagado
void olvidar();              // borra la red guardada y abre el portal

// ---- para servidor_web.cpp ----

// Prueba una red elegida en el portal. Si conecta, se guarda.
// Devuelve false si los datos no son validos o el portal no esta abierto.
bool probar(const char* ssid, const char* clave);

void actividad();            // alguien esta usando el portal: no reintentar aun
void ipVista();              // el telefono ya leyo la IP: el portal puede cerrarse
void olvidarEnBreve();       // olvidar(), pero dejando salir antes la respuesta HTTP

}  // namespace red

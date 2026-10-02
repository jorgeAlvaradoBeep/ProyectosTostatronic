/*
 * ============================================================
 *  BASCULA MULTIUSOS - ESP32-C6 / ESP32-C5 + HX711 + GC9A01
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 *  www.tostatronic.com
 * ============================================================
 *
 *  Bascula con pantalla redonda, contador de piezas por peso,
 *  calorias por alimento y pagina web con el peso en vivo.
 *  Aprovecha el WiFi 6 del ESP32-C6 y el WiFi 6 de doble banda
 *  (2.4 y 5 GHz) del ESP32-C5.
 *
 *  >>> ESTADO: FASE 4 de 6 - PAGINA WEB Y CONTADOR DE PIEZAS <<<
 *  Ya pesa en gramos, kilos u onzas (filtro de mediana, peso
 *  estable, tara, cero y calibracion guardada en la NVS), CUENTA
 *  PIEZAS por peso (con perfiles guardados y avisos cuando el
 *  conteo no es confiable) y se conecta al WiFi SIN claves en el
 *  codigo: abre la red "Tostatronic-Bascula" con un portal para
 *  elegir tu red desde el telefono. Ya conectada sirve una pagina
 *  con el peso en vivo, el contador de piezas y los ajustes, y el
 *  firmware se actualiza desde el navegador, sin cable.
 *
 *  El WiFi es un extra: la bascula pesa completa sin el, y nada de
 *  la red detiene ni reinicia la placa.
 *
 *  ------------------------------------------------------------
 *  HARDWARE
 *  ------------------------------------------------------------
 *    Microcontrolador  ESP32-C6 o ESP32-C5 (DevKit o mini)
 *    Celda de carga    tipo barra, 1 kg (tambien 5, 10 y 20 kg)
 *    Amplificador      HX711 a 10 muestras por segundo (o 80)
 *    Pantalla          GC9A01 redonda 1.28", 240x240, SPI
 *    Teclado           membrana 1x4 (MENU, ARRIBA, ABAJO, OK)
 *
 *  Los pines estan en config.h, con una seccion por placa. Ahi
 *  mismo viene que placa elegir en el Arduino IDE.
 *  Partition Scheme: Minimal SPIFFS (1.9MB APP with OTA).
 *
 *  ------------------------------------------------------------
 *  LIBRERIAS (Gestor de librerias del Arduino IDE)
 *  ------------------------------------------------------------
 *    GFX Library for Arduino (moononournation)  >= 1.6.5
 *    ArduinoJson (Benoit Blanchon)              >= 7   [desde la fase 5]
 *    Core: esp32 de Espressif >= 3.3.0 (el C5 no existe antes)
 *
 *    NO uses TFT_eSPI: no compila para el ESP32-C6 con el core 3.x.
 *
 *  ------------------------------------------------------------
 *  TECLAS
 *  ------------------------------------------------------------
 *    En todas partes: MENU corta = menu / atras,
 *                     MENU larga = volver a pesar.
 *    Pesando:  OK corta = tara, OK larga = cero,
 *              ARRIBA / ABAJO = unidad (g, kg, oz).
 *    Menu principal: Pesar, Contar piezas, Conexion, Calibrar,
 *                    Ajustes, Diagnostico.
 *
 *  La primera vez que arranca (sin calibracion guardada) entra
 *  directo al asistente de calibracion.
 *
 *  ------------------------------------------------------------
 *  ARCHIVOS
 *  ------------------------------------------------------------
 *    config.h            pines por placa y constantes
 *    pantalla.h/.cpp     capa propia sobre la libreria grafica
 *    hx711.h/.cpp        lectura del HX711 en seccion critica
 *    filtro.h            mediana + descarte de atipicos
 *    bascula.h/.cpp      tarea de muestreo, estabilidad, cero y tara
 *    ajustes.h/.cpp      calibracion y preferencias en NVS
 *    contador.h/.cpp     contador de piezas y sus perfiles
 *    teclado.h/.cpp      antirrebote y pulsacion corta / larga
 *    red.h/.cpp          WiFi: portal cautivo, red guardada, mDNS
 *    servidor_web.h/.cpp paginas, peso en vivo, ordenes y firmware
 *    paginas.h           el HTML, embebido (sin nada de internet)
 *    prueba_wifi.h/.cpp  radio transmitiendo, para el Diagnostico
 *    ui*.h/.cpp          lo que se ve en la pantalla redonda
 *    fuentes.h           GENERADO (docs/herramientas/generar_recursos.py)
 *    logo.h              GENERADO (idem)
 * ============================================================
 */

#include "config.h"
#include "pantalla.h"
#include "teclado.h"
#include "bascula.h"
#include "ajustes.h"
#include "contador.h"
#include "prueba_wifi.h"
#include "red.h"
#include "servidor_web.h"
#include "ui.h"

const uint16_t REFRESCO_PANTALLA_MS = 100;   // 10 cuadros por segundo sobran para leer un numero
const uint16_t REPORTE_SERIAL_MS    = 1000;

uint32_t ultimoRefresco = 0;
uint32_t ultimoReporte  = 0;
bool     firmwareConfirmado = false;

// Una linea por segundo con todo lo necesario para depurar sin pantalla.
// Se arma completa y se manda solo si cabe en el bufer de salida: si nadie
// esta leyendo el puerto, la linea se tira en vez de frenar la bascula.
void reporteSerial() {
  Lectura l = bascula::leer();
  Peso    p = bascula::peso();
  const char* estados[] = { "ok", "SIN RESPUESTA", "SATURADO", "TRAMA LENTA" };
  const char* redes[]   = { "apagada", "portal", "conectando", "conectada" };
  InfoRed r = red::info();

  char linea[320];
  int  n = snprintf(linea, sizeof(linea), "crudo=%ld  filtrado=%ld  ", (long)l.crudo, (long)l.filtrado);
  if (p.calibrada) n += snprintf(linea + n, sizeof(linea) - n, "neto=%.2f g  tara=%.2f g  ", p.netoG, p.taraG);
  else             n += snprintf(linea + n, sizeof(linea) - n, "SIN CALIBRAR  ");
  n += snprintf(linea + n, sizeof(linea) - n,
                "estable=%d  sps=%.1f  ruido_pp=%ld  atipicas=%lu  estado=%s  lentas=%lu  trama_max=%lu us",
                l.estable, l.muestrasPorSegundo, (long)l.ruidoPicoPico, (unsigned long)l.atipicos, estados[l.estado],
                (unsigned long)l.tramasLentas, (unsigned long)l.tramaMaxMicros);
  n += snprintf(linea + n, sizeof(linea) - n, "  red=%s", redes[r.estado]);
  if (r.estado == RED_CONECTADA) n += snprintf(linea + n, sizeof(linea) - n, " %d dBm", r.rssi);
  if (pruebaWifi::activa())      n += snprintf(linea + n, sizeof(linea) - n, "  prueba=%lu paq/s", (unsigned long)pruebaWifi::paquetesPorSegundo());
  n += snprintf(linea + n, sizeof(linea) - n, "\n");

  if (n > 0 && n < (int)sizeof(linea) && Serial.availableForWrite() >= n) Serial.write((const uint8_t*)linea, n);
}

void setup() {
  Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
  // En las mini, Serial es el USB nativo. Con el cable puesto pero sin
  // monitor abierto, cada print esperaria hasta 2 s a que alguien lea:
  // la bascula se congelaria y perderia teclas. Con 0 ms, lo que no cabe
  // se descarta al instante.
  Serial.setTxTimeoutMs(0);
#endif
  delay(300);   // en las mini el USB es nativo: tiempo para que el monitor reconecte

  Serial.println();
  Serial.println("=== " PROYECTO_NOMBRE " v" PROYECTO_VERSION " ===");
  Serial.println("Desarrollado por " PROYECTO_AUTOR " - " PROYECTO_ING " - " PROYECTO_WEB);
  Serial.printf("Placa: %s  |  HX711 a %u SPS\n", NOMBRE_PLACA, HX_MUESTRAS_POR_SEGUNDO);

  if (!pantalla::iniciar()) {
    Serial.println("ERROR: la pantalla no inicio. Revisa el cableado SPI y config.h.");
  }

  ajustes::cargar();
  contador::cargar();
  Ajustes& a = ajustes::actual();
  pantalla::brillo(a.brillo);
  if (a.cal.valida) Serial.printf("Calibracion: factor=%.3f cuentas/g  offset=%ld  celda=%u kg\n", a.cal.factor, (long)a.cal.offset, a.capacidadKg);
  else              Serial.println("Sin calibracion guardada: se abre el asistente.");

  teclado::iniciar();
  bascula::configurar(a.cal, a.capacidadKg);
  bascula::iniciar();     // desde aqui la celda se lee sola, en su propia tarea
  red::iniciar();         // el WiFi se conecta por debajo, mientras sale el logo

  ui::arranque(2500);
  ui::iniciar();
}

void loop() {
  EventoTecla evento;
  while (teclado::leer(evento)) ui::tecla(evento);

  // Portal, servidor web y estado del WiFi. Regresa de inmediato, salvo
  // mientras llega un firmware nuevo (la pantalla muestra el avance).
  red::atender();

  uint32_t ahora = millis();

  // Ya lleva un rato corriendo bien: si este firmware llego por WiFi, se
  // queda como el definitivo (ver servidor_web.h).
  if (!firmwareConfirmado && ahora >= OTA_CONFIRMAR_MS) {
    firmwareConfirmado = true;
    web::confirmarFirmware();
  }

  if (ahora - ultimoRefresco >= REFRESCO_PANTALLA_MS) {
    ultimoRefresco = ahora;
    ui::refrescar();
  }

  if (ahora - ultimoReporte >= REPORTE_SERIAL_MS) {
    ultimoReporte = ahora;
    reporteSerial();
  }

  delay(2);   // cede el CPU: en un solo nucleo, loop() no debe girar en vacio
}

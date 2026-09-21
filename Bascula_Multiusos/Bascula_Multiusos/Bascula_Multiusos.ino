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
 *  >>> ESTADO: FASE 1 de 6 - ARRANQUE DEL HARDWARE <<<
 *  Esta version comprueba que la pantalla, el HX711 y el teclado
 *  estan bien cableados. Muestra el peso CRUDO (cuentas del ADC,
 *  todavia sin calibrar), las muestras por segundo y el ruido.
 *
 *  ------------------------------------------------------------
 *  HARDWARE
 *  ------------------------------------------------------------
 *    Microcontrolador  ESP32-C6 o ESP32-C5 (DevKit o mini)
 *    Celda de carga    tipo barra, 1 kg (tambien 5, 10 y 20 kg)
 *    Amplificador      HX711 a 80 muestras por segundo
 *    Pantalla          GC9A01 redonda 1.28", 240x240, SPI
 *    Teclado           membrana 1x4 (MENU, ARRIBA, ABAJO, OK)
 *
 *  Los pines estan en config.h, con una seccion por placa. Ahi
 *  mismo viene que placa elegir en el Arduino IDE.
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
 *  TECLAS EN ESTA PRUEBA
 *  ------------------------------------------------------------
 *    OK corta ........ toma el cero (el numero pasa a ser relativo)
 *    OK larga ........ quita el cero
 *    ARRIBA / ABAJO .. brillo (si BL va a un GPIO)
 *    MENU larga ...... muestra el contorno de las zonas de texto y
 *                      el circulo util, para revisar el diseno
 *    Cada tecla enciende su punto en pantalla mientras se presiona.
 *
 *  ------------------------------------------------------------
 *  ARCHIVOS
 *  ------------------------------------------------------------
 *    config.h        pines por placa y constantes
 *    pantalla.h/.cpp capa propia sobre la libreria grafica
 *    hx711.h/.cpp    lectura del HX711 en seccion critica
 *    bascula.h/.cpp  tarea de muestreo
 *    teclado.h/.cpp  antirrebote y pulsacion corta / larga
 *    ui.h/.cpp       lo que se ve en la pantalla redonda
 *    fuentes.h       GENERADO (docs/herramientas/generar_recursos.py)
 *    logo.h          GENERADO (idem)
 * ============================================================
 */

#include "config.h"
#include "pantalla.h"
#include "teclado.h"
#include "bascula.h"
#include "ui.h"

const uint16_t REFRESCO_PANTALLA_MS = 100;   // 10 cuadros por segundo sobran para leer un numero
const uint16_t REPORTE_SERIAL_MS    = 1000;

uint32_t ultimoRefresco = 0;
uint32_t ultimoReporte  = 0;

void setup() {
  Serial.begin(115200);
  delay(300);   // en las mini el USB es nativo: tiempo para que el monitor reconecte

  Serial.println();
  Serial.println("=== " PROYECTO_NOMBRE " v" PROYECTO_VERSION " ===");
  Serial.println("Desarrollado por " PROYECTO_AUTOR " - " PROYECTO_ING " - " PROYECTO_WEB);
  Serial.printf("Placa: %s  |  HX711 a %u SPS\n", NOMBRE_PLACA, HX_MUESTRAS_POR_SEGUNDO);

  if (!pantalla::iniciar()) {
    Serial.println("ERROR: la pantalla no inicio. Revisa el cableado SPI y config.h.");
  }

  teclado::iniciar();
  bascula::iniciar();     // desde aqui la celda se lee sola, en su propia tarea

  ui::arranque(2500);
  ui::pruebaHardwareEntrar();
}

void loop() {
  EventoTecla evento;
  while (teclado::leer(evento)) ui::pruebaHardwareTecla(evento);

  uint32_t ahora = millis();

  if (ahora - ultimoRefresco >= REFRESCO_PANTALLA_MS) {
    ultimoRefresco = ahora;
    ui::pruebaHardwareRefrescar(bascula::leer());
  }

  if (ahora - ultimoReporte >= REPORTE_SERIAL_MS) {
    ultimoReporte = ahora;
    Lectura l = bascula::leer();
    const char* estados[] = { "ok", "SIN RESPUESTA", "SATURADO", "TRAMA LENTA" };
    Serial.printf("crudo=%ld  sps=%.1f  ruido_pp=%ld  estado=%s  tramas=%lu  lentas=%lu  trama_max=%lu us\n",
                  (long)l.promedio, l.muestrasPorSegundo, (long)l.ruidoPicoPico, estados[l.estado],
                  (unsigned long)l.tramasLeidas, (unsigned long)l.tramasLentas, (unsigned long)l.tramaMaxMicros);
  }

  delay(2);   // cede el CPU: en un solo nucleo, loop() no debe girar en vacio
}

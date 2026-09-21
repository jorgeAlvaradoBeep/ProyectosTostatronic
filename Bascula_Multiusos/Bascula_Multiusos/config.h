/*
 * ============================================================
 *  CONFIGURACION - BASCULA MULTIUSOS
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 *  www.tostatronic.com
 * ============================================================
 *
 *  TODOS los pines y constantes del proyecto viven aqui. Ningun
 *  otro archivo lleva un numero de GPIO escrito.
 *
 *  1) Elige tu placa en "SELECCION DE PLACA".
 *  2) En el Arduino IDE elige la placa que corresponde:
 *
 *       PLACA_C6_DEVKIT   ESP32C6 Dev Module
 *       PLACA_C6_MINI     ESP32C6 Dev Module   + USB CDC On Boot: Enabled
 *       PLACA_C5_DEVKIT   ESP32C5 Dev Module
 *       PLACA_C5_MINI     ESP32C5 Dev Module   + USB CDC On Boot: Enabled
 *
 *     Partition Scheme (todas): Huge APP (3MB No OTA/1MB SPIFFS)
 *
 *     Las mini no traen convertidor USB-serie: su USB-C va directo
 *     al chip. Sin "USB CDC On Boot: Enabled" el sketch funciona,
 *     pero el monitor serie no muestra nada.
 *
 *  ------------------------------------------------------------
 *  PINES QUE NO SE TOCAN
 *  ------------------------------------------------------------
 *  ESP32-C6   4, 5, 8, 9, 15 ... strapping (definen el arranque)
 *             12, 13 ........... USB (se pierde la programacion)
 *             16, 17 ........... UART0 (monitor serie)
 *             24 a 30 .......... Flash interna
 *
 *  ESP32-C5   NO coincide con el C6. Segun Espressif:
 *             26, 27, 28 ....... strapping de MODO DE ARRANQUE: prohibidos
 *             13, 14 ........... USB
 *             11, 12 ........... UART0 (monitor serie)
 *             16 a 22 .......... Flash
 *             15 ............... CS de la PSRAM: libre SOLO en chips sin
 *                                PSRAM (la mini, C5HF4). En el DevKit no.
 *             2, 3, 7, 25 ...... strapping "de bajo riesgo": solo eligen la
 *                                fuente del JTAG y el flanco de muestreo
 *                                SDIO. No afectan el arranque y aqui SI se
 *                                usan, porque al C5 no le sobran pines
 *                                (GPIO7 es, de hecho, el MOSI nativo de SPI2).
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ------------------ SELECCION DE PLACA ------------------

#define PLACA_C6_DEVKIT  1   // ESP32-C6-DevKitC-1
#define PLACA_C6_MINI    2   // ESP32-C6 Super Mini
#define PLACA_C5_DEVKIT  3   // ESP32-C5-DevKitC-1
#define PLACA_C5_MINI    4   // ESP32-C5 Mini (NiceMCU ESP32-C5_MINI_V1.0)

#ifndef PLACA
  #define PLACA  PLACA_C6_DEVKIT     // <<< cambia aqui tu placa
#endif

#define SIN_PIN  (-1)   // senal que no va a un GPIO (se cablea fija, ver abajo)

// ------------------ PINES POR PLACA ------------------

#if PLACA == PLACA_C6_DEVKIT
  #define NOMBRE_PLACA    "ESP32-C6 DevKit"
  #define PIN_TFT_SCK     18
  #define PIN_TFT_MOSI    19
  #define PIN_TFT_CS      20
  #define PIN_TFT_DC      21
  #define PIN_TFT_RST     22
  #define PIN_TFT_BL      23
  #define PIN_HX_DOUT      6
  #define PIN_HX_SCK       7
  #define PIN_TECLA_1      0   // MENU
  #define PIN_TECLA_2      1   // ARRIBA
  #define PIN_TECLA_3      2   // ABAJO
  #define PIN_TECLA_4      3   // OK

#elif PLACA == PLACA_C6_MINI
  // La Super Mini solo tiene 10 pines "limpios" en el borde: se ahorran
  // dos cableando fijo   BL -> 3V3   y   RST de la pantalla -> 3V3
  // (esta placa no expone EN/RST; el reinicio de la pantalla se hace por
  // comando). Si algun dia falta un pin mas: CS de la pantalla a GND.
  #define NOMBRE_PLACA    "ESP32-C6 Mini"
  #define PIN_TFT_SCK     18
  #define PIN_TFT_MOSI    19
  #define PIN_TFT_CS      20
  #define PIN_TFT_DC      14
  #define PIN_TFT_RST     SIN_PIN   // -> 3V3
  #define PIN_TFT_BL      SIN_PIN   // -> 3V3
  #define PIN_HX_DOUT      6
  #define PIN_HX_SCK       7
  #define PIN_TECLA_1      0
  #define PIN_TECLA_2      1
  #define PIN_TECLA_3      2
  #define PIN_TECLA_4      3

#elif PLACA == PLACA_C5_DEVKIT
  #define NOMBRE_PLACA    "ESP32-C5 DevKit"
  #define PIN_TFT_SCK      6
  #define PIN_TFT_MOSI     7   // strapping de bajo riesgo (MOSI nativo de SPI2)
  #define PIN_TFT_DC       8
  #define PIN_TFT_RST      9
  #define PIN_TFT_CS      10
  #define PIN_TFT_BL      25   // strapping de bajo riesgo
  #define PIN_HX_DOUT      4
  #define PIN_HX_SCK       5
  #define PIN_TECLA_1     23
  #define PIN_TECLA_2     24
  #define PIN_TECLA_3      2   // strapping de bajo riesgo
  #define PIN_TECLA_4      3   // strapping de bajo riesgo

#elif PLACA == PLACA_C5_MINI
  // La mini usa el C5HF4 (sin PSRAM): GPIO15 queda libre y alcanza para
  // los 12 pines, con brillo por PWM incluido. Variante de ahorro si
  // necesitas pines:  BL -> 3V3  y  RST de la pantalla -> pin RST de la placa.
  #define NOMBRE_PLACA    "ESP32-C5 Mini"
  #define PIN_TFT_SCK      6
  #define PIN_TFT_MOSI     7   // strapping de bajo riesgo (MOSI nativo de SPI2)
  #define PIN_TFT_DC       8
  #define PIN_TFT_RST      9
  #define PIN_TFT_CS      10
  #define PIN_TFT_BL      15
  #define PIN_HX_DOUT      4
  #define PIN_HX_SCK       5
  #define PIN_TECLA_1      0
  #define PIN_TECLA_2      1
  #define PIN_TECLA_3      2   // strapping de bajo riesgo
  #define PIN_TECLA_4      3   // strapping de bajo riesgo

#else
  #error "PLACA no valida: usa PLACA_C6_DEVKIT, PLACA_C6_MINI, PLACA_C5_DEVKIT o PLACA_C5_MINI"
#endif

// Que la placa elegida aqui coincida con la elegida en el IDE: cablear
// con el mapa de un chip y compilar para el otro es dificil de depurar.
#if (PLACA == PLACA_C6_DEVKIT || PLACA == PLACA_C6_MINI) && !defined(CONFIG_IDF_TARGET_ESP32C6)
  #error "config.h dice ESP32-C6, pero en el IDE hay otra placa. Elige ESP32C6 Dev Module."
#endif
#if (PLACA == PLACA_C5_DEVKIT || PLACA == PLACA_C5_MINI) && !defined(CONFIG_IDF_TARGET_ESP32C5)
  #error "config.h dice ESP32-C5, pero en el IDE hay otra placa. Elige ESP32C5 Dev Module."
#endif

// ------------------ PANTALLA GC9A01 ------------------

const uint16_t TFT_ANCHO        = 240;
const uint16_t TFT_ALTO         = 240;
const uint32_t TFT_FRECUENCIA   = 40000000;  // 40 MHz. Con cables largos baja a 20 MHz
const uint8_t  TFT_ROTACION     = 0;         // 0..3, segun como montes la pantalla
const bool     TFT_IPS          = true;      // los modulos GC9A01 redondos son IPS
const uint8_t  TFT_BRILLO       = 200;       // 0..255 (solo si BL va a un GPIO)

// ------------------ HX711 ------------------

// Tu modulo esta a 80 muestras por segundo (pin RATE en alto). Si usas
// uno de fabrica (10 SPS), cambia este valor: de aqui salen las ventanas
// del filtro y los tiempos de estabilidad.
const uint8_t  HX_MUESTRAS_POR_SEGUNDO = 80;

// Si DOUT no baja en este tiempo, la celda esta desconectada o el HX711
// sin alimentacion. Se avisa en pantalla en vez de colgar la bascula.
const uint16_t HX_ESPERA_MS = 300;

// ------------------ TECLADO 1x4 ------------------
// Comun del teclado a GND; cada tecla a su GPIO con INPUT_PULLUP.

const uint16_t TECLA_REBOTE_MS    = 25;
const uint16_t TECLA_LARGA_MS     = 600;
const uint16_t TECLA_REPETIR_MS   = 110;   // repeticion mientras sigue presionada

// ------------------ ATRIBUCION ------------------

#define PROYECTO_NOMBRE   "Báscula Multiusos"
#define PROYECTO_VERSION  "0.1"
#define PROYECTO_AUTOR    "Tostatronic"
#define PROYECTO_ING      "Ing. Jorge Alvarado"
#define PROYECTO_WEB      "tostatronic.com"

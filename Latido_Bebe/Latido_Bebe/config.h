/*
 * ============================================================
 *  CONFIGURACION - LATIDO
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 *  www.tostatronic.com
 * ============================================================
 *
 *  Todos los pines y constantes del proyecto viven aqui.
 *
 *  Arduino IDE:
 *    Placa ............. ESP32C3 Dev Module
 *    USB CDC On Boot ... Enabled   (obligatorio: GPIO20 y 21 se usan para el audio)
 *    Partition Scheme .. No OTA (2MB APP/2MB SPIFFS)
 *                        La particion real sale de partitions.csv, que va junto
 *                        al sketch: 1.4 MB de programa y 2.5 MB para el video.
 *    Si cambias de particion despues de haber subido un video, el video se
 *    pierde (se vuelve a subir desde la pagina).
 *
 *  ------------------------------------------------------------
 *  ESP32-C3 SUPER MINI: PINES
 *  ------------------------------------------------------------
 *                      USB-C
 *              5  [ ]       [ ] 5V   <- bateria (por el interruptor)
 *              6  [ ]       [ ] GND
 *              7  [ ]       [ ] 3V3  -> pantalla
 *              8  [ ]       [ ] 4
 *              9  [ ]       [ ] 3
 *             10  [ ]       [ ] 2
 *             20  [ ]       [ ] 1
 *             21  [ ]       [ ] 0
 *
 *  GPIO2, 8 y 9 definen el arranque (strapping):
 *    2 .. no se usa.
 *    8 .. RST de la pantalla: en reposo esta en alto, que es lo que pide el
 *         arranque, y el LED azul de la placa (que cuelga de este pin) queda apagado.
 *    9 .. es el boton BOOT de la placa; aqui hace lo mismo que el boton externo.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

#define SIN_PIN  (-1)

// ------------------ PANTALLA GC9A01 (SPI) ------------------
#define PIN_TFT_SCK     4     // SCL
#define PIN_TFT_MOSI    6     // SDA
#define PIN_TFT_DC      5     // DC
#define PIN_TFT_CS      7     // CS
#define PIN_TFT_RST     8     // RST
#define PIN_TFT_BL      3     // BLK (brillo). Si tu modulo no trae BLK no pasa nada: se queda sin cablear.
#define TFT_FRECUENCIA  40000000UL   // con cables largos de protoboard, si ves basura baja a 27000000
#define TFT_ROTACION    0     // 0..3. Cambialo si la imagen sale girada en tu carcasa.

// ------------------ AMPLIFICADOR MAX98357A (I2S) ------------------
#define PIN_I2S_BCLK    20
#define PIN_I2S_LRC     21
#define PIN_I2S_DIN     10

// La bocina es de 0.5 W. Con la bateria (3.7 a 4.2 V) el MAX98357A le puede meter
// casi 1 W, asi que el 100 % del volumen se queda en esta fraccion de la escala.
#define VOLUMEN_TOPE    0.70f
#define VOLUMEN_INICIAL 70    // 0..100, la primera vez

// ------------------ BOTON (opcional) ------------------
// Entre el pin y GND. Toque corto: pausa / sigue. Dejarlo presionado 1.5 s:
// prende o apaga el WiFi. El boton BOOT de la placa hace lo mismo.
#define PIN_BOTON       1
#define PIN_BOTON_BOOT  9

// ------------------ BATERIA (opcional, recomendado) ------------------
// Divisor de dos resistencias de 100 k: BAT+ --[100k]--+--[100k]-- GND, el punto
// medio al pin. Si no lo cableas el firmware lo nota solo y no hace nada.
#define PIN_BATERIA         0
#define BATERIA_FACTOR      2.0f     // divisor 1:2
#define BATERIA_BAJA_MV     3350     // avisa
#define BATERIA_CORTE_MV    3200     // apaga pantalla y audio para cuidar la celda

// ------------------ WIFI ------------------
#define WIFI_NOMBRE     "Latido"
#define WIFI_CLAVE      "corazon123"     // minimo 8 caracteres; "" = red abierta
// Al encender, el WiFi queda prendido este tiempo. Si nadie se conecta se apaga
// solo para ahorrar bateria (sin video cargado se queda prendido).
#define WIFI_VENTANA_S  180

// ------------------ ARCHIVO ------------------
#define RUTA_VIDEO      "/latido.ltd"
#define VERSION_FW      "1.0.0"

// El monitor serie solo existe por USB: con "USB CDC On Boot" apagado, Serial
// usaria GPIO20/21, que aqui son del amplificador.
#if ARDUINO_USB_CDC_ON_BOOT
  #define REGISTRO(...)  Serial.printf(__VA_ARGS__)
#else
  #define REGISTRO(...)  do {} while (0)
#endif

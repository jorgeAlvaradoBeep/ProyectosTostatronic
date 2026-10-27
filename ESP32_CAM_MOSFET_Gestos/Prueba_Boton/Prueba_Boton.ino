/*
 * ============================================================
 *  PRUEBA CON BOTON - ESP32 + 74LS07 + IRFZ44N + modulo LED 12 V
 *  Tostatronic - www.tostatronic.com
 * ============================================================
 *
 *  Prueba el circuito de potencia sin WiFi ni camara: cada vez que
 *  presionas el boton BOOT de la placa (GPIO0), la carga cambia de
 *  estado. El LED azul de la placa (GPIO2) copia el estado de GPIO33.
 *
 *    GPIO33 en 1 -> 74LS07 suelta la salida -> pull-up 4.7k a 12 V
 *                -> compuerta del IRFZ44N a 12 V -> LED ENCENDIDO
 *    GPIO33 en 0 -> 74LS07 jala a tierra -> LED apagado
 *
 *  Si esto funciona, el circuito esta bien y cualquier falla del
 *  proyecto por gestos esta en el WiFi o la camara.
 *
 *  Arduino IDE: placa "ESP32 Wrover Module". Monitor serie a 115200.
 * ============================================================
 */

const uint8_t PIN_CARGA = 33;  // a la entrada 1A (pin 1) de la 74LS07
const uint8_t PIN_BOOT  = 0;   // boton BOOT de la placa (va a GND al presionarlo)
const uint8_t PIN_LED   = 2;   // LED de usuario de la placa

const uint16_t ANTIRREBOTE_MS = 40;

bool encendida = false;

void ponerCarga(bool encender) {
  encendida = encender;
  digitalWrite(PIN_CARGA, encender ? HIGH : LOW);
  digitalWrite(PIN_LED, encender ? HIGH : LOW);
  Serial.printf("[%lu ms] GPIO33 = %d  ->  LED %s\n", millis(), encender, encender ? "ENCENDIDO" : "apagado");
}

void setup() {
  pinMode(PIN_CARGA, OUTPUT);       // lo primero: la carga apagada
  digitalWrite(PIN_CARGA, LOW);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BOOT, INPUT_PULLUP);

  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== Prueba con boton | ESP32 + 74LS07 + IRFZ44N ===");
  Serial.println("Presiona BOOT para encender / apagar el LED de 12 V.");
  ponerCarga(false);
}

void loop() {
  static bool anterior = HIGH;
  static uint32_t cambio = 0;

  bool lectura = digitalRead(PIN_BOOT);
  if (lectura != anterior && millis() - cambio > ANTIRREBOTE_MS) {
    cambio = millis();
    anterior = lectura;
    if (lectura == LOW) ponerCarga(!encendida);   // solo al presionar, no al soltar
  }
}

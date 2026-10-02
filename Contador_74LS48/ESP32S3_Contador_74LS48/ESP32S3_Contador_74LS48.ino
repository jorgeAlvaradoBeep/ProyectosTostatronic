/*
 * ============================================================
 *  CONTADOR 0-9 CON ESP32-S3 + 74LS48 + DISPLAY 7 SEG
 *  Tostatronic - www.tostatronic.com
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Maneja un display de 7 segmentos de CATODO COMUN con solo
 *  4 GPIO del ESP32, usando el decodificador BCD 74LS48.
 *
 *  Placa: ESP32-S3 (DevKitC-1 y similares)
 *  Para el ESP32 clasico usa el sketch ESP32_Contador_74LS48.
 *
 *  OJO: el S3 no tiene GPIO 22, 23, 24 ni 25. Los pines del
 *  sketch clasico (25, 26, 27, 33) no sirven aqui.
 *
 *  ------------------------------------------------------------
 *  74LS48 = CATODO COMUN
 *  ------------------------------------------------------------
 *  Las salidas del 74LS48 son activas en ALTO y tienen
 *  resistencias pull-up internas de ~2 kohm: el segmento se
 *  enciende cuando la salida va a 1.  Por eso:
 *      - El display debe ser de CATODO COMUN (comun a GND).
 *      - Los segmentos se conectan DIRECTO, sin resistencias:
 *        la pull-up interna ya limita la corriente (~1.5 mA).
 *  Para display de ANODO COMUN se usa el 74LS47.
 *
 *  ------------------------------------------------------------
 *  ¿ESP32 A 3.3V CON UN CHIP TTL DE 5V?  SI, Y ASI DEBE SER.
 *  ------------------------------------------------------------
 *  El 74LS48 se alimenta con 5V (pin 5V del ESP32-S3).
 *  Sus entradas son TTL: reconocen un "1" desde 2.0V.
 *  El ESP32-S3 saca 3.3V  ==>  3.3V > 2.0V, dentro de especificacion.
 *  Las salidas del 48 solo van al display, nunca regresan al
 *  ESP32, asi que ningun pin del ESP32 ve 5V.
 *
 *  ------------------------------------------------------------
 *  CONEXIONES  (74LS48 en DIP-16)
 *  ------------------------------------------------------------
 *    ESP32-S3 GPIO4 -> pin 7   A   (bit 0, el de menor peso)
 *    ESP32-S3 GPIO5 -> pin 1   B   (bit 1)
 *    ESP32-S3 GPIO6 -> pin 2   C   (bit 2)
 *    ESP32-S3 GPIO7 -> pin 6   D   (bit 3, el de mayor peso)
 *    pin 3   LT      -> 5V   (prueba de lampara: activo en bajo)
 *    pin 5   RBI     -> 5V   (apagado del cero:   activo en bajo)
 *    pin 4   BI/RBO  -> 5V   (apagado total:      activo en bajo)
 *    pin 16  VCC     -> 5V
 *    pin 8   GND     -> GND  (tierra comun con el ESP32)
 *    pin 13,12,11,10,9,15,14  a..g -> segmentos a..g del display
 *    Display: pines 3 y 8 (comun) -> GND
 *
 *    Punto decimal (opcional): el 74LS48 NO lo maneja.
 *    ESP32-S3 GPIO15 -> R 1 kohm -> DP (pin 5 del display).
 *    Con 1 kohm el punto brilla parecido a los segmentos.
 *
 *    >>> CAPACITOR DE 0.1uF CERAMICO ENTRE PIN 16 Y PIN 8,
 *        PEGADO AL CHIP.
 *
 *  Los GPIO 4, 5, 6, 7 y 15 estan seguidos en el header del
 *  DevKitC-1, y ninguno es de strapping (0, 3, 45, 46).
 * ============================================================
 */

// ------------------ PLACA ------------------

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
  #error "Este sketch es para el ESP32-S3. Para el ESP32 clasico usa ESP32_Contador_74LS48."
#endif

// ------------------ CONFIGURACION ------------------

// Entradas BCD del 74LS48, de menor a mayor peso: A, B, C, D
const uint8_t PINES_BCD[4] = { 4, 5, 6, 7 };

// Punto decimal (opcional). Pon 255 si no lo conectaste.
const uint8_t PIN_DP = 15;

// Cada cuanto avanza el contador y parpadea el punto (ms)
const uint32_t INTERVALO_CUENTA = 1000;
const uint32_t INTERVALO_PUNTO  = 500;

// ------------------ SALIDA AL 74LS48 ------------------

/*
 * Pone en las 4 entradas el numero n en binario (BCD).
 * Bit 0 -> A, bit 1 -> B, bit 2 -> C, bit 3 -> D.
 * El 74LS48 hace el resto: decide que segmentos encender.
 */
void mostrar(uint8_t n) {
  for (uint8_t bit = 0; bit < 4; bit++) {
    digitalWrite(PINES_BCD[bit], (n >> bit) & 1);
  }
}

/*
 * 15 (1111) apaga todos los segmentos en el 74LS48.
 * Es lo mas limpio para "borrar" el display sin pines extra.
 */
void apagar() {
  mostrar(15);
}

void punto(bool encendido) {
  if (PIN_DP != 255) digitalWrite(PIN_DP, encendido);
}

// ------------------ ESTADO ------------------

uint8_t  numero     = 0;
bool     contando   = true;
bool     puntoOn    = false;
uint32_t tCuenta    = 0;
uint32_t tPunto     = 0;

// ------------------ MONITOR SERIE ------------------
//   0-9  -> muestra ese numero y pausa el contador
//   c    -> continua contando
//   x    -> apaga el display

void leerSerie() {
  while (Serial.available()) {
    char ch = Serial.read();

    if (ch >= '0' && ch <= '9') {
      numero = ch - '0';
      contando = false;
      mostrar(numero);
      punto(true);
      Serial.printf("Mostrando %u (pausa). Escribe 'c' para continuar.\n", numero);
    } else if (ch == 'c' || ch == 'C') {
      contando = true;
      tCuenta = millis();
      Serial.println("Contando...");
    } else if (ch == 'x' || ch == 'X') {
      contando = false;
      apagar();
      punto(false);
      Serial.println("Display apagado. Escribe un numero o 'c'.");
    }
  }
}

// ------------------ SETUP / LOOP ------------------

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < 4; i++) pinMode(PINES_BCD[i], OUTPUT);
  if (PIN_DP != 255) pinMode(PIN_DP, OUTPUT);

  // Prueba rapida al arrancar: 8 con punto durante medio segundo
  mostrar(8);
  punto(true);
  delay(500);

  mostrar(numero);
  punto(false);
  tCuenta = tPunto = millis();

  Serial.println();
  Serial.println("Contador 0-9 con 74LS48 - Tostatronic");
  Serial.println("Escribe 0-9 para mostrar un numero, 'c' para contar, 'x' para apagar.");
}

void loop() {
  leerSerie();
  if (!contando) return;

  uint32_t ahora = millis();

  if (ahora - tCuenta >= INTERVALO_CUENTA) {
    tCuenta += INTERVALO_CUENTA;
    numero = (numero + 1) % 10;
    mostrar(numero);
    Serial.printf("%u  (D C B A = %u %u %u %u)\n", numero,
                  (numero >> 3) & 1, (numero >> 2) & 1,
                  (numero >> 1) & 1, numero & 1);
  }

  if (ahora - tPunto >= INTERVALO_PUNTO) {
    tPunto += INTERVALO_PUNTO;
    puntoOn = !puntoOn;
    punto(puntoOn);
  }
}

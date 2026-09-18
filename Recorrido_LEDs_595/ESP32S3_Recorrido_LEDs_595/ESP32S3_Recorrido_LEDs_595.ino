/*
 * ============================================================
 *  RECORRIDO DE LEDs CON ESP32-S3 + 74HC595
 *  Tostatronic - www.tostatronic.com
 * ============================================================
 *
 *  Controla 8 LEDs (o 16, 24, 32) usando solo 3 GPIO del ESP32-S3.
 *
 *  Placa: ESP32-S3 (ESP32-S3-DevKitC-1 y similares)
 *  Para el ESP32 clasico usa el sketch ESP32_Recorrido_LEDs_595.
 *
 *  ------------------------------------------------------------
 *  OJO CON LOS PINES DEL S3
 *  ------------------------------------------------------------
 *  El S3 tiene un hueco en la numeracion: existen GPIO 0-21 y
 *  26-48, pero 22, 23, 24 y 25 NO EXISTEN. Los pines tipicos de
 *  los tutoriales del ESP32 clasico (23, 18, 5) no sirven aqui.
 *
 *  Si mueves los pines, evita:
 *      - 0, 3, 45, 46   (strapping)
 *      - 19 y 20        (USB nativo)
 *      - 26 a 32        (flash / PSRAM internos)
 *      - 33 a 37        (en modulos con flash/PSRAM octal: N8R8, N16R8)
 *
 *  ------------------------------------------------------------
 *  OJO CON LOS 3.3V  (esto es lo que rompe la mayoria de tutoriales)
 *  ------------------------------------------------------------
 *  El ESP32-S3 saca 3.3V en sus GPIO. Un 74HC595 alimentado a 5V
 *  exige VIH minimo de 0.7 x VCC = 3.5V para leer un "1".
 *  3.3V < 3.5V  ==>  queda FUERA de especificacion. A veces jala,
 *  a veces mete bits basura, y a veces solo falla cuando calienta.
 *
 *  Este sketch asume la solucion correcta:
 *      --> ALIMENTAR EL 74HC595 CON 3.3V (pin 3V3 del ESP32-S3)
 *          El HC595 trabaja de 2V a 6V, asi que a 3.3V va perfecto
 *          y el VIH baja a 2.31V. Cero conflicto de niveles.
 *
 *  Alternativas validas si necesitas 5V en el 595:
 *      - Usar 74HCT595 (entradas TTL, VIH = 2.0V) a 5V.
 *      - Usar 74LS595   (entradas TTL, VIH = 2.0V) a 5V.
 *      - Poner un level shifter en SER, SRCLK y RCLK.
 *  NUNCA 74HC595 a 5V manejado directo desde el ESP32-S3.
 *
 *  ------------------------------------------------------------
 *  LEDs EN MODO SUMIDERO  (el LED prende cuando la salida va en BAJO)
 *  ------------------------------------------------------------
 *      3.3V --> anodo LED --> catodo --> R 220 ohm --> salida Qx
 *
 *  Calculo: (3.3V - 2.0V Vf rojo - 0.3V VOL) / 0.005A = 200 ohm
 *           --> resistencia comercial 220 ohm, ~4.5 mA por LED.
 *  8 LEDs x 4.5 mA = 36 mA, muy por debajo del limite del chip.
 *
 *  Si prefieres modo fuente (anodo a la salida, catodo a GND),
 *  cambia MODO_SUMIDERO a false. El HC595 maneja ambos, pero el
 *  sumidero deja mas margen y es lo que se usa en produccion.
 *
 *  ------------------------------------------------------------
 *  CONEXIONES  (74HC595 en DIP-16)
 *  ------------------------------------------------------------
 *    ESP32-S3 GPIO11 -> pin 14  SER    (datos serie)
 *    ESP32-S3 GPIO12 -> pin 11  SRCLK  (reloj del shift register)
 *    ESP32-S3 GPIO10 -> pin 12  RCLK   (latch / storage clock)
 *    pin 13  OE    -> GND            (salidas siempre habilitadas)
 *    pin 10  SRCLR -> 3.3V           (nunca borrar)
 *    pin 16  VCC   -> 3.3V
 *    pin 8   GND   -> GND
 *    pin 15, 1..7  QA..QH -> LEDs (cada uno con su R de 220 ohm)
 *    pin 9   QH'   -> SER del siguiente 595 (para cascada)
 *
 *    >>> CAPACITOR DE 0.1uF CERAMICO ENTRE PIN 16 Y PIN 8,
 *        PEGADO AL CHIP. No es opcional: sin el vas a ver LEDs
 *        parpadeando donde no deben.
 *
 *  Los GPIO elegidos son MOSI, SCK y SS del bus SPI2/FSPI por
 *  hardware, asi que migrar a SPI nativo despues es cambiar solo
 *  la funcion escribir().
 *
 *  ------------------------------------------------------------
 *  MONITOR SERIE
 *  ------------------------------------------------------------
 *  Si tu placa tiene dos puertos USB, usa el marcado "UART"/"COM".
 *  Si solo tiene el USB nativo, activa en el Arduino IDE:
 *      Herramientas -> USB CDC On Boot -> Enabled
 * ============================================================
 */

// ------------------ PLACA ------------------

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
  #error "Este sketch es para el ESP32-S3. Para el ESP32 clasico usa ESP32_Recorrido_LEDs_595."
#endif

// ------------------ CONFIGURACION ------------------

const uint8_t PIN_DATA  = 11;   // SER   -> pin 14
const uint8_t PIN_CLOCK = 12;   // SRCLK -> pin 11
const uint8_t PIN_LATCH = 10;   // RCLK  -> pin 12

// Cuantos 74HC595 tienes encadenados (1 = 8 LEDs, 2 = 16 LEDs... maximo 4)
const uint8_t NUM_REGISTROS = 1;

// El patron viaja en un uint32_t: 4 registros x 8 bits es el tope.
static_assert(NUM_REGISTROS >= 1 && NUM_REGISTROS <= 4,
              "NUM_REGISTROS debe estar entre 1 y 4");

// true  = LED prende con salida en BAJO  (anodo comun a 3.3V)
// false = LED prende con salida en ALTO  (catodo comun a GND)
const bool MODO_SUMIDERO = true;

const uint8_t NUM_LEDS = NUM_REGISTROS * 8;

// ------------------ SALIDA AL 595 ------------------

/*
 * Envia el patron completo a la cadena de registros.
 * Bit en 1 = LED encendido. La inversion del modo sumidero
 * se aplica aqui adentro, para que los patrones se escriban
 * siempre en logica positiva y se lean facil.
 */
void escribir(uint32_t patron) {
  if (MODO_SUMIDERO) patron = ~patron;

  digitalWrite(PIN_LATCH, LOW);

  // El primer byte que sale termina en el ULTIMO chip de la cadena,
  // por eso se recorre del registro mas alto al mas bajo.
  for (int8_t reg = NUM_REGISTROS - 1; reg >= 0; reg--) {
    uint8_t b = (patron >> (8 * reg)) & 0xFF;
    shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, b);
  }

  digitalWrite(PIN_LATCH, HIGH);
}

// ------------------ PATRONES ------------------
// Cada patron es una funcion pura: recibe el numero de paso
// y devuelve que LEDs deben estar encendidos en ese instante.
// Asi no hay delay() bloqueando nada.

/* Mascara con los n bits bajos en 1. Un corrimiento de 32 sobre
 * un uint32_t es comportamiento indefinido, por eso el caso aparte. */
uint32_t mascara(uint8_t n) {
  return (n >= 32) ? 0xFFFFFFFF : ((1UL << n) - 1);
}

/* Un LED recorre de QA hasta el final */
uint32_t patronRecorrido(uint16_t paso) {
  return 1UL << paso;
}

/* Ida y vuelta, tipo Knight Rider */
uint32_t patronPingPong(uint16_t paso) {
  uint16_t i = (paso < NUM_LEDS) ? paso : (2 * NUM_LEDS - 2 - paso);
  return 1UL << i;
}

/* La barra se va llenando: de 1 LED hasta todos */
uint32_t patronLlenado(uint16_t paso) {
  return mascara(paso + 1);
}

/* La barra se va vaciando: de todos hasta 1 LED */
uint32_t patronVaciado(uint16_t paso) {
  return mascara(NUM_LEDS) & ~mascara(paso);
}

/* Contador binario de 8 bits: se ve en los primeros 8 LEDs */
uint32_t patronContador(uint16_t paso) {
  return paso;
}

/* Dos LEDs que salen de los extremos y se encuentran al centro */
uint32_t patronEncuentro(uint16_t paso) {
  return (1UL << paso) | (1UL << (NUM_LEDS - 1 - paso));
}

// ------------------ SECUENCIA ------------------

typedef uint32_t (*FuncionPatron)(uint16_t);

struct Patron {
  const char*   nombre;
  FuncionPatron funcion;
  uint16_t      pasos;       // cuantos pasos dura
  uint16_t      intervalo;   // ms entre paso y paso
  uint8_t       repeticiones;
};

Patron secuencia[] = {
  { "Recorrido",  patronRecorrido, NUM_LEDS,            80,  2 },
  { "Ping-pong",  patronPingPong,  2 * NUM_LEDS - 2,    70,  3 },
  { "Llenado",    patronLlenado,   NUM_LEDS,            70,  1 },
  { "Vaciado",    patronVaciado,   NUM_LEDS,            70,  1 },
  { "Encuentro",  patronEncuentro, NUM_LEDS / 2,       120,  3 },
  { "Contador",   patronContador,  256,                 45,  1 },
};

const uint8_t NUM_PATRONES = sizeof(secuencia) / sizeof(secuencia[0]);

uint8_t  patronActual = 0;
uint16_t pasoActual   = 0;
uint8_t  vueltaActual = 0;
uint32_t ultimoPaso   = 0;

// ------------------ SETUP / LOOP ------------------

void setup() {
  Serial.begin(115200);

  pinMode(PIN_DATA,  OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);

  escribir(0);   // arranca con todo apagado

  Serial.println();
  Serial.println("=== Recorrido de LEDs | ESP32-S3 + 74HC595 ===");
  Serial.printf("Registros: %u  |  LEDs: %u  |  Modo: %s\n",
                NUM_REGISTROS, NUM_LEDS,
                MODO_SUMIDERO ? "sumidero" : "fuente");
  Serial.printf("Patron: %s\n", secuencia[0].nombre);
}

void loop() {
  Patron& p = secuencia[patronActual];

  // Sin delay(): el loop queda libre para WiFi, botones,
  // sensores o lo que le quieras colgar despues.
  if (millis() - ultimoPaso < p.intervalo) return;
  ultimoPaso = millis();

  // Se acabo la vuelta: siguiente repeticion o siguiente patron.
  // Se revisa ANTES de escribir, asi el ultimo paso de cada vuelta
  // dura su intervalo completo en vez de ser pisado al instante.
  if (pasoActual >= p.pasos) {
    pasoActual = 0;
    vueltaActual++;

    if (vueltaActual >= p.repeticiones) {
      vueltaActual = 0;
      patronActual = (patronActual + 1) % NUM_PATRONES;
      escribir(0);   // respiro entre patrones (dura un intervalo)

      Serial.printf("Patron: %s\n", secuencia[patronActual].nombre);
      return;
    }
  }

  escribir(p.funcion(pasoActual));
  pasoActual++;
}

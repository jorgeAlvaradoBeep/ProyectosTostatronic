/*
 * ============================================================
 *  BASCULA BASICA - Arduino UNO + HX711 + LCD 16x2 I2C
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Version sencilla de la Bascula Multiusos (ESP32): solo pesar,
 *  tara, cero y calibracion con UN peso patron. Sin conteo de
 *  piezas y sin internet. La calibracion se guarda en la EEPROM.
 *
 *  CONEXIONES
 *    HX711   VCC -> 5V    GND -> GND    DT -> D3    SCK -> D2
 *    LCD     VCC -> 5V    GND -> GND    SDA -> A4   SCL -> A5
 *            (direccion 0x27 o 0x3F: se detecta sola)
 *    Teclado de membrana 1x4: comun -> GND, teclas -> D4..D7
 *            1 MENU (D4)   2 ARRIBA (D5)   3 ABAJO (D6)   4 OK (D7)
 *
 *  USO
 *    OK corto ........ tara (lo que hay en el plato pasa a ser 0)
 *    OK largo ........ cero (plato vacio = 0, quita la tara)
 *    MENU ............ calibrar
 *
 *  CALIBRAR
 *    1. Vacia el plato y presiona OK: toma el cero.
 *    2. Con ARRIBA / ABAJO elige cuantos gramos pesa tu patron
 *       (dejando presionado avanza rapido) y presiona OK.
 *    3. Pon el patron en el plato y presiona OK.
 *    En cualquier paso, MENU cancela y se queda la calibracion vieja.
 *
 *  Camino de cada muestra (igual que en la version ESP32):
 *    HX711 -> filtro robusto (mediana + descarte de atipicos)
 *          -> deteccion de estabilidad
 *          -> si esta estable, promedio largo y seguimiento de cero
 *          -> retencion del ultimo digito para la pantalla
 *
 *  Todo se guarda en CUENTAS del ADC. Los gramos se calculan solo
 *  al mostrar:   gramos = (cuentas - cero) / factor
 *  El factor puede ser negativo (celda al reves o A+/A- cruzados):
 *  la calibracion lo detecta sola.
 *
 *  Librerias: LiquidCrystal_I2C (Frank de Brabander). Wire y EEPROM
 *  vienen con el IDE. El HX711 se lee sin libreria.
 * ============================================================
 */

#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

// ------------------ PINES ------------------

const uint8_t PIN_HX_DOUT = 3;
const uint8_t PIN_HX_SCK  = 2;
const uint8_t PIN_TECLA[] = { 4, 5, 6, 7 };   // MENU, ARRIBA, ABAJO, OK

// ------------------ CELDA ------------------

// Capacidad de tu celda y su "division": el paso minimo que muestra la
// pantalla. No es la resolucion del HX711 (mucho mas fina), sino lo que
// la celda sostiene sin que el ultimo digito baile. Valores probados:
//   1 kg -> 0.1 g    5 kg -> 0.5 g    10 kg -> 1 g    20 kg -> 1 g
const float CAPACIDAD_G = 5000;
const float DIVISION_G  = 0.5f;

// ------------------ HX711 ------------------

// El modulo de fabrica da 10 muestras por segundo (RATE en bajo).
const uint8_t  HX_MUESTRAS_POR_SEGUNDO = 10;
// Si DOUT no baja en este tiempo, la celda esta desconectada.
const uint16_t HX_ESPERA_MS = 300;

// ------------------ FILTRO Y ESTABILIDAD ------------------

const uint8_t  VENTANA_FILTRO        = 5;      // impar: mediana exacta
const uint16_t ESTABLE_MS            = 1000;   // tiempo dentro de la banda para darlo por estable
const float    ESTABLE_BANDA_DIV     = 1.0f;   // ancho de la banda, en divisiones
const int32_t  ESTABLE_BANDA_SIN_CAL = 300;    // en cuentas, antes de calibrar
const float    ALFA_ESTABLE          = 2.0f / HX_MUESTRAS_POR_SEGUNDO;   // promedio largo de ~0.5 s
const float    HISTERESIS_DIV        = 0.6f;   // retencion del ultimo digito

const uint16_t MUESTRAS_ESTABLE = ESTABLE_MS * HX_MUESTRAS_POR_SEGUNDO / 1000;

// ------------------ CERO Y TARA ------------------

// Cero al arrancar: si el plato difiere del cero calibrado en menos de
// este porcentaje de la capacidad, se toma como cero nuevo (corrige la
// deriva de un dia a otro). Si es mas, se asume que hay algo encima.
const float    CERO_ARRANQUE_PORCIENTO = 10.0f;
// Con el plato vacio y estable, derivas menores a esto se corrigen solas.
const float    SEGUIMIENTO_CERO_DIV = 0.5f;
// Tara y cero solo se toman estable. Si se mueve, se espera hasta esto.
const uint16_t ESPERA_ESTABLE_MS = 4000;

// ------------------ CALIBRACION ------------------

const int32_t  CAL_CUENTAS_MINIMAS    = 5000;    // el patron debe mover al menos esto
const float    CAL_PESO_MIN_PORCIENTO = 5.0f;    // patron minimo, % de la capacidad
const uint8_t  CAL_MUESTRAS           = 10;      // muestras estables que se promedian
const uint16_t CAL_ESPERA_MS          = 15000;   // tope para que se asiente

// ------------------ TECLADO ------------------

const uint16_t TECLA_REBOTE_MS  = 25;
const uint16_t TECLA_LARGA_MS   = 600;
const uint16_t TECLA_REPETIR_MS = 110;

// ------------------ PANTALLA ------------------

const uint16_t MENSAJE_MS = 1500;
const uint16_t REFRESCO_MS = 150;

#define PROYECTO_VERSION "1.0"

// ============================================================
//  TIPOS
// ============================================================
//  Van todos aqui arriba, antes de la primera funcion: el IDE de
//  Arduino genera los prototipos de las funciones en ese punto y
//  tienen que conocer ya estos tipos.

enum Tecla : uint8_t { TECLA_MENU, TECLA_ARRIBA, TECLA_ABAJO, TECLA_OK, NUM_TECLAS };
enum TipoPulsacion : uint8_t { PULSACION_CORTA, PULSACION_LARGA, PULSACION_REPETIR };

enum Pantalla : uint8_t { P_PESAR, P_CAL_VACIO, P_CAL_CERO, P_CAL_PESO, P_CAL_PONER, P_CAL_MIDE, P_CAL_FIN };
enum Pendiente : uint8_t { NADA, PEND_TARA, PEND_CERO };

// Lo que se guarda en la EEPROM.
struct Guardado {
  uint16_t marca;
  float    factor;
  int32_t  offset;
  uint32_t pesoG;
  uint8_t  suma;
};

// ============================================================
//  TECLADO 1x4 - por sondeo con millis(), con antirrebote
// ============================================================

struct EstadoTecla {
  bool     lectura = false, estable = false, fueLarga = false;
  uint32_t cambioMs = 0, presionMs = 0, repeticionMs = 0;
};
EstadoTecla teclas[NUM_TECLAS];

// CORTA al soltar, LARGA una vez al cumplir TECLA_LARGA_MS, y luego
// REPETIR cada TECLA_REPETIR_MS mientras siga presionada.
bool leerTecla(Tecla& tecla, TipoPulsacion& tipo) {
  uint32_t ahora = millis();
  for (uint8_t i = 0; i < NUM_TECLAS; i++) {
    EstadoTecla& k = teclas[i];
    bool lectura = digitalRead(PIN_TECLA[i]) == LOW;

    if (lectura != k.lectura) { k.lectura = lectura; k.cambioMs = ahora; }
    if (lectura != k.estable && ahora - k.cambioMs >= TECLA_REBOTE_MS) {
      k.estable = lectura;
      if (k.estable) {
        k.presionMs = ahora;
        k.fueLarga  = false;
      } else if (!k.fueLarga) {
        tecla = (Tecla)i; tipo = PULSACION_CORTA;
        return true;
      }
    }
    if (!k.estable) continue;

    if (!k.fueLarga && ahora - k.presionMs >= TECLA_LARGA_MS) {
      k.fueLarga = true;
      k.repeticionMs = ahora;
      tecla = (Tecla)i; tipo = PULSACION_LARGA;
      return true;
    }
    if (k.fueLarga && ahora - k.repeticionMs >= TECLA_REPETIR_MS) {
      k.repeticionMs = ahora;
      tecla = (Tecla)i; tipo = PULSACION_REPETIR;
      return true;
    }
  }
  return false;
}

// ============================================================
//  HX711 - 24 pulsos de reloj, un bit por pulso
// ============================================================
//  Si SCK se queda en ALTO mas de 60 us, el HX711 se apaga a media
//  trama. Por eso cada pulso en alto va sin interrupciones (dura
//  unos pocos microsegundos); entre pulsos si se atienden.

int32_t leerHX711() {
  uint32_t valor = 0;
  for (uint8_t i = 0; i < 25; i++) {        // 24 bits + 1 pulso = canal A, ganancia 128
    noInterrupts();
    digitalWrite(PIN_HX_SCK, HIGH);
    delayMicroseconds(1);
    if (i < 24) valor = (valor << 1) | (uint32_t)digitalRead(PIN_HX_DOUT);
    digitalWrite(PIN_HX_SCK, LOW);
    interrupts();
    delayMicroseconds(1);
  }
  if (valor & 0x800000UL) valor |= 0xFF000000UL;   // complemento a dos -> con signo
  return (int32_t)valor;
}

// ============================================================
//  FILTRO ROBUSTO - mediana + descarte de atipicos
// ============================================================
//  1. MEDIANA de la ventana: una muestra disparada no la mueve.
//  2. MAD (desviacion absoluta mediana): la escala del ruido.
//  3. Se descartan las muestras a mas de 6 MAD y se promedian las demas.

void ordenar(int32_t* v, uint8_t n) {
  for (uint8_t i = 1; i < n; i++) {
    int32_t x = v[i];
    int8_t  j = i - 1;
    while (j >= 0 && v[j] > x) { v[j + 1] = v[j]; j--; }
    v[j + 1] = x;
  }
}

int32_t mediana(int32_t* v, uint8_t n) {
  ordenar(v, n);
  if (n & 1) return v[n / 2];
  return (v[n / 2 - 1] / 2) + (v[n / 2] / 2);
}

int32_t filtroRobusto(const int32_t* muestras, uint8_t n) {
  int32_t copia[VENTANA_FILTRO], desvio[VENTANA_FILTRO];
  for (uint8_t i = 0; i < n; i++) copia[i] = muestras[i];
  int32_t med = mediana(copia, n);

  for (uint8_t i = 0; i < n; i++) desvio[i] = labs(muestras[i] - med);
  // El +2 evita tirar muestras casi identicas (MAD = 0) por una cuenta.
  int32_t limite = 6 * mediana(desvio, n) + 2;

  int32_t suma = 0;
  uint8_t usadas = 0;
  for (uint8_t i = 0; i < n; i++) {
    // Se suma la distancia a la mediana: asi la suma no se desborda.
    if (labs(muestras[i] - med) <= limite) { suma += muestras[i] - med; usadas++; }
  }
  return usadas ? med + suma / usadas : med;
}

// ============================================================
//  BASCULA - estado, en cuentas del ADC
// ============================================================

struct Calibracion {
  float    factor = 0;    // cuentas por gramo (con signo)
  int32_t  offset = 0;    // cuentas con el plato vacio al calibrar
  uint32_t pesoG  = 0;    // patron que se uso (se propone la proxima vez)
  bool     valida = false;
};
Calibracion cal;

bool     hayCelda  = false;
bool     saturado  = false;
bool     estable   = false;
int32_t  filtrado  = 0;      // despues del filtro (y del promedio largo si esta estable)
int32_t  mostrado  = 0;      // filtrado con retencion del ultimo digito
int32_t  ceroCuentas = 0;
int32_t  taraCuentas = 0;    // relativa al cero; 0 = sin tara
bool     ceroArranquePendiente = true;
uint32_t ultimoDatoMs = 0;

int32_t  ventana[VENTANA_FILTRO];
uint8_t  indice = 0, llenas = 0;
uint16_t racha = 0;           // muestras seguidas dentro de la banda
int32_t  baseRacha = 0;       // primera muestra de la racha: las sumas van relativas a ella
int32_t  sumaRacha = 0;
float    suave = 0;

// Medicion para calibrar: promedio de CAL_MUESTRAS muestras estables.
bool     midiendo = false;
uint8_t  nMedida  = 0;
int32_t  sumaMedida = 0, baseMedida = 0;

float cuentasPorDivision() {
  if (!cal.valida) return ESTABLE_BANDA_SIN_CAL;
  return max(1.0f, DIVISION_G * fabs(cal.factor));
}

void ajustarCero() {
  if (!cal.valida) return;
  float cuentasPorG = fabs(cal.factor);

  if (ceroArranquePendiente) {
    ceroArranquePendiente = false;
    float lejania = fabs((float)(filtrado - cal.offset)) / cuentasPorG;
    if (lejania <= CAPACIDAD_G * CERO_ARRANQUE_PORCIENTO / 100.0f) ceroCuentas = mostrado = filtrado;
    return;
  }
  // Seguimiento: solo con el plato vacio y sin tara.
  if (taraCuentas == 0 && labs(filtrado - ceroCuentas) <= SEGUIMIENTO_CERO_DIV * DIVISION_G * cuentasPorG) {
    ceroCuentas = mostrado = filtrado;
  }
}

void procesarMuestra(int32_t crudo) {
  ventana[indice] = crudo;
  indice = (indice + 1) % VENTANA_FILTRO;
  if (llenas < VENTANA_FILTRO) llenas++;
  int32_t valor = filtroRobusto(ventana, llenas);

  // Estabilidad: racha de muestras cerca del promedio de la propia racha.
  int32_t banda = max(1L, (int32_t)(ESTABLE_BANDA_DIV * cuentasPorDivision()));
  if (racha > 0 && labs(valor - (baseRacha + sumaRacha / racha)) <= banda) {
    if (racha < 30000) { racha++; sumaRacha += valor - baseRacha; }
  } else {
    racha = 1;
    baseRacha = valor;
    sumaRacha = 0;
  }

  bool ahoraEstable = racha >= MUESTRAS_ESTABLE;
  if (ahoraEstable && estable) suave += (valor - suave) * ALFA_ESTABLE;
  else suave = valor;
  estable  = ahoraEstable;
  filtrado = lroundf(suave);

  // Retencion del ultimo digito: ya estable, lo mostrado se queda quieto
  // mientras el filtrado no se aleje mas de la histeresis.
  int32_t histeresis = max(1L, (int32_t)(HISTERESIS_DIV * cuentasPorDivision()));
  if (!estable || labs(filtrado - mostrado) > histeresis) mostrado = filtrado;
  if (estable) ajustarCero();

  if (midiendo) {
    if (!estable) nMedida = 0;
    else {
      if (nMedida == 0) { baseMedida = filtrado; sumaMedida = 0; }
      if (nMedida < CAL_MUESTRAS) { sumaMedida += filtrado - baseMedida; nMedida++; }
    }
  }
}

// Llamar seguido desde loop(): nunca se queda esperando al HX711.
void atenderHX711() {
  if (digitalRead(PIN_HX_DOUT) == LOW) {          // DOUT en bajo = dato listo
    int32_t crudo = leerHX711();
    ultimoDatoMs = millis();
    hayCelda = true;
    saturado = crudo == 0x7FFFFF || crudo == -0x800000L;
    procesarMuestra(crudo);
  } else if (millis() - ultimoDatoMs > HX_ESPERA_MS && hayCelda) {
    hayCelda = false;
    estable  = false;
    racha = llenas = indice = 0;   // al reconectar no se mezclan muestras viejas
  }
}

// Solo funcionan con el peso estable.
bool tarar() {
  if (!estable || !cal.valida) return false;
  int32_t bruto = filtrado - ceroCuentas;
  // Tarar con el plato vacio equivale a quitar la tara.
  bool vacio = labs(bruto) <= SEGUIMIENTO_CERO_DIV * DIVISION_G * fabs(cal.factor);
  taraCuentas = vacio ? 0 : bruto;
  mostrado = filtrado;
  return true;
}

bool hacerCero() {
  if (!estable) return false;
  ceroCuentas = mostrado = filtrado;
  taraCuentas = 0;
  ceroArranquePendiente = false;
  return true;
}

// ============================================================
//  EEPROM - la calibracion sobrevive al apagado
// ============================================================

const uint16_t MARCA_EEPROM = 0x7B01;

uint8_t sumaDe(const Guardado& g) {
  const uint8_t* p = (const uint8_t*)&g;
  uint8_t s = 0x5A;
  for (uint8_t i = 0; i < offsetof(Guardado, suma); i++) s += p[i];
  return s;
}

void cargarCalibracion() {
  Guardado g;
  EEPROM.get(0, g);
  if (g.marca != MARCA_EEPROM || g.suma != sumaDe(g) || !(fabs(g.factor) > 0.01f)) return;
  cal.factor = g.factor;
  cal.offset = g.offset;
  cal.pesoG  = g.pesoG;
  cal.valida = true;
  ceroCuentas = cal.offset;
}

void guardarCalibracion() {
  Guardado g = { MARCA_EEPROM, cal.factor, cal.offset, cal.pesoG, 0 };
  g.suma = sumaDe(g);
  EEPROM.put(0, g);
}

// ============================================================
//  PANTALLA LCD 16x2
// ============================================================

LiquidCrystal_I2C* lcd = nullptr;
char enPantalla[2][17];          // lo que ya esta escrito: solo se reescribe lo que cambia

char mensaje[17];
uint32_t mensajeHastaMs = 0;

// Los modulos traen el PCF8574 (0x20..0x27) o el PCF8574A (0x38..0x3F).
uint8_t buscarLCD() {
  for (uint8_t dir = 0x20; dir <= 0x3F; dir++) {
    if (dir == 0x28) dir = 0x38;
    Wire.beginTransmission(dir);
    if (Wire.endTransmission() == 0) return dir;
  }
  return 0x27;
}

void escribirLinea(uint8_t fila, const char* texto) {
  char linea[17];
  snprintf(linea, sizeof(linea), "%-16s", texto);
  if (strcmp(linea, enPantalla[fila]) == 0) return;
  strcpy(enPantalla[fila], linea);
  lcd->setCursor(0, fila);
  lcd->print(linea);
}

void avisar(const __FlashStringHelper* texto) {
  strncpy_P(mensaje, (const char*)texto, sizeof(mensaje) - 1);
  mensaje[sizeof(mensaje) - 1] = '\0';
  mensajeHastaMs = millis() + MENSAJE_MS;
}

bool hayMensaje() { return (int32_t)(mensajeHastaMs - millis()) > 0; }

uint8_t decimales() {
  if (DIVISION_G >= 1.0f) return 0;
  if (DIVISION_G >= 0.1f) return 1;
  return 2;
}

// Gramos redondeados a la division, como texto.
void formatearGramos(char* destino, float gramos) {
  gramos = roundf(gramos / DIVISION_G) * DIVISION_G;
  dtostrf(gramos, 0, decimales(), destino);
}

// ============================================================
//  PANTALLAS
// ============================================================

Pantalla pantalla = P_PESAR;
Pendiente pendiente = NADA;
uint32_t  pendienteMs = 0;

uint32_t calPesoG   = 0;
int32_t  calCero    = 0;
uint32_t calInicioMs = 0;
uint16_t repeticiones = 0;
uint32_t finMs = 0;

uint32_t pesoMinimoG() { return (uint32_t)ceil(CAPACIDAD_G * CAL_PESO_MIN_PORCIENTO / 100.0f); }

void empezarMedicion() {
  midiendo = true;
  nMedida = 0;
  calInicioMs = millis();
}

bool medicionLista() { return nMedida >= CAL_MUESTRAS; }
int32_t medicion()   { return baseMedida + sumaMedida / CAL_MUESTRAS; }

void irA(Pantalla p) {
  pantalla = p;
  midiendo = false;
  pendiente = NADA;
}

void dibujarPesar(char* l1, char* l2) {
  char num[12], tara[12];

  if (!hayCelda) {
    strcpy_P(l1, PSTR("Sin senal HX711"));
    strcpy_P(l2, PSTR("Revisa cables"));
    return;
  }
  if (!cal.valida) {
    strcpy_P(l1, PSTR("Sin calibrar"));
    strcpy_P(l2, PSTR("MENU = calibrar"));
    return;
  }

  float brutoG = (mostrado - ceroCuentas) / cal.factor;
  float taraG  = taraCuentas ? taraCuentas / cal.factor : 0;
  float netoG  = brutoG - taraG;
  // Nunca se muestran pesos negativos (como en la version ESP32).
  if (netoG < 0) netoG = 0;

  // Como las comerciales: se permite pasar la capacidad por 9 divisiones.
  if (saturado || brutoG > CAPACIDAD_G + 9 * DIVISION_G) {
    strcpy_P(l1, PSTR("  SOBRECARGA"));
    strcpy_P(l2, PSTR("Quita peso"));
    return;
  }

  formatearGramos(num, netoG);
  snprintf_P(l1, 17, PSTR("%s%10s g"), taraCuentas ? "Neto" : "Peso", num);

  tara[0] = '\0';
  if (taraCuentas) {
    formatearGramos(num, taraG);
    snprintf_P(tara, sizeof(tara), PSTR("T:%sg"), num);
  }
  const char* estado = pendiente != NADA ? "Espere" : (estable ? "Estable" : "");
  snprintf_P(l2, 17, PSTR("%-7s%9s"), estado, tara);
}

void dibujar() {
  char l1[17] = "", l2[17] = "";

  switch (pantalla) {
    case P_PESAR:
      dibujarPesar(l1, l2);
      break;
    case P_CAL_VACIO:
      strcpy_P(l1, PSTR("Vacia el plato"));
      strcpy_P(l2, PSTR("OK=sigue MENU=no"));
      break;
    case P_CAL_CERO:
      strcpy_P(l1, PSTR("Tomando cero..."));
      strcpy_P(l2, PSTR("No toques nada"));
      break;
    case P_CAL_PESO:
      strcpy_P(l1, PSTR("Peso del patron"));
      snprintf_P(l2, 17, PSTR("%7lu g   ^v OK"), (unsigned long)calPesoG);
      break;
    case P_CAL_PONER:
      snprintf_P(l1, 17, PSTR("Pon %lu g"), (unsigned long)calPesoG);
      strcpy_P(l2, PSTR("y presiona OK"));
      break;
    case P_CAL_MIDE:
      strcpy_P(l1, PSTR("Midiendo..."));
      strcpy_P(l2, PSTR("No toques nada"));
      break;
    case P_CAL_FIN: {
      char f[12];
      dtostrf(cal.factor, 0, 2, f);
      strcpy_P(l1, PSTR("Calibrada!"));
      snprintf_P(l2, 17, PSTR("%s cuentas/g"), f);
      break;
    }
  }
  if (!hayCelda && pantalla != P_PESAR) strcpy_P(l2, PSTR("Sin senal HX711"));
  if (hayMensaje()) strcpy(l2, mensaje);

  escribirLinea(0, l1);
  escribirLinea(1, l2);
}

// ============================================================
//  LOGICA DE CADA PANTALLA
// ============================================================

void cancelarCalibracion() {
  irA(P_PESAR);
  avisar(F("Cancelado"));
}

void atenderTecla(Tecla t, TipoPulsacion tipo) {
  if (pantalla != P_PESAR && pantalla != P_CAL_FIN && t == TECLA_MENU && tipo == PULSACION_CORTA) {
    cancelarCalibracion();
    return;
  }

  switch (pantalla) {
    case P_PESAR:
      if (t == TECLA_OK && tipo == PULSACION_CORTA && cal.valida) { pendiente = PEND_TARA; pendienteMs = millis(); }
      if (t == TECLA_OK && tipo == PULSACION_LARGA && cal.valida) { pendiente = PEND_CERO; pendienteMs = millis(); }
      if (t == TECLA_MENU && tipo == PULSACION_CORTA) irA(P_CAL_VACIO);
      break;

    case P_CAL_VACIO:
      if (t == TECLA_OK && tipo == PULSACION_CORTA) { irA(P_CAL_CERO); empezarMedicion(); }
      break;

    case P_CAL_PESO: {
      if (t != TECLA_ARRIBA && t != TECLA_ABAJO) {
        if (t == TECLA_OK && tipo == PULSACION_CORTA) irA(P_CAL_PONER);
        break;
      }
      // Dejando presionado avanza de 1 en 1, luego de 10 en 10 y de 100 en 100.
      if (tipo != PULSACION_REPETIR) repeticiones = 0;
      else repeticiones++;
      uint32_t paso = repeticiones < 10 ? 1 : (repeticiones < 25 ? 10 : 100);
      int32_t g = (int32_t)calPesoG;
      if (t == TECLA_ARRIBA) g = (g / (int32_t)paso + 1) * paso;
      else                   g = ((g + paso - 1) / paso - 1) * paso;
      calPesoG = constrain(g, (int32_t)pesoMinimoG(), (int32_t)CAPACIDAD_G);
      break;
    }

    case P_CAL_PONER:
      if (t == TECLA_OK && tipo == PULSACION_CORTA) { irA(P_CAL_MIDE); empezarMedicion(); }
      break;

    case P_CAL_FIN:
      if (tipo == PULSACION_CORTA) irA(P_PESAR);
      break;

    default:
      break;
  }
}

void atenderPantalla() {
  uint32_t ahora = millis();

  switch (pantalla) {
    case P_PESAR:
      if (pendiente == NADA) break;
      if (pendiente == PEND_TARA ? tarar() : hacerCero()) {
        if (pendiente == PEND_TARA) avisar(taraCuentas ? F("Tara lista") : F("Sin tara"));
        else avisar(F("Cero listo"));
        pendiente = NADA;
      } else if (ahora - pendienteMs > ESPERA_ESTABLE_MS) {
        avisar(F("Se mueve: no"));
        pendiente = NADA;
      }
      break;

    case P_CAL_CERO:
    case P_CAL_MIDE:
      if (ahora - calInicioMs > CAL_ESPERA_MS) {
        avisar(F("No se asienta"));
        empezarMedicion();
        break;
      }
      if (!medicionLista()) break;

      if (pantalla == P_CAL_CERO) {
        calCero = medicion();
        if (calPesoG < pesoMinimoG() || calPesoG > CAPACIDAD_G) calPesoG = (uint32_t)(CAPACIDAD_G / 2);
        irA(P_CAL_PESO);
      } else {
        int32_t delta = medicion() - calCero;
        if (labs(delta) < CAL_CUENTAS_MINIMAS) {
          // El patron casi no movio la lectura: celda mal cableada o no se puso.
          irA(P_CAL_PONER);
          avisar(F("Poco cambio"));
          break;
        }
        cal.factor = (float)delta / calPesoG;
        cal.offset = calCero;
        cal.pesoG  = calPesoG;
        cal.valida = true;
        ceroCuentas = calCero;
        taraCuentas = 0;
        ceroArranquePendiente = false;
        guardarCalibracion();
        Serial.print(F("[calibracion] cero="));
        Serial.print(calCero);
        Serial.print(F("  patron="));
        Serial.print(calPesoG);
        Serial.print(F(" g  cuentas="));
        Serial.print(medicion());
        Serial.print(F("  factor="));
        Serial.print(cal.factor, 3);
        Serial.println(F(" cuentas/g"));
        irA(P_CAL_FIN);
        finMs = ahora;
      }
      break;

    case P_CAL_FIN:
      if (ahora - finMs > 3000) irA(P_PESAR);
      break;

    default:
      break;
  }
}

// ============================================================
//  SETUP / LOOP
// ============================================================

void setup() {
  Serial.begin(115200);

  pinMode(PIN_HX_DOUT, INPUT);
  pinMode(PIN_HX_SCK, OUTPUT);
  digitalWrite(PIN_HX_SCK, LOW);   // SCK en bajo = HX711 despierto
  for (uint8_t i = 0; i < NUM_TECLAS; i++) pinMode(PIN_TECLA[i], INPUT_PULLUP);

  Wire.begin();
  static LiquidCrystal_I2C pantallaLcd(buscarLCD(), 16, 2);
  lcd = &pantallaLcd;
  lcd->init();
  lcd->backlight();

  cargarCalibracion();
  calPesoG = cal.pesoG;

  lcd->setCursor(0, 0);
  lcd->print(F("  Tostatronic"));
  lcd->setCursor(0, 1);
  lcd->print(F("  Bascula v" PROYECTO_VERSION));
  Serial.println(F("Bascula basica Tostatronic v" PROYECTO_VERSION));
  Serial.println(cal.valida ? F("Calibracion cargada de la EEPROM") : F("Sin calibrar: MENU para calibrar"));
  delay(1500);
  lcd->clear();
  memset(enPantalla, ' ', sizeof(enPantalla));
  enPantalla[0][16] = enPantalla[1][16] = '\0';

  ultimoDatoMs = millis();
}

void loop() {
  atenderHX711();

  Tecla t;
  TipoPulsacion tipo;
  if (leerTecla(t, tipo)) atenderTecla(t, tipo);

  atenderPantalla();

  static uint32_t refrescoMs = 0, serialMs = 0;
  if (millis() - refrescoMs >= REFRESCO_MS) {
    refrescoMs = millis();
    dibujar();
  }

  // Una linea por segundo al monitor serie, para ver las cuentas crudas.
  if (millis() - serialMs >= 1000) {
    serialMs = millis();
    Serial.print(F("filtrado="));
    Serial.print(filtrado);
    Serial.print(estable ? F(" estable") : F(" ......."));
    if (cal.valida) {
      Serial.print(F("  peso="));
      Serial.print((mostrado - ceroCuentas - taraCuentas) / cal.factor, decimales());
      Serial.print(F(" g"));
    }
    Serial.println(hayCelda ? F("") : F("  SIN SENAL HX711"));
  }
}

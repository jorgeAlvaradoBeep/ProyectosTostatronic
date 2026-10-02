/*
 * ============================================================
 *  VISTA: ASISTENTE DE CALIBRACION
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Una celda de carga entrega "cuentas", no gramos. Calibrar es
 *  encontrar dos numeros:
 *
 *    offset  cuentas con el plato vacio
 *    factor  cuentas por gramo = (cuentas con peso - offset) / gramos
 *
 *  Paso 1  Plato vacio ............ OK toma el offset
 *  Paso 2  Peso conocido encima ... OK toma las cuentas con peso
 *  Paso 3  Cuanto pesa ............ ARRIBA / ABAJO, OK guarda
 *
 *  Las capturas solo se hacen con la lectura ESTABLE (el anillo
 *  se llena mientras se asienta y se pone verde). Si se presiona
 *  OK antes, se espera a que se asiente.
 *
 *  MENU corta regresa un paso (o sale, desde el primero).
 * ============================================================
 */

#include "ui_comun.h"

namespace ui {
namespace vCalibrar {

namespace {

enum Paso : uint8_t { PASO_CERO, PASO_CARGA, PASO_VALOR, PASO_LISTO };

Paso     paso = PASO_CERO;
bool     primerArranque = false;
int32_t  cuentasCero = 0, cuentasCarga = 0;
uint32_t valorG = 500;
uint16_t repeticiones = 0;      // para acelerar ARRIBA / ABAJO sostenidas
bool     esperando = false;     // OK presionado, falta que se asiente
uint32_t esperaHastaMs = 0;
float    factorNuevo = 0;

uint32_t capacidadG() { return ajustes::actual().capacidadKg * 1000UL; }

void mostrarPaso() {
  limpiarVista();
  esperando = false;
  char texto[40];

  if (paso == PASO_LISTO) {
    pintar(zTitulo, "LISTO", COLOR_VERDE);
    pintar(zLinea1, "Calibración", COLOR_TEXTO);
    pintar(zLinea2, "guardada", COLOR_TEXTO);
    snprintf(texto, sizeof(texto), "%.1f cuentas por gramo", fabsf(factorNuevo));
    pintar(zDato, texto, COLOR_TENUE);
    pintar(zPie, "OK: pesar", COLOR_TENUE);
    anillo(1.0f, COLOR_VERDE);
    return;
  }

  pintar(zTitulo, "CALIBRAR", COLOR_AZUL);
  switch (paso) {
    case PASO_CERO:
      pintar(zLinea1, "Retira todo", COLOR_TEXTO);
      pintar(zLinea2, "el peso del plato", COLOR_TEXTO);
      pintar(zPie, "OK: tomar el cero", COLOR_TENUE);
      break;
    case PASO_CARGA:
      pintar(zLinea1, "Coloca un peso", COLOR_TEXTO);
      pintar(zLinea2, "conocido", COLOR_TEXTO);
      pintar(zPie, "OK: continuar", COLOR_TENUE);
      break;
    default:   // PASO_VALOR
      pintar(zUnidad, "g", COLOR_TENUE);
      pintar(zEstado, "ARRIBA / ABAJO: ajustar", COLOR_TENUE);
      pintar(zPie, "OK: guardar", COLOR_TENUE);
      break;
  }
}

const char* textoPaso() {
  switch (paso) {
    case PASO_CERO:  return primerArranque ? "Primer uso · paso 1 de 3" : "Paso 1 de 3";
    case PASO_CARGA: return "Paso 2 de 3";
    default:         return "Paso 3 · ¿cuánto pesa?";
  }
}

// Se llama cuando la lectura ya esta estable.
void capturar(const Lectura& l) {
  esperando = false;
  if (paso == PASO_CERO) {
    cuentasCero = l.filtrado;
    paso = PASO_CARGA;
    mostrarPaso();
    return;
  }
  // PASO_CARGA
  if (abs(l.filtrado - cuentasCero) < CAL_CUENTAS_MINIMAS) {
    aviso("Casi no cambió: pon más peso", COLOR_NARANJA, 2500);
    return;
  }
  cuentasCarga = l.filtrado;
  valorG = constrain(ajustes::actual().pesoCalG, 1UL, capacidadG());
  repeticiones = 0;
  paso = PASO_VALOR;
  mostrarPaso();
}

void guardar() {
  uint32_t minimoG = (uint32_t)ceilf(capacidadG() * CAL_PESO_MIN_PORCIENTO / 100.0f);
  if (valorG < minimoG) {
    char texto[40];
    snprintf(texto, sizeof(texto), "Usa al menos %lu g", (unsigned long)minimoG);
    aviso(texto, COLOR_NARANJA, 2500);
    return;
  }

  factorNuevo = (float)(cuentasCarga - cuentasCero) / valorG;

  Ajustes& a = ajustes::actual();
  a.cal.factor = factorNuevo;
  a.cal.offset = cuentasCero;
  a.cal.valida = true;
  a.pesoCalG   = valorG;
  ajustes::guardar();
  bascula::configurar(a.cal, a.capacidadKg);

  Serial.printf("[calibracion] cero=%ld  carga=%ld  peso=%lu g  factor=%.3f cuentas/g\n",
                (long)cuentasCero, (long)cuentasCarga, (unsigned long)valorG, factorNuevo);
  paso = PASO_LISTO;
  mostrarPaso();
}

// Paso de ARRIBA / ABAJO: de uno en uno al principio, y cada vez mas
// rapido mientras la tecla sigue presionada.
uint32_t incremento() {
  if (repeticiones < 10) return 1;
  if (repeticiones < 25) return 10;
  return 100;
}

}  // namespace

void entrar(bool primero) {
  primerArranque = primero;
  paso = PASO_CERO;
  mostrarPaso();
}

void tecla(const EventoTecla& e) {
  switch (e.tecla) {
    case TECLA_MENU:
      if (e.tipo != PULSACION_CORTA) return;
      if (paso == PASO_CERO || paso == PASO_LISTO) { irA(paso == PASO_LISTO ? V_PESAR : V_MENU); return; }
      paso = (Paso)(paso - 1);
      mostrarPaso();
      break;

    case TECLA_OK:
      if (e.tipo != PULSACION_CORTA) return;
      if (paso == PASO_LISTO) { irA(V_PESAR); return; }
      if (paso == PASO_VALOR) { guardar(); return; }
      if (bascula::leer().estable) capturar(bascula::leer());
      else { esperando = true; esperaHastaMs = millis() + ESPERA_ESTABLE_MS; }
      break;

    case TECLA_ARRIBA:
    case TECLA_ABAJO: {
      if (paso != PASO_VALOR) return;
      if (e.tipo != PULSACION_CORTA) repeticiones++;
      uint32_t delta = incremento();
      if (e.tecla == TECLA_ARRIBA) valorG = min(valorG + delta, capacidadG());
      else                         valorG = valorG > delta ? valorG - delta : 1;
      break;
    }

    default: break;
  }
}

void refrescar() {
  if (paso == PASO_LISTO) return;
  if (!avisoVigente()) pintar(zAviso, textoPaso(), COLOR_TENUE);

  char texto[40];

  if (paso == PASO_VALOR) {
    snprintf(texto, sizeof(texto), "%lu", (unsigned long)valorG);
    pintarNumero(zNumero, texto, COLOR_TEXTO);
    // Al soltar ARRIBA / ABAJO la velocidad vuelve a uno en uno.
    if (!teclado::presionada(TECLA_ARRIBA) && !teclado::presionada(TECLA_ABAJO)) repeticiones = 0;
    return;
  }

  // Pasos 1 y 2: la lectura en vivo y que tan asentada esta.
  Lectura l = bascula::leer();
  bool hayCelda = l.estado != HX_SIN_RESPUESTA && !l.doutAlAire;

  if (paso == PASO_CERO) snprintf(texto, sizeof(texto), "lectura: %ld", (long)l.filtrado);
  else                   snprintf(texto, sizeof(texto), "cambio: %ld cuentas", (long)(l.filtrado - cuentasCero));
  pintar(zDato, hayCelda ? texto : "", COLOR_TENUE);

  if (!hayCelda) {
    pintar(zEstado, "SIN CELDA: revisa el HX711", COLOR_NARANJA);
    anillo(1.0f, COLOR_NARANJA);
    esperando = false;
    return;
  }

  anillo(l.progresoEstable, l.estable ? COLOR_VERDE : COLOR_AMBAR);
  if (esperando)       pintar(zEstado, "esperando a que se asiente…", COLOR_AMBAR);
  else if (l.estable)  pintar(zEstado, "• ESTABLE", COLOR_VERDE);
  else                 pintar(zEstado, "asentándose…", COLOR_TENUE);

  if (esperando) {
    if (l.estable) capturar(l);
    else if ((int32_t)(millis() - esperaHastaMs) >= 0) {
      esperando = false;
      aviso("No se asentó: intenta de nuevo", COLOR_NARANJA, 2500);
    }
  }
}

}  // namespace vCalibrar
}  // namespace ui

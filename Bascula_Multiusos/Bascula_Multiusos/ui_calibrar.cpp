/*
 * ============================================================
 *  VISTA: ASISTENTE DE CALIBRACION
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Una celda de carga entrega "cuentas", no gramos. Calibrar es
 *  encontrar el cero y el factor (cuentas por gramo). Se hace con
 *  el plato vacio y TRES pesos conocidos, de menor a mayor: con
 *  uno solo, una pesa equivocada se copia a todo el rango y nadie
 *  lo nota; con tres, la que no concuerda se delata (la cuenta
 *  esta en calibracion.h).
 *
 *  Paso 1  Plato vacio ............ OK toma el cero
 *  Paso 2  Peso bajo (p. ej. 200 g) ARRIBA / ABAJO ajusta cuanto
 *  Paso 3  Peso medio (500 g)       pesa lo que se puso; OK lo
 *  Paso 4  Peso alto (1000 g)       toma cuando la lectura esta
 *                                   ESTABLE (anillo verde)
 *
 *  Si un peso no concuerda con los otros dos, se avisa cual y se
 *  repite desde ese; nada se guarda hasta que los tres concuerden.
 *  Al final se muestra el factor y lo mas que se aparto un peso.
 *
 *  La bascula propone los pesos (20, 50 y 100 % de la capacidad, o
 *  los que se usaron la ultima vez). MENU corta regresa un paso
 *  (o sale, desde el primero).
 * ============================================================
 */

#include "ui_comun.h"
#include "calibracion.h"

namespace ui {
namespace vCalibrar {

namespace {

enum Paso : uint8_t { PASO_CERO, PASO_PUNTO, PASO_REVISAR, PASO_LISTO };

const char* const NOMBRE_PUNTO[CAL_PUNTOS] = { "bajo", "medio", "alto" };

Paso     paso = PASO_CERO;
uint8_t  indice = 0;            // que peso se esta tomando (0..CAL_PUNTOS-1)
bool     primerArranque = false;
int32_t  cuentasCero = 0;
PuntoCal puntos[CAL_PUNTOS];
uint32_t valorG = 0;            // gramos del peso actual (lo que ajusta ARRIBA / ABAJO)
uint16_t repeticiones = 0;      // para acelerar ARRIBA / ABAJO sostenidas
bool     esperando = false;     // OK presionado, falta que se asiente
uint32_t esperaHastaMs = 0;
ResultadoCal resultado;

uint8_t  capacidadKg()    { return ajustes::actual().capacidadKg; }
uint32_t anteriorG()      { return indice ? puntos[indice - 1].gramos  : 0; }
int32_t  anteriorCuentas(){ return indice ? puntos[indice - 1].cuentas : cuentasCero; }

// "0.2 g" con los decimales de la celda.
void textoGramos(float g, char* texto, size_t tam) {
  if (bascula::peso().divisionG < 1.0f) snprintf(texto, tam, "%.1f g", g);
  else                                   snprintf(texto, tam, "%.0f g", g);
}

// Peso con el que arranca el punto 'indice': el que ya se habia tomado
// (si se esta repitiendo) o el que propone la bascula.
void proponer() {
  uint32_t g = puntos[indice].gramos;
  if (!g || calibracion::revisarPeso(indice, g, capacidadKg(), anteriorG())) g = calibracion::propuestaG(indice, capacidadKg(), anteriorG());
  valorG = g;
  repeticiones = 0;
}

void mostrarPaso() {
  limpiarVista();
  esperando = false;
  char texto[40], gramos[16];

  switch (paso) {
    case PASO_LISTO:
      pintar(zTitulo, "LISTO", COLOR_VERDE);
      pintar(zLinea1, "Calibración guardada", COLOR_TEXTO);
      textoGramos(resultado.desvioMaxG, gramos, sizeof(gramos));
      snprintf(texto, sizeof(texto), "desvío máximo %s", gramos);
      pintar(zLinea2, texto, COLOR_TEXTO);
      snprintf(texto, sizeof(texto), "%.1f cuentas por gramo", fabsf(resultado.factor));
      pintar(zDato, texto, COLOR_TENUE);
      pintar(zPie, "OK: pesar", COLOR_TENUE);
      anillo(1.0f, COLOR_VERDE);
      return;

    case PASO_REVISAR:
      pintar(zTitulo, "REVISA", COLOR_NARANJA);
      snprintf(texto, sizeof(texto), "El peso %u no concuerda", resultado.puntoMalo + 1);
      pintar(zLinea1, texto, COLOR_TEXTO);
      textoGramos(fabsf(resultado.desvioMaloG), gramos, sizeof(gramos));
      snprintf(texto, sizeof(texto), "se aparta %s", gramos);
      pintar(zLinea2, texto, COLOR_TEXTO);
      pintar(zDato, "de los otros · ¿pesa eso?", COLOR_TENUE);
      pintar(zPie, "OK: repetir desde ahí", COLOR_TENUE);
      anillo(1.0f, COLOR_NARANJA);
      return;

    case PASO_CERO:
      pintar(zTitulo, "CALIBRAR", COLOR_AZUL);
      pintar(zLinea1, "Retira todo", COLOR_TEXTO);
      pintar(zLinea2, "el peso del plato", COLOR_TEXTO);
      pintar(zPie, "OK: tomar el cero", COLOR_TENUE);
      return;

    default:   // PASO_PUNTO
      pintar(zTitulo, "CALIBRAR", COLOR_AZUL);
      pintar(zUnidad, "g · ARRIBA/ABAJO", COLOR_TENUE);
      pintar(zPie, "OK: tomar el peso", COLOR_TENUE);
      return;
  }
}

void textoPaso(char* texto, size_t tam) {
  if (paso == PASO_CERO) strlcpy(texto, primerArranque ? "Primer uso · paso 1 de 4" : "Paso 1 de 4", tam);
  else snprintf(texto, tam, "Peso %u de %u · %s", indice + 1, CAL_PUNTOS, NOMBRE_PUNTO[indice]);
}

// El peso escrito no sirve: se explica por que (mismos motivos que la web).
void avisarPeso(const char* motivo) {
  char texto[40];
  if (strcmp(motivo, "orden_pesos") == 0)
    snprintf(texto, sizeof(texto), "Debe ser mayor que %lu g", (unsigned long)anteriorG());
  else if (strcmp(motivo, "peso_alto") == 0)
    snprintf(texto, sizeof(texto), "El último: al menos %lu g", (unsigned long)calibracion::minimoG(indice, capacidadKg(), anteriorG()));
  else
    snprintf(texto, sizeof(texto), "Usa de %lu a %lu g", (unsigned long)calibracion::minimoG(indice, capacidadKg(), anteriorG()),
             (unsigned long)calibracion::maximoG(indice, capacidadKg()));
  aviso(texto, COLOR_NARANJA, 2500);
}

// Se llama cuando la lectura ya esta estable.
void capturar(const Lectura& l) {
  esperando = false;
  if (paso == PASO_CERO) {
    cuentasCero = l.filtrado;
    paso = PASO_PUNTO;
    indice = 0;
    proponer();
    mostrarPaso();
    return;
  }

  // PASO_PUNTO
  if (calibracion::revisarCuentas(indice, l.filtrado, cuentasCero, anteriorCuentas())) {
    aviso(indice ? "Casi no cambió: ¿pusiste el peso?" : "Casi no cambió: pon más peso", COLOR_NARANJA, 2500);
    return;
  }
  puntos[indice].gramos  = valorG;
  puntos[indice].cuentas = l.filtrado;

  if (indice + 1 < CAL_PUNTOS) {
    indice++;
    proponer();
    mostrarPaso();
    return;
  }

  resultado = calibracion::calcular(cuentasCero, puntos, bascula::peso().divisionG);
  if (!resultado.ok) {
    paso = PASO_REVISAR;
    mostrarPaso();
    return;
  }
  calibracion::guardar(cuentasCero, puntos, resultado);
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
  indice = 0;
  for (uint8_t i = 0; i < CAL_PUNTOS; i++) puntos[i] = PuntoCal();
  mostrarPaso();
}

void tecla(const EventoTecla& e) {
  switch (e.tecla) {
    case TECLA_MENU:
      if (e.tipo != PULSACION_CORTA) return;
      if (paso == PASO_LISTO)                           { irA(V_PESAR); return; }
      if (paso == PASO_CERO || paso == PASO_REVISAR)    { irA(V_MENU);  return; }
      // PASO_PUNTO: al peso anterior (se vuelve a tomar) o al cero.
      if (indice == 0) paso = PASO_CERO;
      else { indice--; proponer(); }
      mostrarPaso();
      break;

    case TECLA_OK:
      if (e.tipo != PULSACION_CORTA) return;
      if (paso == PASO_LISTO)   { irA(V_PESAR); return; }
      if (paso == PASO_REVISAR) {
        paso = PASO_PUNTO;
        indice = resultado.puntoMalo;
        proponer();
        mostrarPaso();
        return;
      }
      if (paso == PASO_PUNTO) {
        const char* motivo = calibracion::revisarPeso(indice, valorG, capacidadKg(), anteriorG());
        if (motivo) { avisarPeso(motivo); return; }
      }
      if (bascula::leer().estable) capturar(bascula::leer());
      else { esperando = true; esperaHastaMs = millis() + ESPERA_ESTABLE_MS; }
      break;

    case TECLA_ARRIBA:
    case TECLA_ABAJO: {
      if (paso != PASO_PUNTO) return;
      if (e.tipo != PULSACION_CORTA) repeticiones++;
      uint32_t delta  = incremento();
      uint32_t maximo = calibracion::maximoG(indice, capacidadKg());
      if (e.tecla == TECLA_ARRIBA) valorG = min(valorG + delta, maximo);
      else                         valorG = valorG > delta ? valorG - delta : 1;
      break;
    }

    default: break;
  }
}

void refrescar() {
  if (paso == PASO_LISTO || paso == PASO_REVISAR) return;

  char texto[40];
  if (!avisoVigente()) { textoPaso(texto, sizeof(texto)); pintar(zAviso, texto, COLOR_TENUE); }

  Lectura l = bascula::leer();
  bool hayCelda = l.estado != HX_SIN_RESPUESTA && !l.doutAlAire;

  if (paso == PASO_PUNTO) {
    snprintf(texto, sizeof(texto), "%lu", (unsigned long)valorG);
    pintarNumero(zNumero, texto, COLOR_TEXTO);
    // Al soltar ARRIBA / ABAJO la velocidad vuelve a uno en uno.
    if (!teclado::presionada(TECLA_ARRIBA) && !teclado::presionada(TECLA_ABAJO)) repeticiones = 0;
  } else {
    snprintf(texto, sizeof(texto), "lectura: %ld", (long)l.filtrado);
    pintar(zDato, hayCelda ? texto : "", COLOR_TENUE);
  }

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

/*
 * ============================================================
 *  BASCULA - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "bascula.h"
#include "config.h"
#include "filtro.h"

namespace {

HX711   celda;
Lectura foto;
portMUX_TYPE candado = portMUX_INITIALIZER_UNLOCKED;   // protege foto y el estado de abajo

// ---- estado de la bascula (siempre bajo el candado) ----
Calibracion cal;
int32_t ceroCuentas  = 0;       // cero vigente: arranque, tecla o seguimiento
int32_t taraCuentas  = 0;       // relativa al cero; 0 = sin tara
bool    ceroArranquePendiente = true;
float   divisionG    = 0.1f;
float   capacidadG   = 1000;

// ---- ventanas, derivadas de las muestras por segundo ----
constexpr uint8_t impar(uint8_t n) { return n | 1; }

// ~1/10 de segundo de muestras, pero nunca menos de 5 (con menos, la
// mediana no puede descartar ni dos atipicos). Impar: mediana exacta.
const uint8_t VENTANA_FILTRO = impar(constrain(HX_MUESTRAS_POR_SEGUNDO / 10, 5, 15));

// Muestras seguidas dentro de la banda para declararlo estable.
const uint16_t MUESTRAS_ESTABLE = max(3, (int)(ESTABLE_MS * HX_MUESTRAS_POR_SEGUNDO / 1000));

// Promedio largo cuando ya esta estable: constante de tiempo de ~0.5 s.
const float ALFA_ESTABLE = min(1.0f, 2.0f / HX_MUESTRAS_POR_SEGUNDO);

// Banda de estabilidad en cuentas, segun la calibracion vigente.
int32_t bandaEstable() {
  if (!cal.valida) return ESTABLE_BANDA_SIN_CAL;
  return max<int32_t>(1, (int32_t)(ESTABLE_BANDA_DIV * divisionG * fabsf(cal.factor)));
}

// Se llama con cada muestra estable (ya dentro del candado).
void ajustarCero(int32_t filtrado) {
  if (!cal.valida) return;
  float cuentasPorG = fabsf(cal.factor);

  if (ceroArranquePendiente) {
    ceroArranquePendiente = false;
    float lejania = fabsf((float)(filtrado - cal.offset)) / cuentasPorG;
    if (lejania <= capacidadG * CERO_ARRANQUE_PORCIENTO / 100.0f) ceroCuentas = filtrado;
    return;
  }
  // Seguimiento: solo con el plato vacio y sin tara.
  if (taraCuentas == 0 && abs(filtrado - ceroCuentas) <= SEGUIMIENTO_CERO_DIV * divisionG * cuentasPorG) {
    ceroCuentas = filtrado;
  }
}

void tareaBascula(void*) {
  Lectura  actual;
  int32_t  ventana[filtro::VENTANA_MAX] = {0};
  uint8_t  indice = 0, llenas = 0;
  int32_t  previo1 = 0, previo2 = 0;   // dos muestras anteriores, para ver picos aislados
  uint8_t  historia = 0;

  // Estabilidad: racha de muestras cerca del promedio de la propia racha.
  uint16_t racha = 0;
  int64_t  sumaRacha = 0;
  float    suave = 0;              // promedio largo, solo mientras esta estable

  uint32_t inicioSegundo = millis();
  uint16_t muestrasSegundo = 0;
  int32_t  minimo = INT32_MAX, maximo = INT32_MIN;

  while (true) {
    int32_t crudo = 0;
    actual.estado = celda.leer(crudo, HX_ESPERA_MS);

    if (actual.estado == HX_OK) {
      actual.crudo = crudo;

      ventana[indice] = crudo;
      indice = (indice + 1) % VENTANA_FILTRO;
      if (llenas < VENTANA_FILTRO) llenas++;

      filtro::Resultado f = filtro::robusto(ventana, llenas);

      // Contador de atipicas para el Diagnostico. Una trama danada deja un
      // PICO AISLADO: la muestra anterior se aleja mucho de su vecina de
      // antes y de la de despues, que entre si coinciden. Poner peso no
      // cuenta (es un escalon: la muestra nueva se queda donde llego).
      if (historia >= 2) {
        int32_t umbral = max(max(4 * f.limite, 3 * actual.ruidoPicoPico), (int32_t)200);
        if (abs(previo1 - previo2) > umbral && abs(previo1 - crudo) > umbral && abs(crudo - previo2) <= umbral / 2) {
          actual.atipicos++;
        }
      }
      previo2 = previo1;
      previo1 = crudo;
      if (historia < 2) historia++;

      portENTER_CRITICAL(&candado);
      int32_t banda = bandaEstable();
      portEXIT_CRITICAL(&candado);

      if (racha > 0 && abs(f.valor - (int32_t)(sumaRacha / racha)) <= banda) {
        racha++;
        sumaRacha += f.valor;
      } else {
        racha = 1;
        sumaRacha = f.valor;
      }
      if (racha > 60000) { sumaRacha = sumaRacha / racha * 1000; racha = 1000; }   // sin desbordes

      bool estable = racha >= MUESTRAS_ESTABLE;
      if (estable && actual.estable) suave += (f.valor - suave) * ALFA_ESTABLE;
      else suave = f.valor;

      actual.estable         = estable;
      actual.progresoEstable = min(1.0f, (float)racha / MUESTRAS_ESTABLE);
      actual.filtrado        = (int32_t)lroundf(suave);

      muestrasSegundo++;
      if (crudo < minimo) minimo = crudo;
      if (crudo > maximo) maximo = crudo;
    } else {
      racha = 0;
      actual.estable = false;
      actual.progresoEstable = 0;
      if (actual.estado == HX_SIN_RESPUESTA) llenas = indice = historia = 0;   // al reconectar no se mezclan muestras viejas
    }

    uint32_t transcurrido = millis() - inicioSegundo;
    if (transcurrido >= 1000) {
      actual.muestrasPorSegundo = muestrasSegundo * 1000.0f / transcurrido;
      actual.ruidoPicoPico      = muestrasSegundo ? (maximo - minimo) : 0;
      // Un DOUT al aire se lee siempre "listo" y dispara las muestras por
      // segundo muy por encima de lo que el HX711 puede dar.
      actual.doutAlAire         = actual.muestrasPorSegundo > HX_MUESTRAS_POR_SEGUNDO * 2.0f;
      inicioSegundo   = millis();
      muestrasSegundo = 0;
      minimo = INT32_MAX;
      maximo = INT32_MIN;
    }

    actual.tramasLeidas   = celda.tramasLeidas();
    actual.tramasLentas   = celda.tramasLentas();
    actual.tramaMaxMicros = celda.tramaMaxMicros();

    portENTER_CRITICAL(&candado);
    foto = actual;
    if (actual.estable && !actual.doutAlAire) ajustarCero(actual.filtrado);
    portEXIT_CRITICAL(&candado);

    // Respiro obligatorio. Con el HX711 bien conectado no cuesta nada (el
    // siguiente dato tarda 100 ms), pero si DOUT queda al aire y se lee
    // siempre en bajo, sin esto la tarea acapararia el CPU y dejaria sin
    // tiempo a la pantalla y al WiFi.
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

}  // namespace

namespace bascula {

void iniciar() {
  celda.iniciar(PIN_HX_DOUT, PIN_HX_SCK);

  // Prioridad por encima de loop() (1) y del servidor web (5): cuando hay
  // dato listo se atiende ya. No acapara el CPU: casi todo el tiempo esta
  // dormida esperando al HX711.
  xTaskCreate(tareaBascula, "bascula", 4096, nullptr, 6, nullptr);
}

void configurar(const Calibracion& nueva, uint8_t capacidadKg) {
  float division = CELDAS[0].divisionG;
  for (uint8_t i = 0; i < NUM_CELDAS; i++) {
    if (CELDAS[i].capacidadKg == capacidadKg) division = CELDAS[i].divisionG;
  }

  portENTER_CRITICAL(&candado);
  cal         = nueva;
  divisionG   = division;
  capacidadG  = capacidadKg * 1000.0f;
  ceroCuentas = nueva.offset;
  taraCuentas = 0;
  ceroArranquePendiente = true;
  portEXIT_CRITICAL(&candado);
}

Lectura leer() {
  portENTER_CRITICAL(&candado);
  Lectura copia = foto;
  portEXIT_CRITICAL(&candado);
  return copia;
}

Peso peso() {
  portENTER_CRITICAL(&candado);
  Lectura     l    = foto;
  Calibracion c    = cal;
  int32_t     cero = ceroCuentas, tara = taraCuentas;
  Peso p;
  p.divisionG  = divisionG;
  p.capacidadG = capacidadG;
  portEXIT_CRITICAL(&candado);

  p.estado          = l.estado;
  p.hayCelda        = l.estado != HX_SIN_RESPUESTA && !l.doutAlAire;
  p.calibrada       = c.valida;
  p.estable         = l.estable;
  p.progresoEstable = l.progresoEstable;

  if (c.valida) {
    p.brutoG     = (l.filtrado - cero) / c.factor;
    p.taraG      = tara ? tara / c.factor : 0;   // sin "-0.00" cuando no hay tara
    p.netoG      = p.brutoG - p.taraG;
    p.conTara    = tara != 0;
    // Como en las comerciales: se permite pasar la capacidad por 9 divisiones.
    p.sobrecarga = l.estado == HX_SATURADO || p.brutoG > p.capacidadG + 9 * p.divisionG;
  }
  return p;
}

bool tarar() {
  bool hecho = false;
  portENTER_CRITICAL(&candado);
  if (foto.estable && cal.valida) {
    int32_t bruto = foto.filtrado - ceroCuentas;
    // Tarar con el plato vacio equivale a quitar la tara.
    bool vacio = abs(bruto) <= SEGUIMIENTO_CERO_DIV * divisionG * fabsf(cal.factor);
    taraCuentas = vacio ? 0 : bruto;
    hecho = true;
  }
  portEXIT_CRITICAL(&candado);
  return hecho;
}

bool cero() {
  bool hecho = false;
  portENTER_CRITICAL(&candado);
  if (foto.estable) {
    ceroCuentas = foto.filtrado;
    taraCuentas = 0;
    ceroArranquePendiente = false;
    hecho = true;
  }
  portEXIT_CRITICAL(&candado);
  return hecho;
}

void quitarTara() {
  portENTER_CRITICAL(&candado);
  taraCuentas = 0;
  portEXIT_CRITICAL(&candado);
}

}  // namespace bascula

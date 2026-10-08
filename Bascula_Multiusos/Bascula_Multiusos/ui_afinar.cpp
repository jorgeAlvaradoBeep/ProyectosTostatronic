/*
 * ============================================================
 *  VISTA: AFINAR RUIDO (escuchar la celda y ajustar los filtros)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Cada celda, cada mesa y cada fuente hacen un ruido distinto.
 *  Este asistente lo MIDE y ajusta los filtros a lo que hay:
 *
 *    Paso 1  Plato vacio ... escucha AFINAR_ESCUCHA_MS: ruido en reposo
 *    Paso 2  Coloca cualquier objeto (lo detecta sola): mide cuanto
 *            tarda en asentarse y vuelve a escuchar con carga
 *    Paso 3  Retira el objeto: mide otra vez el asentamiento
 *
 *  Con el ruido (desviacion estandar en cuentas) y el asentamiento
 *  se arma un PerfilRuido (ver ruido.h): tamano del filtro, banda
 *  de estabilidad y promedio largo. Se guarda en la NVS y se aplica
 *  al instante. MENU cancela (o regresa, desde el primer paso).
 * ============================================================
 */

#include "ui_comun.h"

namespace ui {
namespace vAfinar {

namespace {

enum Paso : uint8_t { P_VACIO, P_ESCUCHA1, P_PONER, P_ASENTAR1, P_ESCUCHA2, P_QUITAR, P_ASENTAR2, P_LISTO };

Paso     paso = P_VACIO;
float    sigma1 = 0, sigma2 = 0;      // ruido en reposo: plato vacio y con objeto (cuentas)
float    media1 = 0, media2 = 0;      // lectura media en cada escucha: referencia para detectar el objeto
uint32_t t0 = 0, asent1 = 0, asent2 = 0;
bool     noSeAsento = false;
PerfilRuido resultado;

float cuentasPorDivision() {
  Ajustes& a = ajustes::actual();
  if (!a.cal.valida) return ESTABLE_BANDA_SIN_CAL;
  return max(1.0f, bascula::peso().divisionG * fabsf(a.cal.factor));
}

// "±0.08 g" si esta calibrada; si no, en cuentas.
void textoRuido(float sigmaCuentas, char* texto, size_t tam) {
  Ajustes& a = ajustes::actual();
  if (a.cal.valida) snprintf(texto, tam, "ruido ±%.2f g", sigmaCuentas / fabsf(a.cal.factor));
  else              snprintf(texto, tam, "ruido ±%.0f cuentas", sigmaCuentas);
}

// Un objeto "cuenta" si mueve la lectura mas que esto.
int32_t escalonCuentas() {
  return (int32_t)max(AFINAR_ESCALON_DIV * cuentasPorDivision(), 8.0f * max(sigma1, sigma2));
}

const char* textoPaso() {
  switch (paso) {
    case P_VACIO: case P_ESCUCHA1:               return "Paso 1 de 3 · plato vacío";
    case P_PONER: case P_ASENTAR1: case P_ESCUCHA2: return "Paso 2 de 3 · con un objeto";
    default:                                     return "Paso 3 de 3 · sin el objeto";
  }
}

void mostrarPaso() {
  limpiarVista();
  char texto[40];

  if (paso == P_LISTO) {
    pintar(zTitulo, "LISTO", COLOR_VERDE);
    pintar(zAviso, "filtros ajustados a tu celda", COLOR_TENUE);
    textoRuido(resultado.sigmaCuentas, texto, sizeof(texto));
    texto[0] = 'R';   // "Ruido ..." con mayuscula
    pintar(zLinea1, texto, COLOR_TEXTO);
    snprintf(texto, sizeof(texto), "filtro de %u muestras", resultado.ventana);
    pintar(zLinea2, texto, COLOR_TEXTO);
    if (noSeAsento) snprintf(texto, sizeof(texto), "no se asentó en %lu s", (unsigned long)(AFINAR_ASENTAR_MAX_MS / 1000));
    else            snprintf(texto, sizeof(texto), "se asienta en %.1f s", resultado.asentamientoMs / 1000.0f);
    pintar(zDato, texto, noSeAsento ? COLOR_AMBAR : COLOR_TENUE);
    pintar(zPie, "OK: pesar", COLOR_TENUE);
    anillo(1.0f, COLOR_VERDE);
    return;
  }

  pintar(zTitulo, "AFINAR", COLOR_AZUL);
  switch (paso) {
    case P_VACIO:
      pintar(zLinea1, "Retira todo del plato", COLOR_TEXTO);
      pintar(zLinea2, "y no toques la mesa", COLOR_TEXTO);
      pintar(zPie, "OK: escuchar", COLOR_TENUE);
      break;
    case P_PONER:
      pintar(zLinea1, "Coloca cualquier objeto", COLOR_TEXTO);
      if (ajustes::actual().cal.valida) snprintf(texto, sizeof(texto), "de %.0f g o más", escalonCuentas() / fabsf(ajustes::actual().cal.factor));
      else                              strlcpy(texto, "que se note", sizeof(texto));
      pintar(zLinea2, texto, COLOR_TEXTO);
      pintar(zPie, "lo detecta sola", COLOR_TENUE);
      break;
    case P_QUITAR:
      pintar(zLinea1, "Retira el objeto", COLOR_TEXTO);
      pintar(zLinea2, "y no toques la mesa", COLOR_TEXTO);
      pintar(zPie, "lo detecta sola", COLOR_TENUE);
      break;
    default: break;   // escuchas y asentamientos se pintan en refrescar()
  }
}

void ir(Paso p) {
  paso = p;
  mostrarPaso();
}

void terminar() {
  float    sigma = max(sigma1, sigma2);
  uint32_t asent = max(asent1, asent2);
  resultado = ruido::proponer(sigma, cuentasPorDivision(), (uint16_t)min<uint32_t>(asent, 65535));

  Ajustes& a = ajustes::actual();
  a.ruido = resultado;
  ajustes::guardar();
  bascula::perfil(resultado);

  Serial.printf("[afinar] sigma vacío=%.1f  con objeto=%.1f cuentas  asentamiento %lu / %lu ms  -> ventana %u\n",
                sigma1, sigma2, (unsigned long)asent1, (unsigned long)asent2, resultado.ventana);
  ir(P_LISTO);
}

}  // namespace

void entrar() {
  paso = P_VACIO;
  sigma1 = sigma2 = media1 = media2 = 0;
  asent1 = asent2 = 0;
  noSeAsento = false;
  mostrarPaso();
}

void tecla(const EventoTecla& e) {
  if (e.tipo != PULSACION_CORTA) return;

  if (e.tecla == TECLA_MENU) {
    bascula::escuchar(0);
    if (paso == P_VACIO)      irA(V_MENU);
    else if (paso == P_LISTO) irA(V_PESAR);
    else                      ir(P_VACIO);     // cancela: nada se guardo
    return;
  }

  if (e.tecla == TECLA_OK) {
    if (paso == P_LISTO) { irA(V_PESAR); return; }
    if (paso != P_VACIO) return;
    if (!bascula::peso().hayCelda) { aviso("Sin celda: revisa el HX711", COLOR_NARANJA, 2500); return; }
    bascula::escuchar(AFINAR_ESCUCHA_MS);
    ir(P_ESCUCHA1);
  }
}

void refrescar() {
  if (paso == P_LISTO) return;
  if (!avisoVigente()) pintar(zAviso, textoPaso(), COLOR_TENUE);

  Lectura l = bascula::leer();
  char    texto[40];

  switch (paso) {
    case P_VACIO: case P_PONER: case P_QUITAR: {
      snprintf(texto, sizeof(texto), "lectura: %ld", (long)l.filtrado);
      pintar(zDato, texto, COLOR_TENUE);
      pintar(zEstado, l.estable ? "• ESTABLE" : "midiendo…", l.estable ? COLOR_VERDE : COLOR_TENUE);
      anillo(l.progresoEstable, l.estable ? COLOR_VERDE : COLOR_AMBAR);
      // El objeto se detecta solo: la lectura se aleja de la media de la escucha anterior.
      if (paso == P_PONER  && abs(l.filtrado - (int32_t)media1) > escalonCuentas()) { t0 = millis(); ir(P_ASENTAR1); }
      if (paso == P_QUITAR && abs(l.filtrado - (int32_t)media2) > escalonCuentas()) { t0 = millis(); ir(P_ASENTAR2); }
      break;
    }

    case P_ESCUCHA1: case P_ESCUCHA2: {
      Escucha e = bascula::escucha();
      snprintf(texto, sizeof(texto), "%.1f", AFINAR_ESCUCHA_MS * (1.0f - e.progreso) / 1000.0f);
      pintarNumero(zNumero, texto, COLOR_TEXTO);
      pintar(zUnidad, "escuchando…", COLOR_TENUE);
      textoRuido(e.sigma, texto, sizeof(texto));
      pintar(zEstado, texto, COLOR_TENUE);
      pintar(zPie, "no toques nada", COLOR_AMBAR);
      anillo(e.progreso, COLOR_AZUL);
      if (e.activa) break;
      if (e.muestras < 10) { aviso("Sin lecturas: revisa la celda", COLOR_NARANJA, 2500); ir(P_VACIO); break; }
      if (paso == P_ESCUCHA1) { sigma1 = e.sigma; media1 = e.media; ir(P_PONER); }
      else                    { sigma2 = e.sigma; media2 = e.media; ir(P_QUITAR); }
      break;
    }

    case P_ASENTAR1: case P_ASENTAR2: {
      uint32_t t = millis() - t0;
      snprintf(texto, sizeof(texto), "%.1f", t / 1000.0f);
      pintarNumero(zNumero, texto, COLOR_TEXTO);
      pintar(zUnidad, "asentándose…", COLOR_TENUE);
      pintar(zEstado, l.estable ? "• ESTABLE" : "midiendo…", l.estable ? COLOR_VERDE : COLOR_TENUE);
      pintar(zPie, "no toques nada", COLOR_AMBAR);
      anillo(l.progresoEstable, COLOR_AMBAR);
      bool tope = t >= AFINAR_ASENTAR_MAX_MS;
      if (!l.estable && !tope) break;
      if (tope) noSeAsento = true;
      if (paso == P_ASENTAR1) { asent1 = t; bascula::escuchar(AFINAR_ESCUCHA_MS); ir(P_ESCUCHA2); }
      else                    { asent2 = t; terminar(); }
      break;
    }

    default: break;
  }
}

}  // namespace vAfinar
}  // namespace ui

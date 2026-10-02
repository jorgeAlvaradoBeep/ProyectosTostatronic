/*
 * ============================================================
 *  VISTA: CONTAR PIEZAS
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Muestra nueva, en tres pasos:
 *
 *    Paso 1  Contenedor vacio en el plato ... OK lo tara
 *    Paso 2  Unas cuantas piezas ............ OK toma su peso
 *    Paso 3  Cuantas piezas son ............. ARRIBA / ABAJO, OK
 *
 *  Con eso se calcula el peso de una pieza y la pantalla pasa a
 *  CONTAR: el numero grande son las piezas, abajo va el peso.
 *
 *    OK corta ........ tara (para cambiar de contenedor)
 *    ARRIBA / ABAJO .. opciones: guardar la pieza, otra muestra,
 *                      elegir una pieza guardada, terminar
 *    MENU corta ...... regresa un paso (o al menu)
 *
 *  Si ya hay piezas guardadas, al entrar se elige una de la lista
 *  y se cuenta de inmediato, sin tomar muestra.
 *
 *  Las capturas solo se hacen con el peso ESTABLE. Los avisos de
 *  "muestra chica", "pieza ligera" y "entre 12 y 13" salen de
 *  contador.h, donde se explica cada uno.
 * ============================================================
 */

#include "ui_comun.h"
#include "contador.h"

namespace ui {
namespace vContar {

namespace {

enum Pantalla : uint8_t { P_ELEGIR, P_TARA, P_MUESTRA, P_CANTIDAD, P_CONTANDO, P_OPCIONES };
Pantalla pantallaActual = P_TARA;

// Que pantalla ya esta dibujada, para borrar solo cuando cambia.
int16_t       dibujada = -1;
const int16_t CLAVE_BLOQUEO = 1000;

bool     esperando = false;       // OK presionado, falta que se asiente
uint32_t esperaHastaMs = 0;
float    muestraG = 0;            // peso tomado en el paso 2
uint32_t cantidad = 10;           // piezas de la muestra; se recuerda la ultima
uint16_t repeticiones = 0;        // para acelerar ARRIBA / ABAJO sostenidas
uint8_t  seleccion = 0;

// Aviso que debe salir DESPUES de cambiar de pantalla (el cambio borra todo).
char     nota[40] = "";
uint16_t notaColor = COLOR_AZUL;

bool cambio(int16_t clave) {
  if (clave == dibujada) return false;
  dibujada = clave;
  limpiarVista();
  return true;
}

void mostrar(Pantalla p) {
  pantallaActual = p;
  dibujada  = -1;
  esperando = false;
}

void anotar(const char* texto, uint16_t color) {
  strlcpy(nota, texto, sizeof(nota));
  notaColor = color;
}

void sacarNota() {
  if (!nota[0]) return;
  aviso(nota, notaColor, 4000);
  nota[0] = 0;
}

bool bloqueada(const Peso& p) { return !p.hayCelda || !p.calibrada; }

void pesoConUnidad(float gramos, const Peso& p, char* texto, size_t tam) {
  char numero[16];
  Unidad u = ajustes::actual().unidad;
  formatearPeso(gramos, u, p.divisionG, numero, sizeof(numero));
  snprintf(texto, tam, "%s %s", numero, nombreUnidad(u));
}

// ------------------ sin celda / sin calibrar ------------------

void refrescarBloqueo(const Peso& p) {
  if (cambio(CLAVE_BLOQUEO + p.hayCelda)) {
    pintar(zTitulo, "CONTAR", COLOR_AZUL);
    pintar(zLinea1, p.hayCelda ? "Sin calibrar" : "Sin celda", COLOR_NARANJA);
    pintar(zDato, p.hayCelda ? "OK: calibrar" : "revisa el HX711", COLOR_TENUE);
    anillo(1.0f, p.hayCelda ? COLOR_AMBAR : COLOR_NARANJA);
  }
}

// ------------------ elegir una pieza guardada ------------------

const char* nombres[PIEZAS_MAX_PERFILES + 1];

uint8_t armarLista() {
  uint8_t n = 0;
  nombres[n++] = "Nueva muestra";
  for (uint8_t i = 0; i < contador::perfiles(); i++) nombres[n++] = contador::perfil(i).nombre;
  if (seleccion >= n) seleccion = 0;
  return n;
}

void refrescarElegir() {
  uint8_t n = armarLista();
  if (cambio(P_ELEGIR * 32 + n)) {
    pintar(zTitulo, "CONTAR", COLOR_AZUL);
    pintar(zPie, "OK: elegir", COLOR_TENUE);
  }
  lista(nombres, n, seleccion);
}

// ------------------ pasos 1 y 2: esperar el peso estable ------------------

void capturar(const Peso& p) {
  esperando = false;
  if (pantallaActual == P_TARA) {
    bascula::tarar();
    mostrar(P_MUESTRA);
    return;
  }
  // P_MUESTRA
  if (p.netoG < p.divisionG) {
    aviso("Pon primero las piezas", COLOR_NARANJA, 2500);
    return;
  }
  muestraG = p.netoG;
  repeticiones = 0;
  mostrar(P_CANTIDAD);
}

void refrescarPaso(const Peso& p) {
  bool tara = pantallaActual == P_TARA;
  char texto[40], peso[24];

  if (cambio(pantallaActual * 32)) {
    pintar(zTitulo, "CONTAR", COLOR_AZUL);
    pintar(zLinea1, tara ? "Pon el contenedor" : "Coloca unas", COLOR_TEXTO);
    pintar(zLinea2, tara ? "vacío en el plato" : "cuantas piezas", COLOR_TEXTO);
    pintar(zPie, tara ? "OK: tara" : "OK: continuar", COLOR_TENUE);
  }
  if (!avisoVigente()) pintar(zAviso, tara ? "Paso 1 de 3" : "Paso 2 de 3", COLOR_TENUE);

  pesoConUnidad(p.netoG, p, peso, sizeof(peso));
  snprintf(texto, sizeof(texto), "%s: %s", tara ? "ahora" : "muestra", peso);
  pintar(zDato, texto, COLOR_TENUE);

  anillo(p.progresoEstable, p.estable ? COLOR_VERDE : COLOR_AMBAR);
  if (esperando)       pintar(zEstado, "esperando a que se asiente…", COLOR_AMBAR);
  else if (p.estable)  pintar(zEstado, "• ESTABLE", COLOR_VERDE);
  else                 pintar(zEstado, "asentándose…", COLOR_TENUE);

  if (esperando) {
    if (p.estable) capturar(p);
    else if ((int32_t)(millis() - esperaHastaMs) >= 0) {
      esperando = false;
      aviso("No se asentó: intenta de nuevo", COLOR_NARANJA, 2500);
    }
  }
}

// ------------------ paso 3: cuantas piezas son ------------------

uint32_t incremento() {
  if (repeticiones < 10) return 1;
  if (repeticiones < 25) return 10;
  return 100;
}

void tomarMuestra(const Peso& p) {
  Muestra m = contador::tomarMuestra(muestraG, cantidad, p.divisionG);
  char texto[40];

  switch (m.calidad) {
    case MUESTRA_INVALIDA:
      aviso("No se pudo tomar la muestra", COLOR_NARANJA, 2500);
      return;
    case MUESTRA_CHICA:
      snprintf(texto, sizeof(texto), "Mejor con %lu piezas o más", (unsigned long)m.sugeridas);
      anotar(texto, COLOR_AMBAR);
      break;
    case MUESTRA_NO_CONFIABLE:
      anotar("Una pieza pesa muy poco", COLOR_NARANJA);
      break;
    default:
      anotar("Muestra tomada", COLOR_VERDE);
      break;
  }
  mostrar(P_CONTANDO);
}

void refrescarCantidad(const Peso& p) {
  char texto[40], peso[24];

  if (cambio(P_CANTIDAD * 32)) {
    pintar(zTitulo, "CONTAR", COLOR_AZUL);
    pintar(zUnidad, "¿cuántas son?", COLOR_TENUE);
    pintar(zEstado, "ARRIBA / ABAJO: ajustar", COLOR_TENUE);
    pintar(zPie, "OK: contar", COLOR_TENUE);
  }
  if (!avisoVigente()) {
    pesoConUnidad(muestraG, p, peso, sizeof(peso));
    snprintf(texto, sizeof(texto), "El peso es de %s", peso);
    pintar(zAviso, texto, COLOR_TENUE);
  }
  snprintf(texto, sizeof(texto), "%lu", (unsigned long)cantidad);
  pintarNumero(zNumero, texto, COLOR_TEXTO);

  // Al soltar ARRIBA / ABAJO la velocidad vuelve a uno en uno.
  if (!teclado::presionada(TECLA_ARRIBA) && !teclado::presionada(TECLA_ABAJO)) repeticiones = 0;
}

// ------------------ contando ------------------

void refrescarContando(const Peso& p) {
  Conteo c = contador::contar(p);
  char   texto[48], peso[24];

  if (!c.activo) {   // lo quitaron desde la pagina web
    mostrar(contador::perfiles() ? P_ELEGIR : P_TARA);
    return;
  }

  if (cambio(P_CONTANDO * 32)) {
    pintar(zTitulo, "PIEZAS", COLOR_AZUL);
    pintar(zPie, "ARRIBA: opciones", COLOR_TENUE);
    sacarNota();
  }

  if (!avisoVigente()) {
    if (c.perfil >= 0) {
      pintarAjustado(zAviso, c.nombre, F_CHICA, F_CHICA, COLOR_TENUE);
    } else {
      snprintf(texto, sizeof(texto), c.unitarioG < 10 ? "%.3f g por pieza" : "%.2f g por pieza", c.unitarioG);
      pintar(zAviso, texto, COLOR_TENUE);
    }
  }

  if (p.sobrecarga) {
    pintarNumero(zNumero, "SOBRECARGA", COLOR_NARANJA);
    anillo(1.0f, COLOR_NARANJA);
  } else {
    snprintf(texto, sizeof(texto), "%ld", (long)c.piezas);
    pintarNumero(zNumero, texto, c.confiable ? COLOR_TEXTO : COLOR_AMBAR);
    anillo(p.brutoG / p.capacidadG, COLOR_AZUL);
  }

  pesoConUnidad(p.netoG, p, peso, sizeof(peso));
  pintar(zUnidad, peso, COLOR_TENUE);

  if (!c.confiable)                   pintar(zEstado, "pieza ligera: no confiable", COLOR_NARANJA);
  else if (p.estable && c.ambiguo) {
    long abajo = (long)floorf(c.exacto);
    snprintf(texto, sizeof(texto), "entre %ld y %ld piezas", abajo, abajo + 1);
    pintar(zEstado, texto, COLOR_AMBAR);
  }
  else if (p.estable)                 pintar(zEstado, "• ESTABLE", COLOR_VERDE);
  else                                pintar(zEstado, "contando…", COLOR_TENUE);
}

// ------------------ opciones ------------------

enum Opcion : uint8_t { O_GUARDAR, O_MUESTRA, O_ELEGIR, O_TERMINAR };

Opcion      opciones[4];
const char* textosOpciones[4];

uint8_t armarOpciones() {
  Conteo  c = contador::contar(bascula::peso());
  uint8_t n = 0;
  if (c.activo && c.perfil < 0 && contador::perfiles() < PIEZAS_MAX_PERFILES) {
    opciones[n] = O_GUARDAR;  textosOpciones[n++] = "Guardar pieza";
  }
  opciones[n] = O_MUESTRA;    textosOpciones[n++] = "Otra muestra";
  if (contador::perfiles())   { opciones[n] = O_ELEGIR; textosOpciones[n++] = "Elegir pieza"; }
  opciones[n] = O_TERMINAR;   textosOpciones[n++] = "Terminar";
  if (seleccion >= n) seleccion = 0;
  return n;
}

void elegirOpcion() {
  char texto[40];
  armarOpciones();
  switch (opciones[seleccion]) {
    case O_GUARDAR: {
      int8_t i = contador::guardar();
      if (i >= 0) { snprintf(texto, sizeof(texto), "Guardada: %s", contador::perfil(i).nombre); anotar(texto, COLOR_VERDE); }
      seleccion = 0;
      mostrar(P_CONTANDO);
      break;
    }
    case O_MUESTRA:  seleccion = 0; mostrar(P_TARA);   break;
    case O_ELEGIR:   seleccion = 0; mostrar(P_ELEGIR); break;
    case O_TERMINAR: contador::quitar(); irA(V_PESAR); break;
  }
}

void refrescarOpciones() {
  uint8_t n = armarOpciones();
  if (cambio(P_OPCIONES * 32 + n)) {
    pintar(zTitulo, "PIEZAS", COLOR_AZUL);
    pintar(zPie, "OK: elegir", COLOR_TENUE);
  }
  lista(textosOpciones, n, seleccion);
}

}  // namespace

void entrar() {
  seleccion = 0;
  nota[0]   = 0;
  Conteo c = contador::contar(bascula::peso());
  mostrar(c.activo ? P_CONTANDO : contador::perfiles() ? P_ELEGIR : P_TARA);
  refrescar();
}

void tecla(const EventoTecla& e) {
  Peso p = bascula::peso();
  bool corta = e.tipo == PULSACION_CORTA;

  if (bloqueada(p)) {
    if (!corta) return;
    if (e.tecla == TECLA_OK && p.hayCelda) irA(V_CALIBRAR);
    if (e.tecla == TECLA_MENU)             irA(V_MENU);
    return;
  }

  switch (pantallaActual) {
    case P_ELEGIR: {
      uint8_t n = armarLista();
      if (e.tecla == TECLA_ARRIBA) seleccion = (seleccion + n - 1) % n;
      if (e.tecla == TECLA_ABAJO)  seleccion = (seleccion + 1) % n;
      if (!corta) return;
      if (e.tecla == TECLA_MENU) irA(V_MENU);
      if (e.tecla == TECLA_OK) {
        if (seleccion == 0) mostrar(P_TARA);
        else { contador::usar(seleccion - 1); mostrar(P_CONTANDO); }
        seleccion = 0;
      }
      break;
    }

    case P_TARA:
    case P_MUESTRA:
      if (!corta) return;
      if (e.tecla == TECLA_MENU) {
        if (pantallaActual == P_MUESTRA)   mostrar(P_TARA);
        else if (contador::perfiles())     mostrar(P_ELEGIR);
        else                               irA(V_MENU);
      }
      if (e.tecla == TECLA_OK) {
        if (p.estable) capturar(p);
        else { esperando = true; esperaHastaMs = millis() + ESPERA_ESTABLE_MS; }
      }
      break;

    case P_CANTIDAD:
      if (e.tecla == TECLA_ARRIBA || e.tecla == TECLA_ABAJO) {
        if (!corta) repeticiones++;
        uint32_t delta = incremento();
        if (e.tecla == TECLA_ARRIBA) cantidad = min(cantidad + delta, (uint32_t)9999);
        else                         cantidad = cantidad > delta ? cantidad - delta : 1;
      }
      if (!corta) return;
      if (e.tecla == TECLA_MENU) mostrar(P_MUESTRA);
      if (e.tecla == TECLA_OK)   tomarMuestra(p);
      break;

    case P_CONTANDO:
      if (!corta) return;
      if (e.tecla == TECLA_MENU) irA(V_MENU);
      if (e.tecla == TECLA_ARRIBA || e.tecla == TECLA_ABAJO) { seleccion = 0; mostrar(P_OPCIONES); }
      if (e.tecla == TECLA_OK) {
        if (!p.estable) { aviso("Espera a que se asiente", COLOR_AMBAR); break; }
        bascula::tarar();
        aviso(bascula::peso().conTara ? "Tara tomada" : "Tara quitada", COLOR_VERDE);
      }
      break;

    case P_OPCIONES: {
      uint8_t n = armarOpciones();
      if (e.tecla == TECLA_ARRIBA) seleccion = (seleccion + n - 1) % n;
      if (e.tecla == TECLA_ABAJO)  seleccion = (seleccion + 1) % n;
      if (!corta) return;
      if (e.tecla == TECLA_MENU) { seleccion = 0; mostrar(P_CONTANDO); }
      if (e.tecla == TECLA_OK)   elegirOpcion();
      break;
    }
  }
}

void refrescar() {
  Peso p = bascula::peso();
  if (bloqueada(p)) { refrescarBloqueo(p); return; }

  switch (pantallaActual) {
    case P_ELEGIR:   refrescarElegir();      break;
    case P_TARA:
    case P_MUESTRA:  refrescarPaso(p);       break;
    case P_CANTIDAD: refrescarCantidad(p);   break;
    case P_CONTANDO: refrescarContando(p);   break;
    case P_OPCIONES: refrescarOpciones();    break;
  }
}

}  // namespace vContar
}  // namespace ui

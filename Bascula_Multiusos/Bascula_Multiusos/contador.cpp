/*
 * ============================================================
 *  CONTADOR DE PIEZAS - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "contador.h"
#include "config.h"
#include <Preferences.h>

namespace {

const char* ESPACIO = "piezas";    // "carpeta" de la NVS para los perfiles

PerfilPieza lista[PIEZAS_MAX_PERFILES];
uint8_t     cuantos = 0;

float  unitarioG = 0;      // peso por pieza en uso; 0 = no se esta contando
int8_t enUso     = -1;     // perfil del que salio, o -1

void guardarLista() {
  Preferences nvs;
  nvs.begin(ESPACIO, false);
  nvs.putUChar("n", cuantos);
  nvs.putBytes("lista", lista, sizeof(PerfilPieza) * cuantos);
  nvs.end();
}

/* Copia un nombre quitandole espacios de las orillas y cortandolo a lo
 * que cabe, sin partir un caracter con acento (varios bytes en UTF-8). */
void copiarNombre(char* destino, const char* origen) {
  while (*origen == ' ') origen++;
  size_t n = strlen(origen);
  if (n > PIEZAS_LARGO_NOMBRE) {
    n = PIEZAS_LARGO_NOMBRE;
    while (n > 0 && ((uint8_t)origen[n] & 0xC0) == 0x80) n--;
  }
  while (n > 0 && origen[n - 1] == ' ') n--;
  memcpy(destino, origen, n);
  destino[n] = 0;
  for (char* c = destino; *c; c++) if ((uint8_t)*c < 0x20) *c = ' ';
}

bool nombreUsado(const char* nombre) {
  for (uint8_t i = 0; i < cuantos; i++) if (strcmp(lista[i].nombre, nombre) == 0) return true;
  return false;
}

}  // namespace

namespace contador {

void cargar() {
  Preferences nvs;
  nvs.begin(ESPACIO, false);   // false: la crea si es el primer arranque
  cuantos = nvs.getUChar("n", 0);
  if (cuantos > PIEZAS_MAX_PERFILES) cuantos = 0;
  size_t esperado = sizeof(PerfilPieza) * cuantos;
  if (cuantos && nvs.getBytes("lista", lista, esperado) != esperado) cuantos = 0;
  nvs.end();

  // Nada de lo leido se usa a ciegas.
  for (uint8_t i = 0; i < cuantos; i++) {
    lista[i].nombre[PIEZAS_LARGO_NOMBRE] = 0;
    if (!(lista[i].unitarioG > 0)) { cuantos = i; break; }
  }
}

Muestra tomarMuestra(float netoG, uint32_t piezas, float divisionG) {
  Muestra m;
  if (piezas == 0 || !(netoG > 0)) return m;

  m.unitarioG = netoG / piezas;
  m.sugeridas = (uint32_t)ceilf(CONTAR_MUESTRA_MIN_DIV * divisionG / m.unitarioG);

  if (m.unitarioG < divisionG)                            m.calidad = MUESTRA_NO_CONFIABLE;
  else if (netoG < CONTAR_MUESTRA_MIN_DIV * divisionG)    m.calidad = MUESTRA_CHICA;
  else                                                    m.calidad = MUESTRA_BIEN;

  unitarioG = m.unitarioG;
  enUso     = -1;
  return m;
}

void quitar() {
  unitarioG = 0;
  enUso     = -1;
}

Conteo contar(const Peso& p) {
  Conteo c;
  if (!(unitarioG > 0)) return c;

  c.activo    = true;
  c.unitarioG = unitarioG;
  c.perfil    = enUso;
  c.confiable = unitarioG >= p.divisionG;
  if (enUso >= 0) strlcpy(c.nombre, lista[enUso].nombre, sizeof(c.nombre));
  if (!p.calibrada) return c;

  c.exacto = p.netoG / unitarioG;
  c.piezas = (int32_t)lroundf(c.exacto);
  // Que tan lejos quedo del entero: 0 = justo en el, 0.5 = a la mitad.
  float lejos = fabsf(c.exacto - c.piezas);
  c.ambiguo = lejos > 0.5f - CONTAR_AMBIGUO;
  return c;
}

uint8_t perfiles() { return cuantos; }

const PerfilPieza& perfil(uint8_t i) { return lista[i < cuantos ? i : 0]; }

bool usar(uint8_t i) {
  if (i >= cuantos) return false;
  unitarioG = lista[i].unitarioG;
  enUso     = i;
  return true;
}

int8_t guardar(const char* nombre) {
  if (enUso >= 0) return enUso;      // ya es un perfil guardado
  if (!(unitarioG > 0) || cuantos >= PIEZAS_MAX_PERFILES) return -1;

  PerfilPieza& nuevo = lista[cuantos];
  nuevo.unitarioG = unitarioG;
  nuevo.nombre[0] = 0;
  if (nombre) copiarNombre(nuevo.nombre, nombre);
  if (!nuevo.nombre[0]) {
    // "Pieza N", con el primer numero que no este usado.
    for (uint8_t n = 1; n <= PIEZAS_MAX_PERFILES + 1; n++) {
      snprintf(nuevo.nombre, sizeof(nuevo.nombre), "Pieza %u", n);
      if (!nombreUsado(nuevo.nombre)) break;
    }
  }
  enUso = cuantos++;
  guardarLista();
  return enUso;
}

bool renombrar(uint8_t i, const char* nombre) {
  if (i >= cuantos) return false;
  char limpio[PIEZAS_LARGO_NOMBRE + 1];
  copiarNombre(limpio, nombre);
  if (!limpio[0]) return false;
  strlcpy(lista[i].nombre, limpio, sizeof(lista[i].nombre));
  guardarLista();
  return true;
}

bool borrar(uint8_t i) {
  if (i >= cuantos) return false;
  for (uint8_t k = i; k + 1 < cuantos; k++) lista[k] = lista[k + 1];
  cuantos--;
  // El peso por pieza en uso se conserva; solo deja de tener nombre.
  if (enUso == i)     enUso = -1;
  else if (enUso > i) enUso--;
  guardarLista();
  return true;
}

}  // namespace contador

/*
 * ============================================================
 *  CONTADOR DE PIEZAS POR PESO
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Contar por peso es una division:
 *
 *      peso de una pieza = peso de la muestra / piezas de la muestra
 *      piezas            = peso en el plato   / peso de una pieza
 *
 *  Lo delicado es saber cuando NO confiar en el resultado:
 *
 *    - Si una pieza pesa menos que la division de la bascula
 *      (0.1 g con la celda de 1 kg), la bascula no alcanza a
 *      distinguir una pieza de mas o de menos: NO CONFIABLE.
 *    - Si la muestra fue muy ligera, el peso por pieza salio con
 *      mucho error y el conteo se va desviando al crecer: se
 *      avisa y se sugiere cuantas piezas poner.
 *    - Si el conteo cae cerca de la mitad entre dos enteros
 *      (12.4 piezas), puede ser 12 o 13: AMBIGUO.
 *
 *  Los pesos por pieza se pueden guardar como perfiles con nombre
 *  ("Tornillo M3x10") para no volver a tomar la muestra. Viven en
 *  la NVS. Desde la bascula se guardan como "Pieza 1", "Pieza 2"...
 *  y el nombre se cambia desde la pagina web (con cuatro teclas no
 *  se puede escribir).
 *
 *  Este modulo no sabe de pantalla ni de web: los dos lo usan.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "bascula.h"

const uint8_t PIEZAS_MAX_PERFILES = 12;
const uint8_t PIEZAS_LARGO_NOMBRE = 24;    // bytes, sin contar el fin de cadena

struct PerfilPieza {
  char  nombre[PIEZAS_LARGO_NOMBRE + 1];
  float unitarioG;
};

struct Conteo {
  bool    activo    = false;   // hay un peso por pieza contra el cual contar
  float   unitarioG = 0;
  int32_t piezas    = 0;       // redondeado; negativo si se sacan piezas tras tarar
  float   exacto    = 0;       // sin redondear
  bool    ambiguo   = false;   // cerca de la mitad entre dos enteros
  bool    confiable = true;    // false: una pieza pesa menos que la division
  int8_t  perfil    = -1;      // perfil en uso; -1 = muestra sin guardar
  char    nombre[PIEZAS_LARGO_NOMBRE + 1] = "";
};

enum CalidadMuestra : uint8_t {
  MUESTRA_BIEN,
  MUESTRA_CHICA,          // sirve, pero conviene una mas grande (ver 'sugeridas')
  MUESTRA_NO_CONFIABLE,   // una pieza pesa menos que la division
  MUESTRA_INVALIDA,       // sin piezas o sin peso: no se toma
};

struct Muestra {
  CalidadMuestra calidad   = MUESTRA_INVALIDA;
  float          unitarioG = 0;
  uint32_t       sugeridas = 0;   // piezas que conviene poner en la muestra
};

namespace contador {

void cargar();     // los perfiles guardados, al arrancar

// Fija el peso por pieza a partir de lo que hay en el plato. Si la
// muestra no es invalida, queda en uso (aunque se avise que es chica).
Muestra tomarMuestra(float netoG, uint32_t piezas, float divisionG);

void quitar();     // deja de contar

Conteo contar(const Peso& p);

// ---- perfiles ----
uint8_t            perfiles();
const PerfilPieza& perfil(uint8_t i);
bool               usar(uint8_t i);

// Guarda el peso por pieza en uso. Sin nombre se llama "Pieza N".
// Devuelve el indice, o -1 si no hay nada en uso o ya no caben.
int8_t guardar(const char* nombre = nullptr);
bool   renombrar(uint8_t i, const char* nombre);
bool   borrar(uint8_t i);

}  // namespace contador

/*
 * ============================================================
 *  VISTA: ACTUALIZACION DESDE EL SERVIDOR
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Se abre de dos maneras:
 *
 *    - Sola, al arrancar: si el servidor tiene una version mas
 *      nueva para esta placa, en cuanto la bascula esta pesando (o
 *      en el menu) aparece la oferta. MENU la pospone hasta el
 *      siguiente arranque; OK la instala.
 *    - Desde MENU > Conexion > "Buscar version" / "Instalar vX".
 *
 *  Una pantalla por estado del actualizador: buscando, al dia,
 *  disponible, descargando (porcentaje y anillo), lista (reinicia
 *  sola) y error (OK reintenta). Mientras descarga, MENU no hace
 *  nada: lo que se interrumpe a medias se descarta de todos modos,
 *  pero mejor que el usuario vea terminar lo que empezo.
 * ============================================================
 */

#include "ui_comun.h"
#include "actualizador.h"

namespace ui {
namespace vActualizar {

namespace {

Vista   origen   = V_PESAR;
int16_t dibujada = -1;

const uint8_t FORMA_SIN_LUGAR = 31;

// true si hubo que borrar: toca pintar tambien lo fijo.
bool cambio(uint8_t forma) {
  if (forma == dibujada) return false;
  dibujada = forma;
  limpiarVista();
  return true;
}

void tamanoEnTexto(uint32_t bytes, char* texto, size_t tam) {
  if (bytes >= 1000000) snprintf(texto, tam, "%.1f MB", bytes / 1000000.0f);
  else                  snprintf(texto, tam, "%lu KB", (unsigned long)(bytes / 1000));
}

}  // namespace

void regresarA(Vista v) { origen = v; }

void entrar() {
  dibujada = -1;
  refrescar();
}

void tecla(const EventoTecla& e) {
  if (e.tipo != PULSACION_CORTA) return;
  InfoActualizador a = actualizador::info();

  if (e.tecla == TECLA_OK) {
    switch (a.estado) {
      case ACT_DISPONIBLE: actualizador::instalar(); break;
      case ACT_ERROR:      actualizador::instalar(); break;   // reintenta la descarga, o la consulta si no habia version
      case ACT_AL_DIA:
      case ACT_INACTIVA:   actualizador::buscar();   break;
      default: break;
    }
  }

  if (e.tecla == TECLA_MENU) {
    if (a.estado == ACT_DESCARGANDO || a.estado == ACT_LISTA) return;   // que termine
    if (a.estado == ACT_DISPONIBLE) actualizador::descartar();
    irA(origen);
  }
}

void refrescar() {
  InfoActualizador a = actualizador::info();
  char texto[48];

  if (cambio(a.hayLugar ? a.estado : FORMA_SIN_LUGAR)) {
    pintar(zTitulo, "VERSIÓN", COLOR_AZUL);
    pintar(zAviso, "instalada: v" PROYECTO_VERSION, COLOR_TENUE);
  }

  if (!a.hayLugar) {
    pintar(zLinea1, "Esta placa se cargó", COLOR_TEXTO);
    pintar(zLinea2, "sin lugar para OTA", COLOR_TEXTO);
    pintar(zDato, "Partition: Minimal SPIFFS", COLOR_AMBAR);
    pintar(zPie, "MENÚ: regresar", COLOR_TENUE);
    anillo(1.0f, COLOR_AMBAR);
    return;
  }

  switch (a.estado) {
    case ACT_BUSCANDO:
      pintar(zLinea1, "Buscando versión", COLOR_TEXTO);
      pintar(zLinea2, "nueva en el servidor…", COLOR_TENUE);
      pintar(zPie, "MENÚ: regresar", COLOR_TENUE);
      anillo((millis() % 2000) / 2000.0f, COLOR_AZUL);   // el anillo da vueltas: algo esta pasando
      break;

    case ACT_DISPONIBLE:
      snprintf(texto, sizeof(texto), "Nueva versión v%s", a.version);
      pintar(zLinea1, texto, COLOR_VERDE);
      if (a.notas[0]) pintarAjustado(zLinea2, a.notas, F_TEXTO, F_CHICA, COLOR_TEXTO);
      else            pintar(zLinea2, "lista para instalar", COLOR_TEXTO);
      tamanoEnTexto(a.tamano, texto, sizeof(texto));
      strlcat(texto, " desde el servidor", sizeof(texto));
      pintar(zDato, texto, COLOR_TENUE);
      pintar(zEstado, "MENÚ: ahora no", COLOR_TENUE);
      pintar(zPie, "OK: instalar", COLOR_VERDE);
      anillo(1.0f, COLOR_VERDE);
      break;

    case ACT_DESCARGANDO:
      snprintf(texto, sizeof(texto), "%u%%", a.porciento);
      pintarNumero(zNumero, texto, COLOR_TEXTO);
      snprintf(texto, sizeof(texto), "bajando v%s…", a.version);
      pintar(zUnidad, texto, COLOR_TENUE);
      pintar(zEstado, "no apagues la báscula", COLOR_AMBAR);
      anillo(a.porciento / 100.0f, COLOR_VERDE);
      break;

    case ACT_LISTA:
      snprintf(texto, sizeof(texto), "v%s instalada", a.version);
      pintar(zLinea1, texto, COLOR_VERDE);
      pintar(zLinea2, "Reiniciando…", COLOR_TEXTO);
      anillo(1.0f, COLOR_VERDE);
      break;

    case ACT_ERROR:
      pintar(zLinea1, "No se actualizó", COLOR_NARANJA);
      pintarAjustado(zLinea2, a.error, F_TEXTO, F_CHICA, COLOR_TEXTO);
      pintar(zDato, "nada cambió", COLOR_TENUE);
      pintar(zEstado, "MENÚ: salir", COLOR_TENUE);
      pintar(zPie, "OK: reintentar", COLOR_TENUE);
      anillo(1.0f, COLOR_NARANJA);
      break;

    case ACT_AL_DIA:
      pintar(zLinea1, "Ya tienes la versión", COLOR_TEXTO);
      pintar(zLinea2, "más reciente", COLOR_TEXTO);
      snprintf(texto, sizeof(texto), "servidor: v%s", a.version);
      pintar(zDato, texto, COLOR_TENUE);
      pintar(zEstado, "OK: buscar otra vez", COLOR_TENUE);
      pintar(zPie, "MENÚ: regresar", COLOR_TENUE);
      anillo(1.0f, COLOR_VERDE);
      break;

    default:   // ACT_INACTIVA: todavia no hay WiFi
      pintar(zLinea1, "Sin conexión WiFi", COLOR_TEXTO);
      pintarAjustado(zLinea2, "no se consultó el servidor", F_TEXTO, F_CHICA, COLOR_TENUE);
      pintar(zEstado, "OK: buscar", COLOR_TENUE);
      pintar(zPie, "MENÚ: regresar", COLOR_TENUE);
      anillo(0, COLOR_AZUL);
      break;
  }
}

}  // namespace vActualizar
}  // namespace ui

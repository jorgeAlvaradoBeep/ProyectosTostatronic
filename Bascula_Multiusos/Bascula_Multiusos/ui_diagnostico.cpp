/*
 * ============================================================
 *  VISTA: DIAGNOSTICO (prueba de hardware y de WiFi)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Lo que en la fase 1 era la pantalla PRUEBA: cuentas crudas del
 *  HX711, muestras por segundo, ruido y las cuatro teclas.
 *
 *    OK corta ...... toma un cero local (el numero pasa a relativo)
 *    OK larga ...... quita ese cero
 *    ARRIBA ........ enciende / apaga la PRUEBA DE WIFI
 *    ABAJO ......... contorno de las zonas de texto (revisar diseno)
 *    MENU corta .... regresa al menu
 *
 *  La prueba de WiFi sigue activa al salir de aqui: asi se puede
 *  pesar y calibrar con el radio transmitiendo.
 * ============================================================
 */

#include "ui_comun.h"
#include "prueba_wifi.h"

namespace ui {
namespace vDiagnostico {

namespace {

const int16_t TECLAS_Y       = 212;
const int16_t TECLAS_X[4]    = { 87, 109, 131, 153 };
const int16_t TECLAS_RADIO   = 6;
int8_t        teclasPintadas[4] = { -1, -1, -1, -1 };

// Una celda de su capacidad nominal, con ganancia 128, anda por los 2
// millones de cuentas: con eso el anillo se llena al cargarla completa.
const float CUENTAS_ANILLO_LLENO = 2097152.0f;

int32_t ceroLocal = 0;

}  // namespace

void entrar() {
  limpiarVista();
  for (int8_t& t : teclasPintadas) t = -1;
  pintar(zTitulo, "PRUEBA", COLOR_AZUL);
  aviso(NOMBRE_PLACA, COLOR_TENUE, 2000);
}

void tecla(const EventoTecla& e) {
  if (e.tipo != PULSACION_CORTA && e.tipo != PULSACION_LARGA) return;

  switch (e.tecla) {
    case TECLA_OK:
      if (e.tipo == PULSACION_CORTA) { ceroLocal = bascula::leer().filtrado; aviso("Cero local tomado", COLOR_VERDE); }
      else                           { ceroLocal = 0; aviso("Sin cero local", COLOR_AMBAR); }
      break;

    case TECLA_ARRIBA:
      if (e.tipo != PULSACION_CORTA) return;
      if (pruebaWifi::activa()) { pruebaWifi::detener(); aviso("WiFi de prueba apagado", COLOR_AMBAR); }
      else                      { pruebaWifi::iniciar(); aviso("WiFi de prueba encendido", COLOR_AZUL); }
      break;

    case TECLA_ABAJO:
      if (e.tipo != PULSACION_CORTA) return;
      pantalla::depuracion(!pantalla::depuracionActiva());
      entrar();
      break;

    case TECLA_MENU:
      if (e.tipo == PULSACION_CORTA) irA(V_MENU);
      break;

    default: break;
  }
}

void refrescar() {
  char texto[40];
  Lectura l = bascula::leer();

  if (!avisoVigente()) {
    if (pruebaWifi::activa()) {
      snprintf(texto, sizeof(texto), "WiFi: %lu paquetes/s", (unsigned long)pruebaWifi::paquetesPorSegundo());
      pintar(zAviso, texto, COLOR_AZUL);
    } else {
      pintar(zAviso, "ARRIBA: prueba de WiFi", COLOR_TENUE);
    }
  }

  if (l.estado == HX_SIN_RESPUESTA || l.doutAlAire) {
    pintarNumero(zNumero, "SIN CELDA", COLOR_NARANJA);
    pintar(zUnidad, l.doutAlAire ? "revisa el cable DOUT" : "revisa el HX711", COLOR_TENUE);
    anillo(1.0f, COLOR_NARANJA);
  } else if (l.estado == HX_SATURADO) {
    pintarNumero(zNumero, "SATURADO", COLOR_NARANJA);
    pintar(zUnidad, "revisa E+ E- A+ A-", COLOR_TENUE);
    anillo(1.0f, COLOR_NARANJA);
  } else {
    int32_t relativo = l.filtrado - ceroLocal;
    snprintf(texto, sizeof(texto), "%ld", (long)relativo);
    pintarNumero(zNumero, texto, COLOR_TEXTO);
    pintar(zUnidad, ceroLocal ? "desde el cero" : "cuentas crudas", COLOR_TENUE);
    anillo(fabsf((float)relativo) / CUENTAS_ANILLO_LLENO, COLOR_AZUL);
  }

  snprintf(texto, sizeof(texto), "%.1f SPS · ruido %ld", l.muestrasPorSegundo, (long)l.ruidoPicoPico);
  pintar(zEstado, texto, COLOR_TENUE);

  // Evidencia de que el WiFi no dana las lecturas: las tramas lentas deben
  // quedarse en 0, y las atipicas casi siempre en 0, sin subir con el WiFi.
  snprintf(texto, sizeof(texto), "lentas %lu · atípicas %lu", (unsigned long)l.tramasLentas, (unsigned long)l.atipicos);
  pintar(zPie, texto, l.tramasLentas || l.atipicos ? COLOR_AMBAR : COLOR_TENUE);

  for (uint8_t i = 0; i < 4; i++) {
    int8_t presionada = teclado::presionada((Tecla)i) ? 1 : 0;
    if (presionada == teclasPintadas[i]) continue;
    teclasPintadas[i] = presionada;
    pantalla::circulo(TECLAS_X[i], TECLAS_Y, TECLAS_RADIO, presionada ? COLOR_AZUL : COLOR_BORDE);
  }
}

}  // namespace vDiagnostico
}  // namespace ui

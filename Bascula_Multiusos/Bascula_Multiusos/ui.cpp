/*
 * ============================================================
 *  INTERFAZ EN LA PANTALLA REDONDA - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "ui.h"
#include "config.h"
#include "pantalla.h"
#include "logo.h"

namespace {

/* Una zona de texto que recuerda lo ultimo que pinto: solo se vuelve a
 * mandar a la pantalla cuando el texto o el color cambian. Asi el SPI
 * queda libre casi todo el tiempo y nada parpadea. */
struct Zona {
  int16_t  cx, cy, ancho, alto;
  Fuente   fuente;
  char     texto[40];
  uint16_t color;
  bool     pintada;
};

void pintar(Zona& z, const char* texto, uint16_t color = COLOR_TEXTO) {
  if (z.pintada && z.color == color && strncmp(z.texto, texto, sizeof(z.texto)) == 0) return;
  strlcpy(z.texto, texto, sizeof(z.texto));
  z.color   = color;
  z.pintada = true;
  pantalla::zonaTexto(z.cx, z.cy, z.ancho, z.alto, texto, z.fuente, color);
}

void olvidar(Zona& z) { z.pintada = false; }

// ------------------ anillo perimetral ------------------

int16_t anilloPintado = -1;     // grados ya dibujados; -1 = hay que dibujar todo
uint16_t anilloColor  = 0;

void anillo(float fraccion, uint16_t color) {
  int16_t grados = (int16_t)(constrain(fraccion, 0.0f, 1.0f) * 360.0f);
  if (grados == anilloPintado && color == anilloColor) return;

  if (anilloPintado < 0 || color != anilloColor) {
    pantalla::arco(ANILLO_EXT, ANILLO_INT, 0, grados, color);
    pantalla::arco(ANILLO_EXT, ANILLO_INT, grados, 360, COLOR_BORDE);
  } else if (grados > anilloPintado) {                 // crecio: solo el tramo nuevo
    pantalla::arco(ANILLO_EXT, ANILLO_INT, anilloPintado, grados, color);
  } else {                                             // bajo: se apaga el tramo sobrante
    pantalla::arco(ANILLO_EXT, ANILLO_INT, grados, anilloPintado, COLOR_BORDE);
  }
  anilloPintado = grados;
  anilloColor   = color;
}

// ------------------ prueba de hardware ------------------
// Centros y tamanos verificados contra el circulo util (radio 106).

Zona zTitulo = { 120,  50, 120, 26, F_TITULO };
Zona zAviso  = { 120,  78, 170, 16, F_CHICA  };
Zona zNumero = { 120, 112, 196, 52, F_NUMERO };
Zona zUnidad = { 120, 150, 150, 18, F_TEXTO  };
Zona zEstado = { 120, 172, 168, 16, F_CHICA  };

const int16_t TECLAS_Y       = 194;
const int16_t TECLAS_X[4]    = { 87, 109, 131, 153 };
const int16_t TECLAS_RADIO   = 6;
int8_t        teclasPintadas[4] = { -1, -1, -1, -1 };

// Una celda de su capacidad nominal, con ganancia 128, anda por los 2
// millones de cuentas: con eso el anillo se llena al cargarla completa.
const float CUENTAS_ANILLO_LLENO = 2097152.0f;

int32_t  ceroCrudo   = 0;
uint8_t  nivelBrillo = TFT_BRILLO;
uint32_t avisoHastaMs = 0;

void aviso(const char* texto, uint16_t color = COLOR_AZUL) {
  pintar(zAviso, texto, color);
  avisoHastaMs = millis() + 1500;
}

const char* nombreTecla(Tecla t) {
  switch (t) {
    case TECLA_MENU:   return "MENÚ";
    case TECLA_ARRIBA: return "ARRIBA";
    case TECLA_ABAJO:  return "ABAJO";
    default:           return "OK";
  }
}

}  // namespace

namespace ui {

// ------------------ arranque ------------------

void arranque(uint16_t duracionMs) {
  pantalla::limpiar();
  pantalla::arco(ANILLO_EXT, ANILLO_INT, 0, 360, COLOR_AZUL);

  pantalla::imagen(CENTRO_X - LOGO_ANCHO / 2, 92 - LOGO_ALTO / 2, LOGO_ANCHO, LOGO_ALTO, LOGO_PIXELES);
  pantalla::zonaTexto(120, 148, 184, 20, PROYECTO_NOMBRE, F_TEXTO, COLOR_TEXTO);
  pantalla::zonaTexto(120, 168, 180, 16, "Desarrollado por " PROYECTO_AUTOR, F_CHICA, COLOR_AZUL);
  pantalla::zonaTexto(120, 186, 150, 16, PROYECTO_ING, F_CHICA, COLOR_TENUE);
  pantalla::zonaTexto(120, 204, 104, 14, PROYECTO_WEB, F_CHICA, COLOR_TENUE);   // aqui abajo el circulo ya es angosto

  delay(duracionMs);
}

// ------------------ prueba de hardware ------------------

void pruebaHardwareEntrar() {
  pantalla::limpiar();
  anilloPintado = -1;
  for (Zona* z : { &zTitulo, &zAviso, &zNumero, &zUnidad, &zEstado }) olvidar(*z);
  for (int8_t& t : teclasPintadas) t = -1;

  pintar(zTitulo, "PRUEBA", COLOR_AZUL);
  pintar(zAviso, NOMBRE_PLACA, COLOR_TENUE);
  avisoHastaMs = millis() + 2500;
}

void pruebaHardwareTecla(const EventoTecla& e) {
  char texto[40];
  const char* tipos[] = { "corta", "larga", "repite" };
  snprintf(texto, sizeof(texto), "%s · %s", nombreTecla(e.tecla), tipos[e.tipo]);

  switch (e.tecla) {
    case TECLA_OK:
      if (e.tipo == PULSACION_CORTA) { ceroCrudo = bascula::leer().promedio; aviso("OK · cero tomado", COLOR_VERDE); return; }
      if (e.tipo == PULSACION_LARGA) { ceroCrudo = 0; aviso("OK larga · sin cero", COLOR_AMBAR); return; }
      break;

    case TECLA_ARRIBA:
    case TECLA_ABAJO:
      if (!pantalla::tieneBrillo()) { aviso("BL fijo a 3V3", COLOR_AMBAR); return; }
      nivelBrillo = constrain((int)nivelBrillo + (e.tecla == TECLA_ARRIBA ? 15 : -15), 10, 255);
      pantalla::brillo(nivelBrillo);
      snprintf(texto, sizeof(texto), "%s · brillo %u", nombreTecla(e.tecla), nivelBrillo);
      break;

    case TECLA_MENU:
      if (e.tipo == PULSACION_LARGA) {       // contornos de las zonas: revisar el diseno
        pantalla::depuracion(!pantalla::depuracionActiva());
        pruebaHardwareEntrar();
        return;
      }
      break;

    default: break;
  }
  aviso(texto);
}

void pruebaHardwareRefrescar(const Lectura& l) {
  char texto[40];

  if (avisoHastaMs && millis() > avisoHastaMs) {
    avisoHastaMs = 0;
    pintar(zAviso, "MENÚ larga: ver zonas", COLOR_TENUE);
  }

  // Un DOUT al aire se lee siempre "listo" y dispara las muestras por
  // segundo muy por encima de lo que el HX711 puede dar.
  bool doutAlAire = l.muestrasPorSegundo > HX_MUESTRAS_POR_SEGUNDO * 2.0f;

  if (l.estado == HX_SIN_RESPUESTA || doutAlAire) {
    pintar(zNumero, "SIN CELDA", COLOR_NARANJA);
    pintar(zUnidad, doutAlAire ? "revisa el cable DOUT" : "revisa el HX711", COLOR_TENUE);
    anillo(1.0f, COLOR_NARANJA);
  } else if (l.estado == HX_SATURADO) {
    pintar(zNumero, "SATURADO", COLOR_NARANJA);
    pintar(zUnidad, "revisa E+ E- A+ A-", COLOR_TENUE);
    anillo(1.0f, COLOR_NARANJA);
  } else {
    int32_t relativo = l.promedio - ceroCrudo;
    snprintf(texto, sizeof(texto), "%ld", (long)relativo);
    // El crudo completo (7 digitos y signo) no cabe con la fuente grande:
    // en ese caso baja a la de titulo. Con el cero tomado casi siempre cabe.
    zNumero.fuente = pantalla::anchoTexto(texto, F_NUMERO) <= zNumero.ancho ? F_NUMERO : F_TITULO;
    pintar(zNumero, texto, COLOR_TEXTO);
    pintar(zUnidad, ceroCrudo ? "desde el cero" : "cuentas crudas", COLOR_TENUE);
    anillo(fabsf((float)relativo) / CUENTAS_ANILLO_LLENO, COLOR_AZUL);
  }

  snprintf(texto, sizeof(texto), "%.1f SPS · ruido %ld", l.muestrasPorSegundo, (long)l.ruidoPicoPico);
  pintar(zEstado, texto, l.tramasLentas ? COLOR_AMBAR : COLOR_TENUE);

  for (uint8_t i = 0; i < 4; i++) {
    int8_t presionada = teclado::presionada((Tecla)i) ? 1 : 0;
    if (presionada == teclasPintadas[i]) continue;
    teclasPintadas[i] = presionada;
    pantalla::circulo(TECLAS_X[i], TECLAS_Y, TECLAS_RADIO, presionada ? COLOR_AZUL : COLOR_BORDE);
  }
}

}  // namespace ui

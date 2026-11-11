/*
 * ============================================================
 *  REPRODUCTOR - implementacion
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 */

#include "reproductor.h"
#include "config.h"
#include "formato.h"
#include "pantalla.h"

#include <LittleFS.h>
#include <ESP_I2S.h>
#include <JPEGDEC.h>

// El I2S de Arduino guarda 6 bloques de 240 muestras antes de sacarlas: lo que
// se acaba de escribir se oye esta cantidad de muestras despues.
static const int32_t COLCHON_I2S = 6 * 240;
static const size_t  BLOQUE = 256;                 // bytes ADPCM por vuelta = 512 muestras = 32 ms
static const uint32_t MAX_CUADRO = 48 * 1024;      // un JPEG de 240x240 anda en 4 a 10 KB

static I2SClass i2s;
static uint32_t i2sHz = 0;
static JPEGDEC  jpeg;

static File        fVideo, fAudio;
static CabeceraLtd cab;
static uint32_t*   indice = nullptr;
static uint8_t*    bufJpeg = nullptr;
static uint32_t    tamBuf = 0;
static uint32_t    tamArchivo = 0;

static SemaphoreHandle_t candado;                  // protege archivo y estado frente a la tarea de audio
static volatile bool     activo = false;
static volatile bool     pausado = false;
static volatile uint16_t ganancia = 0;             // Q15

// Reloj del audio (lo escribe la tarea, lo lee el video)
static volatile uint32_t enviadas = 0;             // muestras entregadas al I2S en esta vuelta
static volatile uint32_t marca = 0;                // micros() de la ultima entrega
static volatile uint32_t vueltas = 0;

static uint32_t  audioFalta = 0;                   // bytes de audio que faltan en esta vuelta
static DecoAdpcm deco;

static int32_t  ultimoCuadro = -1;
static uint32_t posVideo = 0xFFFFFFFF;             // posicion del archivo de video (para no hacer seek de mas)
static uint32_t relojSinAudio = 0;                 // micros() del cuadro 0 cuando el archivo no trae audio
static uint32_t pausaDesde = 0;

// ------------------------------------------------------------
//  Audio
// ------------------------------------------------------------

static void audioAlInicio() {
  fAudio.seek(cab.audioPos);
  audioFalta = cab.audioBytes;
  deco.reiniciar();
}

static void tareaAudio(void*) {
  static uint8_t crudo[BLOQUE];
  static int16_t pcm[BLOQUE * 2 * 2];              // 2 muestras por byte, en estereo (L = R)

  for (;;) {
    bool trabajo = false;
    xSemaphoreTake(candado, portMAX_DELAY);
    if (activo && !pausado && cab.audioCodec == 1) {
      trabajo = true;
      size_t pedir = audioFalta < BLOQUE ? audioFalta : BLOQUE;
      int n = pedir ? fAudio.read(crudo, pedir) : 0;
      if (n <= 0) {                                // fin de la vuelta (o lectura fallida): de nuevo
        audioAlInicio();
        enviadas = 0;
        marca = micros();
        vueltas = vueltas + 1;
      } else {
        audioFalta -= n;
        const int32_t g = ganancia;
        int16_t* p = pcm;
        for (int i = 0; i < n; i++) {
          int16_t a = (int16_t)((deco.muestra(crudo[i] & 0x0F) * g) >> 15);
          int16_t b = (int16_t)((deco.muestra(crudo[i] >> 4) * g) >> 15);
          *p++ = a; *p++ = a; *p++ = b; *p++ = b;
        }
        i2s.write((const uint8_t*)pcm, (size_t)n * 8);   // espera a que haya lugar: marca el ritmo
        enviadas = enviadas + (uint32_t)n * 2;
        marca = micros();
      }
    }
    xSemaphoreGive(candado);
    // Un respiro fuera del candado: deja pasar a quien quiera cerrar el archivo.
    vTaskDelay(trabajo ? 1 : pdMS_TO_TICKS(10));
  }
}

static bool i2sPreparar(uint32_t hz) {
  if (i2sHz == hz) return true;
  if (i2sHz) i2s.end();
  i2sHz = 0;
  i2s.setPins(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DIN);
  // Estereo con la misma muestra en L y R: el MAX98357A promedia los dos canales.
  if (!i2s.begin(I2S_MODE_STD, hz, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) return false;
  i2sHz = hz;
  return true;
}

// ------------------------------------------------------------
//  Video
// ------------------------------------------------------------

static int dibujarBloque(JPEGDRAW* d) {
  gfx->draw16bitBeRGBBitmap(d->x, d->y, d->pPixels, d->iWidth, d->iHeight);
  return 1;
}

static void dibujarCuadro(uint32_t i) {
  uint32_t ini = indice[i], tam = indice[i + 1] - ini;
  if (tam == 0 || tam > tamBuf) return;
  if (posVideo != ini && !fVideo.seek(ini)) { posVideo = 0xFFFFFFFF; return; }
  if (fVideo.read(bufJpeg, tam) != tam) { posVideo = 0xFFFFFFFF; return; }
  posVideo = ini + tam;

  if (!jpeg.openRAM(bufJpeg, tam, dibujarBloque)) return;
  jpeg.setPixelType(RGB565_BIG_ENDIAN);
  int x = (240 - jpeg.getWidth()) / 2, y = (240 - jpeg.getHeight()) / 2;
  jpeg.decode(x < 0 ? 0 : x, y < 0 ? 0 : y, 0);
  jpeg.close();
}

// Cuadro que corresponde a lo que se esta oyendo en este instante.
static uint32_t cuadroDelMomento() {
  uint64_t num;
  if (cab.audioCodec == 1) {
    uint32_t e, m;
    do { e = enviadas; m = marca; } while (e != enviadas);
    int32_t pos = (int32_t)e - COLCHON_I2S + (int32_t)((uint64_t)(micros() - m) * cab.audioHz / 1000000ULL);
    if (pos > (int32_t)e) pos = e;                 // en pausa el reloj se detiene solo
    if (pos < 0) pos = vueltas ? pos + (int32_t)cab.audioMuestras : 0;   // la cola de la vuelta anterior
    num = (uint64_t)pos * cab.fpsX100 / ((uint64_t)cab.audioHz * 100);
  } else {
    num = (uint64_t)(micros() - relojSinAudio) * cab.fpsX100 / 100000000ULL;
    if (num >= cab.cuadros) {                      // vuelta completa: reancla el reloj
      relojSinAudio = micros();
      num = 0;
    }
  }
  return num >= cab.cuadros ? cab.cuadros - 1 : (uint32_t)num;
}

void reproductorTick() {
  if (!activo || pausado) return;
  uint32_t c = cuadroDelMomento();
  if ((int32_t)c == ultimoCuadro) return;
  ultimoCuadro = c;
  dibujarCuadro(c);
}

// ------------------------------------------------------------
//  Abrir / cerrar
// ------------------------------------------------------------

void reproductorIniciar() {
  candado = xSemaphoreCreateMutex();
  memset(&cab, 0, sizeof(cab));
  xTaskCreate(tareaAudio, "audio", 6144, nullptr, 5, nullptr);
}

static void soltar() {
  if (fVideo) fVideo.close();
  if (fAudio) fAudio.close();
  free(indice);  indice = nullptr;
  free(bufJpeg); bufJpeg = nullptr;
  tamBuf = 0;
  cab.cuadros = 0;
  cab.audioCodec = 0;
}

void reproductorCerrar() {
  xSemaphoreTake(candado, portMAX_DELAY);
  activo = false;
  soltar();
  xSemaphoreGive(candado);
}

const char* reproductorAbrir() {
  reproductorCerrar();
  xSemaphoreTake(candado, portMAX_DELAY);
  const char* error = nullptr;

  do {
    fVideo = LittleFS.open(RUTA_VIDEO, "r");
    if (!fVideo) { error = "No hay video"; break; }
    tamArchivo = fVideo.size();
    if (fVideo.read((uint8_t*)&cab, sizeof(cab)) != sizeof(cab) || memcmp(cab.magia, "LTD1", 4) != 0) {
      error = "El archivo no es .ltd"; break;
    }
    if (cab.cuadros == 0 || cab.fpsX100 < 100 || cab.fpsX100 > 3000) { error = "Cabecera invalida"; break; }

    size_t bytesIndice = ((size_t)cab.cuadros + 1) * 4;
    indice = (uint32_t*)malloc(bytesIndice);
    if (!indice) { error = "Sin memoria (indice)"; break; }
    if (fVideo.read((uint8_t*)indice, bytesIndice) != bytesIndice) { error = "Archivo incompleto"; break; }

    uint32_t mayor = 0;
    bool bien = indice[0] >= sizeof(cab) + bytesIndice;
    for (uint32_t i = 0; bien && i < cab.cuadros; i++) {
      if (indice[i + 1] <= indice[i] || indice[i + 1] > tamArchivo) bien = false;
      else if (indice[i + 1] - indice[i] > mayor) mayor = indice[i + 1] - indice[i];
    }
    if (!bien) { error = "Archivo incompleto"; break; }
    if (mayor > MAX_CUADRO) { error = "Cuadros demasiado grandes"; break; }

    tamBuf = mayor;
    bufJpeg = (uint8_t*)malloc(tamBuf);
    if (!bufJpeg) { error = "Sin memoria (cuadro)"; break; }

    if (cab.audioCodec == 1) {
      bool audioBien = cab.audioHz >= 8000 && cab.audioHz <= 48000 && cab.audioBytes > 0 &&
                       cab.audioMuestras > 0 && (uint64_t)cab.audioPos + cab.audioBytes <= tamArchivo;
      if (audioBien) fAudio = LittleFS.open(RUTA_VIDEO, "r");
      if (!audioBien || !fAudio || !i2sPreparar(cab.audioHz)) {
        REGISTRO("[audio] no disponible: se reproduce solo el video\n");
        if (fAudio) fAudio.close();
        cab.audioCodec = 0;
      } else {
        audioAlInicio();
      }
    } else {
      cab.audioCodec = 0;
    }
  } while (false);

  if (error) {
    soltar();
  } else {
    enviadas = 0; vueltas = 0; marca = micros();
    relojSinAudio = micros();
    ultimoCuadro = -1;
    posVideo = 0xFFFFFFFF;
    pausado = false;
    activo = true;
    REGISTRO("[video] %u cuadros a %.2f fps, audio %s, %u bytes\n", cab.cuadros, cab.fpsX100 / 100.0f,
             cab.audioCodec ? "si" : "no", (unsigned)tamArchivo);
  }
  xSemaphoreGive(candado);
  return error;
}

// ------------------------------------------------------------
//  Controles
// ------------------------------------------------------------

void reproductorPausa(bool pausar) {
  if (!activo || pausar == pausado) return;
  if (pausar) pausaDesde = micros();
  else relojSinAudio += micros() - pausaDesde;     // sin audio: que no brinque al volver
  pausado = pausar;
  if (!pausar) ultimoCuadro = -1;                  // redibuja al volver (la pantalla pudo mostrar otra cosa)
}

bool reproductorEnPausa() { return pausado; }
bool reproductorActivo()  { return activo; }

void reproductorVolumen(uint8_t porciento) {
  if (porciento > 100) porciento = 100;
  float v = porciento / 100.0f;
  ganancia = (uint16_t)(v * v * VOLUMEN_TOPE * 32767.0f);   // curva cuadratica: se siente pareja al oido
}

InfoVideo reproductorInfo() {
  InfoVideo info = {};
  info.hay = activo;
  if (activo) {
    info.bytes = tamArchivo;
    info.cuadros = cab.cuadros;
    info.fps = cab.fpsX100 / 100.0f;
    info.segundos = cab.cuadros * 100.0f / cab.fpsX100;
    info.audio = cab.audioCodec == 1;
  }
  return info;
}

/*
 * ============================================================
 *  CAMARA WiFi CON PORTAL CAUTIVO - ESP32-WROVER-CAM + OV2640
 *  Tostatronic - www.tostatronic.com
 * ============================================================
 *
 *  Sin claves de WiFi escritas en el codigo. La placa arranca asi:
 *
 *    1. NO tiene WiFi guardado  -> abre la red "Tostatronic-CAM".
 *       Te conectas con el telefono y se abre solo un portal
 *       cautivo que escanea las redes, eliges la tuya y pones
 *       la clave. La pagina te muestra la IP que le toco.
 *
 *    2. YA tiene WiFi guardado  -> se enlaza a tu red y sirve una
 *       pagina con el video de la camara y el boton
 *       "Iniciar captura".  Entra por la IP o por
 *       http://tostacam.local
 *
 *  Para volver al portal: boton "Olvidar esta red WiFi" de la
 *  pagina, o deja presionado BOOT 3 segundos.
 *
 *  ------------------------------------------------------------
 *  ARDUINO IDE
 *  ------------------------------------------------------------
 *    Placa ............ ESP32 Wrover Module
 *    Partition Scheme . Huge APP (3MB No OTA/1MB SPIFFS)
 *    Core ............. esp32 de Espressif 3.x
 *
 *    Con "ESP32 Wrover Module" la PSRAM ya queda habilitada. Si
 *    usas "ESP32 Dev Module", activa a mano  PSRAM: Enabled.
 *    No hay que instalar librerias: todo viene en el core.
 *
 *  ------------------------------------------------------------
 *  POR QUE DOS SERVIDORES
 *  ------------------------------------------------------------
 *  Un stream MJPEG es una respuesta HTTP que nunca termina. Si
 *  viviera en el mismo servidor que la pagina, mientras hay video
 *  no se atenderia ningun boton. Por eso:
 *      puerto 80 -> pagina, foto, estado, resolucion
 *      puerto 81 -> solo el stream
 *  Cada servidor corre en su propia tarea de FreeRTOS.
 *
 *  ------------------------------------------------------------
 *  OJO CON LOS GPIO
 *  ------------------------------------------------------------
 *  La camara ocupa 14 GPIO (ver la tabla de abajo) y la Flash y
 *  PSRAM otros 8, entre ellos GPIO16 y GPIO17. No cuelgues nada
 *  ahi. Este sketch solo usa GPIO2 (LED) y GPIO0 (boton BOOT).
 * ============================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "esp_camera.h"

// WebServer (portal) y esp_http_server (camara) definen HTTP_ANY cada
// uno a su modo. Aqui no se usa: se suelta para compilar sin warnings.
#undef HTTP_ANY
#include "esp_http_server.h"
#include "paginas.h"

// ------------------ PLACA ------------------

#if !defined(CONFIG_IDF_TARGET_ESP32)
  #error "Este sketch es para la ESP32-WROVER-CAM (ESP32 clasico). Elige la placa ESP32 Wrover Module."
#endif

// ------------------ CONFIGURACION ------------------

const char* AP_SSID     = "Tostatronic-CAM";  // red que abre el portal
const char* AP_CLAVE    = "";                 // "" = abierta. Si pones clave, minimo 8 caracteres
const char* NOMBRE_MDNS = "tostacam";         // http://tostacam.local

const uint8_t PIN_LED  = 2;   // LED de usuario de la placa
const uint8_t PIN_BOOT = 0;   // boton BOOT: 3 s presionado = olvidar WiFi

const uint32_t ESPERA_WIFI_MS   = 20000;   // tope para enlazarse al router
const uint32_t REINTENTO_MS     = 180000;  // portal sin nadie conectado: reintenta la red guardada
const uint32_t PULSACION_BOOT_MS = 3000;

// Si la imagen te sale de cabeza o en espejo (depende de como
// montes la placa), se corrige aqui sin tocar nada mas.
const bool VOLTEO_VERTICAL    = false;
const bool ESPEJO_HORIZONTAL  = false;

const uint8_t CALIDAD_JPEG = 12;   // 10 = mejor imagen ... 63 = mas compresion

// Pines de la OV2640 en la ESP32-WROVER-CAM (mismo mapa que la
// WROVER-KIT de Espressif). PWDN y RESET no estan cableados.
#define CAM_PIN_PWDN   -1
#define CAM_PIN_RESET  -1
#define CAM_PIN_XCLK   21
#define CAM_PIN_SIOD   26
#define CAM_PIN_SIOC   27
#define CAM_PIN_Y9     35
#define CAM_PIN_Y8     34
#define CAM_PIN_Y7     39
#define CAM_PIN_Y6     36
#define CAM_PIN_Y5     19
#define CAM_PIN_Y4     18
#define CAM_PIN_Y3      5
#define CAM_PIN_Y2      4
#define CAM_PIN_VSYNC  25
#define CAM_PIN_HREF   23
#define CAM_PIN_PCLK   22

// ------------------ RESOLUCIONES ------------------

struct Resolucion {
  const char* clave;     // lo que viaja en la URL
  const char* nombre;    // lo que se ve en el boton
  framesize_t tamano;
  uint16_t    ancho;
  uint16_t    alto;
};

// De menor a mayor. Sin PSRAM el tope es SVGA: el cuadro ya no
// cabe en la RAM interna (para eso son los 4 MB de PSRAM).
const Resolucion RESOLUCIONES[] = {
  { "qvga", "QVGA", FRAMESIZE_QVGA,  320,  240 },
  { "vga",  "VGA",  FRAMESIZE_VGA,   640,  480 },
  { "svga", "SVGA", FRAMESIZE_SVGA,  800,  600 },
  { "xga",  "XGA",  FRAMESIZE_XGA,  1024,  768 },
  { "uxga", "UXGA", FRAMESIZE_UXGA, 1600, 1200 },
};

const uint8_t NUM_RESOLUCIONES = sizeof(RESOLUCIONES) / sizeof(RESOLUCIONES[0]);
const uint8_t RES_INICIAL      = 1;   // VGA: buen balance entre nitidez y cuadros por segundo
const uint8_t RES_TOPE_SIN_PSRAM = 2; // SVGA

uint8_t resMaxima = NUM_RESOLUCIONES - 1;
volatile uint8_t resActual = RES_INICIAL;

// ------------------ ESTADO GLOBAL ------------------

enum Modo { MODO_PORTAL, MODO_CAMARA };
enum EstadoConexion { INACTIVO, CONECTANDO, CONECTADO, FALLO };

Modo           modo = MODO_PORTAL;
EstadoConexion estadoConexion = INACTIVO;

Preferences prefs;
String ssidGuardado, claveGuardada;

// --- portal ---
WebServer  servidorPortal(80);
DNSServer  dns;
const IPAddress IP_PORTAL(192, 168, 4, 1);

String   ssidPendiente, clavePendiente;
uint32_t inicioIntento   = 0;
uint32_t momentoConectado = 0;
uint32_t momentoIpVista  = 0;    // cuando el telefono ya leyo la IP
uint32_t ultimaActividad = 0;

// --- camara ---
httpd_handle_t servidorWeb    = NULL;   // puerto 80
httpd_handle_t servidorStream = NULL;   // puerto 81

volatile bool  transmitiendo   = false;
volatile float fpsActual       = 0;
volatile bool  reinicioPedido  = false;

#define LIMITE_PARTE "tostatronic0123456789cam"
static const char* TIPO_STREAM   = "multipart/x-mixed-replace;boundary=" LIMITE_PARTE;
static const char* LIMITE_STREAM = "\r\n--" LIMITE_PARTE "\r\n";
static const char* PARTE_STREAM  = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// ------------------ CREDENCIALES (NVS) ------------------

void cargarCredenciales() {
  prefs.begin("wifi", true);
  ssidGuardado  = prefs.getString("ssid", "");
  claveGuardada = prefs.getString("clave", "");
  prefs.end();
}

void guardarCredenciales(const String& ssid, const String& clave) {
  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("clave", clave);
  prefs.end();
}

void borrarCredenciales() {
  prefs.begin("wifi", false);
  prefs.clear();
  prefs.end();
}

// ------------------ UTILERIAS ------------------

/* Un SSID puede traer comillas o diagonales: se escapan para que
 * el JSON no se rompa (ni se cuele nada raro a la pagina). */
String escaparJson(const String& s) {
  String r;
  r.reserve(s.length() + 4);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') { r += '\\'; r += c; }
    else if ((uint8_t)c < 0x20) r += ' ';
    else r += c;
  }
  return r;
}

/* LED sin delay(): periodo 0 = encendido fijo */
void parpadear(uint16_t periodoMs) {
  if (periodoMs == 0) { digitalWrite(PIN_LED, HIGH); return; }
  digitalWrite(PIN_LED, (millis() / periodoMs) % 2);
}

/* BOOT presionado 3 s seguidos = olvidar el WiFi y volver al portal */
void revisarBotonBoot() {
  static uint32_t inicioPulsacion = 0;

  if (digitalRead(PIN_BOOT) == HIGH) { inicioPulsacion = 0; return; }
  if (inicioPulsacion == 0) { inicioPulsacion = millis(); return; }

  if (millis() - inicioPulsacion >= PULSACION_BOOT_MS) {
    Serial.println("BOOT 3 s: se borra el WiFi guardado y se reinicia.");
    borrarCredenciales();
    for (uint8_t i = 0; i < 10; i++) {   // aviso visual
      digitalWrite(PIN_LED, i % 2);
      delay(60);
    }
    ESP.restart();
  }
}

// ============================================================
//  MODO PORTAL
// ============================================================

void portalRaiz() {
  ultimaActividad = millis();
  servidorPortal.sendHeader("Cache-Control", "no-store");
  servidorPortal.send_P(200, "text/html", PAGINA_PORTAL);
}

/* Todo lo que no sea del portal se redirige a la raiz. Asi es como
 * Android (generate_204), iOS (hotspot-detect.html) y Windows
 * (connecttest.txt) detectan que hay portal y lo abren solos. */
void portalRedirigir() {
  servidorPortal.sendHeader("Location", String("http://") + IP_PORTAL.toString() + "/", true);
  servidorPortal.send(302, "text/plain", "");
}

/* Escaneo asincrono: nunca bloquea al servidor. La pagina pregunta
 * cada segundo hasta que el estado sea "listo". */
void portalRedes() {
  ultimaActividad = millis();
  int16_t n = WiFi.scanComplete();

  if (n >= 0 && servidorPortal.hasArg("nuevo")) {   // resultados viejos: fuera
    WiFi.scanDelete();
    n = WIFI_SCAN_FAILED;
  }
  if (n == WIFI_SCAN_FAILED && estadoConexion != CONECTANDO) {
    WiFi.scanNetworks(true);                        // true = asincrono
    n = WIFI_SCAN_RUNNING;
  }
  if (n < 0) {
    servidorPortal.send(200, "application/json", "{\"estado\":\"escaneando\"}");
    return;
  }

  String json = "{\"estado\":\"listo\",\"redes\":[";
  for (int16_t i = 0; i < n; i++) {
    if (i) json += ',';
    json += "{\"ssid\":\"" + escaparJson(WiFi.SSID(i)) + "\"";
    json += ",\"rssi\":" + String(WiFi.RSSI(i));
    json += ",\"segura\":";
    json += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "false" : "true";
    json += '}';
  }
  json += "]}";
  WiFi.scanDelete();
  servidorPortal.send(200, "application/json", json);
}

void portalConectar() {
  ultimaActividad = millis();
  String ssid  = servidorPortal.arg("ssid");
  String clave = servidorPortal.arg("clave");

  bool claveValida = clave.length() == 0 || (clave.length() >= 8 && clave.length() <= 63);
  if (ssid.length() == 0 || ssid.length() > 32 || !claveValida) {
    servidorPortal.send(400, "application/json", "{\"error\":\"datos invalidos\"}");
    return;
  }

  ssidPendiente  = ssid;
  clavePendiente = clave;
  estadoConexion = CONECTANDO;
  inicioIntento  = millis();
  servidorPortal.send(200, "application/json", "{\"estado\":\"conectando\"}");

  Serial.printf("Probando la red \"%s\"...\n", ssid.c_str());
  WiFi.scanDelete();
  WiFi.begin(ssid.c_str(), clave.length() ? clave.c_str() : NULL);
}

void portalEstado() {
  const char* nombres[] = { "inactivo", "conectando", "conectado", "fallo" };

  String json = "{\"estado\":\"";
  json += nombres[estadoConexion];
  json += "\",\"ssid\":\"" + escaparJson(ssidPendiente) + "\"";
  if (estadoConexion == CONECTADO) {
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += ",\"mdns\":\"" + String(NOMBRE_MDNS) + "\"";
    if (momentoIpVista == 0) momentoIpVista = millis();
  }
  json += '}';
  servidorPortal.send(200, "application/json", json);
}

void iniciarPortal(bool falloAlArrancar) {
  modo = MODO_PORTAL;

  // AP + STA a la vez: el AP sirve el portal mientras la parte STA
  // escanea y prueba la clave, sin tirar al telefono.
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(false);
  WiFi.disconnect();   // si venimos de un intento fallido, que deje de buscar esa red
  WiFi.softAPConfig(IP_PORTAL, IP_PORTAL, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, strlen(AP_CLAVE) >= 8 ? AP_CLAVE : NULL);

  // DNS comodin: cualquier dominio resuelve a la placa.
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", IP_PORTAL);

  servidorPortal.on("/", portalRaiz);
  servidorPortal.on("/redes", portalRedes);
  servidorPortal.on("/conectar", HTTP_POST, portalConectar);
  servidorPortal.on("/estado", portalEstado);
  servidorPortal.onNotFound(portalRedirigir);
  servidorPortal.begin();

  if (falloAlArrancar) {          // la pagina avisa que la red guardada no respondio
    estadoConexion = FALLO;
    ssidPendiente  = ssidGuardado;
  }

  WiFi.scanNetworks(true);        // que la lista ya este lista cuando abran el portal
  ultimaActividad = millis();

  Serial.println();
  Serial.println("--- MODO PORTAL ---");
  Serial.printf("Conectate a la red WiFi \"%s\"%s\n", AP_SSID,
                strlen(AP_CLAVE) >= 8 ? " (con clave)" : " (abierta)");
  Serial.printf("Si el portal no abre solo, entra a http://%s\n", IP_PORTAL.toString().c_str());
}

void atenderPortal() {
  dns.processNextRequest();
  servidorPortal.handleClient();

  switch (estadoConexion) {
    case CONECTANDO:
      parpadear(100);
      if (WiFi.status() == WL_CONNECTED) {
        guardarCredenciales(ssidPendiente, clavePendiente);
        estadoConexion   = CONECTADO;
        momentoConectado = millis();
        momentoIpVista   = 0;
        Serial.printf("Enlazada. IP: %s  (se guarda y se reinicia en modo camara)\n",
                      WiFi.localIP().toString().c_str());
      } else if (millis() - inicioIntento > ESPERA_WIFI_MS) {
        WiFi.disconnect();          // que deje de reintentar: si no, el AP se vuelve inestable
        estadoConexion = FALLO;
        Serial.println("No se pudo enlazar: clave incorrecta o red fuera de alcance.");
      }
      break;

    case CONECTADO:
      parpadear(0);
      // Se le da tiempo al telefono de leer la IP en pantalla. Si nunca la
      // pidio (se solto del AP), se reinicia igual: la IP sale por Serial.
      if ((momentoIpVista && millis() - momentoIpVista > 8000) ||
          millis() - momentoConectado > 30000) {
        ESP.restart();
      }
      break;

    default:
      parpadear(500);
      // Hay red guardada pero el router no estaba (p. ej. se fue la luz):
      // si nadie esta usando el portal, se vuelve a intentar.
      if (ssidGuardado.length() && WiFi.softAPgetStationNum() == 0 &&
          millis() - ultimaActividad > REINTENTO_MS) {
        Serial.println("Portal sin uso: se reintenta la red guardada.");
        ESP.restart();
      }
      break;
  }
}

// ============================================================
//  MODO CAMARA
// ============================================================

bool iniciarCamara() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_pwdn     = CAM_PIN_PWDN;
  config.pin_reset    = CAM_PIN_RESET;
  config.pin_xclk     = CAM_PIN_XCLK;
  config.pin_sccb_sda = CAM_PIN_SIOD;
  config.pin_sccb_scl = CAM_PIN_SIOC;
  config.pin_d7       = CAM_PIN_Y9;
  config.pin_d6       = CAM_PIN_Y8;
  config.pin_d5       = CAM_PIN_Y7;
  config.pin_d4       = CAM_PIN_Y6;
  config.pin_d3       = CAM_PIN_Y5;
  config.pin_d2       = CAM_PIN_Y4;
  config.pin_d1       = CAM_PIN_Y3;
  config.pin_d0       = CAM_PIN_Y2;
  config.pin_vsync    = CAM_PIN_VSYNC;
  config.pin_href     = CAM_PIN_HREF;
  config.pin_pclk     = CAM_PIN_PCLK;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;   // el OV2640 comprime por hardware
  config.jpeg_quality = CALIDAD_JPEG;

  if (psramFound()) {
    // El driver reserva los buffers UNA vez, al iniciar: se piden para la
    // resolucion mas grande y despues se baja. Asi se puede cambiar de
    // resolucion en caliente sin reiniciar la camara.
    resMaxima          = NUM_RESOLUCIONES - 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.fb_count    = 2;
    config.grab_mode   = CAMERA_GRAB_LATEST;   // siempre el cuadro mas nuevo: menos retraso
  } else {
    Serial.println("AVISO: no se detecto PSRAM. Resolucion limitada a SVGA.");
    resMaxima          = RES_TOPE_SIN_PSRAM;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.fb_count    = 1;
    config.grab_mode   = CAMERA_GRAB_WHEN_EMPTY;
  }
  config.frame_size = RESOLUCIONES[resMaxima].tamano;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERROR: la camara no inicio (0x%x). Revisa el cable flex de la OV2640.\n", err);
    return false;
  }

  sensor_t* s = esp_camera_sensor_get();
  resActual = min(RES_INICIAL, resMaxima);
  s->set_framesize(s, RESOLUCIONES[resActual].tamano);
  s->set_vflip(s, VOLTEO_VERTICAL);
  s->set_hmirror(s, ESPEJO_HORIZONTAL);
  return true;
}

// ---------- puerto 80 ----------

static esp_err_t camRaiz(httpd_req_t* req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, PAGINA_CAMARA, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t camEstado(httpd_req_t* req) {
  const Resolucion& r = RESOLUCIONES[resActual];

  String json = "{\"transmitiendo\":";
  json += transmitiendo ? "true" : "false";
  json += ",\"fps\":" + String(fpsActual, 1);
  json += ",\"res\":\"" + String(r.clave) + "\"";
  json += ",\"ancho\":" + String(r.ancho) + ",\"alto\":" + String(r.alto);
  json += ",\"rssi\":" + String(WiFi.RSSI());
  json += ",\"psram_total\":" + String(ESP.getPsramSize());
  json += ",\"psram_libre\":" + String(ESP.getFreePsram());
  json += ",\"ap\":\"" + escaparJson(AP_SSID) + "\"";
  json += ",\"resoluciones\":[";
  for (uint8_t i = 0; i <= resMaxima; i++) {
    if (i) json += ',';
    json += "{\"clave\":\"" + String(RESOLUCIONES[i].clave) + "\"";
    json += ",\"nombre\":\"" + String(RESOLUCIONES[i].nombre) + "\"";
    json += ",\"ancho\":" + String(RESOLUCIONES[i].ancho);
    json += ",\"alto\":" + String(RESOLUCIONES[i].alto) + "}";
  }
  json += "]}";

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, json.c_str(), json.length());
}

/* /resolucion?val=vga  -> se aplica en caliente, con el video corriendo */
static esp_err_t camResolucion(httpd_req_t* req) {
  char consulta[32], valor[8];

  if (httpd_req_get_url_query_str(req, consulta, sizeof(consulta)) == ESP_OK &&
      httpd_query_key_value(consulta, "val", valor, sizeof(valor)) == ESP_OK) {
    for (uint8_t i = 0; i <= resMaxima; i++) {
      if (strcmp(valor, RESOLUCIONES[i].clave) != 0) continue;

      sensor_t* s = esp_camera_sensor_get();
      if (s->set_framesize(s, RESOLUCIONES[i].tamano) != 0) break;
      resActual = i;
      Serial.printf("Resolucion: %s (%ux%u)\n", RESOLUCIONES[i].nombre,
                    RESOLUCIONES[i].ancho, RESOLUCIONES[i].alto);
      return httpd_resp_sendstr(req, "ok");
    }
  }
  httpd_resp_set_status(req, "400 Bad Request");
  return httpd_resp_sendstr(req, "resolucion no valida");
}

/* Una sola foto JPEG, para el boton "Tomar foto" */
static esp_err_t camFoto(httpd_req_t* req) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=tostacam.jpg");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  esp_err_t res = httpd_resp_send(req, (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

static esp_err_t camOlvidar(httpd_req_t* req) {
  Serial.println("Olvidar WiFi pedido desde la pagina.");
  borrarCredenciales();
  reinicioPedido = true;    // se reinicia desde loop(), ya que salio la respuesta
  return httpd_resp_sendstr(req, "ok");
}

// ---------- puerto 81 ----------

/* Stream MJPEG: una respuesta que no termina. Cada cuadro JPEG va
 * como una "parte" y el navegador va reemplazando la imagen. El
 * ciclo solo se rompe cuando el navegador cierra la conexion
 * (boton "Detener captura"), porque ahi falla el envio. */
static esp_err_t camStream(httpd_req_t* req) {
  char encabezado[64];

  esp_err_t res = httpd_resp_set_type(req, TIPO_STREAM);
  if (res != ESP_OK) return res;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");

  Serial.println("Captura iniciada.");
  transmitiendo = true;
  uint32_t inicioVentana = millis();
  uint16_t cuadros = 0;

  while (true) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("ERROR: la camara no entrego cuadro.");
      res = ESP_FAIL;
      break;
    }

    size_t n = snprintf(encabezado, sizeof(encabezado), PARTE_STREAM, (unsigned)fb->len);
    res = httpd_resp_send_chunk(req, LIMITE_STREAM, strlen(LIMITE_STREAM));
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, encabezado, n);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
    esp_camera_fb_return(fb);   // SIEMPRE devolver el buffer, o la camara se queda sin cuadros
    if (res != ESP_OK) break;

    cuadros++;
    uint32_t transcurrido = millis() - inicioVentana;
    if (transcurrido >= 1000) {
      fpsActual     = cuadros * 1000.0f / transcurrido;
      cuadros       = 0;
      inicioVentana = millis();
    }
  }

  transmitiendo = false;
  fpsActual     = 0;
  Serial.println("Captura detenida.");
  return res;
}

void iniciarServidoresCamara() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;

  httpd_uri_t rutas[] = {
    { "/",           HTTP_GET,  camRaiz,       NULL },
    { "/estado",     HTTP_GET,  camEstado,     NULL },
    { "/resolucion", HTTP_GET,  camResolucion, NULL },
    { "/foto",       HTTP_GET,  camFoto,       NULL },
    { "/olvidar",    HTTP_POST, camOlvidar,    NULL },
  };

  if (httpd_start(&servidorWeb, &config) == ESP_OK) {
    for (auto& ruta : rutas) httpd_register_uri_handler(servidorWeb, &ruta);
  }

  // Segunda instancia: necesita su propio puerto de control.
  config.server_port += 1;
  config.ctrl_port   += 1;
  httpd_uri_t rutaStream = { "/stream", HTTP_GET, camStream, NULL };

  if (httpd_start(&servidorStream, &config) == ESP_OK) {
    httpd_register_uri_handler(servidorStream, &rutaStream);
  }
}

/* Intenta enlazarse con la red guardada. Devuelve true si lo logro. */
bool conectarRedGuardada() {
  Serial.printf("Conectando a \"%s\"", ssidGuardado.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(NOMBRE_MDNS);
  WiFi.setSleep(false);   // sin ahorro de energia en el radio: el video va mas fluido
  WiFi.begin(ssidGuardado.c_str(), claveGuardada.length() ? claveGuardada.c_str() : NULL);

  uint32_t inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < ESPERA_WIFI_MS) {
    parpadear(100);
    revisarBotonBoot();
    if (millis() % 500 < 20) Serial.print('.');
    delay(20);
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void iniciarModoCamara() {
  modo = MODO_CAMARA;

  if (!iniciarCamara()) {
    // Sin camara no hay nada que mostrar: parpadeo rapido para siempre
    // (BOOT 3 s sigue funcionando para volver al portal).
    while (true) { parpadear(60); revisarBotonBoot(); delay(10); }
  }

  iniciarServidoresCamara();
  if (MDNS.begin(NOMBRE_MDNS)) MDNS.addService("http", "tcp", 80);
  parpadear(0);

  Serial.println();
  Serial.println("--- MODO CAMARA ---");
  Serial.printf("PSRAM: %s\n", psramFound() ? "detectada" : "NO detectada");
  Serial.printf("Abre en el navegador:  http://%s   o   http://%s.local\n",
                WiFi.localIP().toString().c_str(), NOMBRE_MDNS);
}

// ------------------ SETUP / LOOP ------------------

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BOOT, INPUT_PULLUP);

  Serial.println();
  Serial.println("=== Camara WiFi con portal cautivo | ESP32-WROVER-CAM ===");

  // Las claves las maneja este sketch en su propio espacio de NVS;
  // que el driver de WiFi no guarde otra copia por su cuenta.
  WiFi.persistent(false);
  cargarCredenciales();

  if (ssidGuardado.length() == 0) {
    Serial.println("No hay WiFi guardado.");
    iniciarPortal(false);
  } else if (conectarRedGuardada()) {
    iniciarModoCamara();
  } else {
    Serial.printf("No se pudo conectar a \"%s\".\n", ssidGuardado.c_str());
    iniciarPortal(true);
  }
}

void loop() {
  revisarBotonBoot();

  if (modo == MODO_PORTAL) {
    atenderPortal();
    return;
  }

  // En modo camara los dos servidores corren en sus propias tareas:
  // aqui solo queda vigilar el reinicio que pide "Olvidar WiFi".
  if (reinicioPedido) {
    delay(500);   // deja salir la respuesta HTTP
    ESP.restart();
  }
  delay(10);
}

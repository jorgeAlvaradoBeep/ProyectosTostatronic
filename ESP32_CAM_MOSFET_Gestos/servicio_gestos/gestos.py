"""
Servicio de gestos - Tostatronic (www.tostatronic.com)
=======================================================

Lee el video de la ESP32-WROVER-CAM por WiFi, detecta tu mano con MediaPipe
y prende o apaga la carga del circuito.

Modo "mano" (por defecto): cada vez que muestras la mano, la carga cambia de
estado (encendido <-> apagado). Retira la mano para el siguiente cambio.

Modo "gestos" (--modo gestos):

    pulgar arriba  ->  GET http://<camara>/carga?estado=1  ->  LED encendido
    puño cerrado   ->  GET http://<camara>/carga?estado=0  ->  LED apagado

    ESP32 GPIO33 -> 74LS07 -> pull-up a 12 V -> IRFZ44N -> modulo LED 12 V

Uso:
    python gestos.py --cam 192.168.1.50        (o --cam tostacam.local)
    python gestos.py --fuente 0 --simular      (prueba con la webcam, sin ESP32)

Teclas en la ventana:  E encender · A apagar · Q / Esc salir

Desarrollado por Tostatronic (Ing. Jorge Alvarado).
"""
import argparse
import os
import threading
import time
import urllib.request

import cv2
import mediapipe as mp
import numpy as np
from mediapipe.tasks import python as mp_python
from mediapipe.tasks.python import vision

AQUI = os.path.dirname(os.path.abspath(__file__))
MODELO = os.path.join(AQUI, "modelos", "gesture_recognizer.task")
MODELO_URL = ("https://storage.googleapis.com/mediapipe-models/gesture_recognizer/"
              "gesture_recognizer/float16/latest/gesture_recognizer.task")

# Con la ESP32-CAM el pulgar abajo se reconoce mal; el puño cerrado es estable.
ENCENDER, APAGAR = "Thumb_Up", "Closed_Fist"

# Colores (BGR), los mismos de la pagina de la camara
FONDO = (25, 21, 16)
VERDE = (167, 212, 45)
NARANJA = (56, 92, 255)
AMBAR = (41, 180, 240)
TENUE = (170, 161, 147)
BLANCO = (244, 246, 243)

# Huesos de la mano (21 puntos de MediaPipe)
CONEXIONES = [(0, 1), (1, 2), (2, 3), (3, 4), (0, 5), (5, 6), (6, 7), (7, 8), (5, 9), (9, 10), (10, 11),
              (11, 12), (9, 13), (13, 14), (14, 15), (15, 16), (13, 17), (17, 18), (18, 19), (19, 20), (0, 17)]


# ------------------------------------------------------------------ video

class LectorMJPEG(threading.Thread):
    """Lee el stream MJPEG de la ESP32 (puerto 81) y se queda solo con el cuadro
    mas nuevo: asi no se acumula retraso aunque el reconocimiento vaya mas lento."""

    def __init__(self, url):
        super().__init__(daemon=True)
        self.url, self.cuadro, self.conectado, self.vivo = url, None, False, True
        self.candado = threading.Lock()

    def run(self):
        while self.vivo:
            try:
                with urllib.request.urlopen(self.url, timeout=5) as r:
                    self.conectado, buf = True, b""
                    while self.vivo:
                        trozo = r.read(4096)
                        if not trozo:
                            break
                        buf += trozo
                        fin = buf.rfind(b"\xff\xd9")          # ultimo JPEG completo del buffer
                        if fin < 0:
                            continue
                        ini = buf.rfind(b"\xff\xd8", 0, fin)
                        if ini >= 0:
                            img = cv2.imdecode(np.frombuffer(buf[ini:fin + 2], np.uint8), cv2.IMREAD_COLOR)
                            if img is not None:
                                with self.candado:
                                    self.cuadro = img
                        buf = buf[fin + 2:]
            except Exception as e:
                print(f"[video] sin conexion con {self.url} ({e}); reintento en 2 s")
            self.conectado = False
            time.sleep(2)

    def leer(self):
        with self.candado:
            return None if self.cuadro is None else self.cuadro.copy()


class LectorLocal(threading.Thread):
    """Webcam (indice) o archivo de video, para probar sin la ESP32."""

    def __init__(self, fuente):
        super().__init__(daemon=True)
        self.cap = cv2.VideoCapture(int(fuente) if str(fuente).isdigit() else fuente)
        self.cuadro, self.conectado, self.vivo = None, self.cap.isOpened(), True
        self.candado = threading.Lock()
        self.fps = self.cap.get(cv2.CAP_PROP_FPS) or 30

    def run(self):
        while self.vivo:
            ok, img = self.cap.read()
            if not ok:
                self.cap.set(cv2.CAP_PROP_POS_FRAMES, 0)      # archivo: vuelve a empezar
                continue
            with self.candado:
                self.cuadro = img
            time.sleep(1 / self.fps)

    def leer(self):
        with self.candado:
            return None if self.cuadro is None else self.cuadro.copy()


# ------------------------------------------------------------------ carga

class Carga:
    """Manda /carga?estado=... a la ESP32 en segundo plano (la ventana nunca se congela)."""

    def __init__(self, host, simular):
        self.base, self.simular = f"http://{host}", simular
        self.encendida, self.error = False, ""
        self.evento = None    # ultimo comando: {"encender", "t", "estado": enviando|ok|error, "ms"} (para la pantalla)
        threading.Thread(target=self._pedir, args=(None,), daemon=True).start()   # estado inicial

    def poner(self, encender):
        self.evento = {"encender": encender, "t": time.monotonic(), "estado": "enviando", "ms": 0}
        if self.simular:
            self.encendida = encender
            self.evento.update(estado="ok")
            print(f"[carga] (simulada) {'ENCENDIDA' if encender else 'apagada'}")
            return
        threading.Thread(target=self._pedir, args=(encender,), daemon=True).start()

    def _pedir(self, encender):
        if self.simular:
            return
        url = self.base + "/carga" + ("" if encender is None else f"?estado={int(encender)}")
        evento, t0 = self.evento, time.monotonic()
        try:
            with urllib.request.urlopen(url, timeout=6) as r:
                estado = b"true" in r.read()
                self.error = ""
            # La consulta inicial puede llegar tarde: si ya se mando un comando, no lo pisa.
            if encender is not None or self.evento is None:
                self.encendida = estado
            if encender is not None:
                ms = int((time.monotonic() - t0) * 1000)
                if evento:
                    evento.update(estado="ok", ms=ms)
                print(f"[carga] {'ENCENDIDA' if self.encendida else 'apagada'} ({ms} ms)")
        except Exception as e:
            self.error = "sin respuesta de la ESP32"
            if encender is not None and evento:
                evento.update(estado="error")
            print(f"[carga] {url} fallo: {e}")


# ------------------------------------------------------------------ reconocimiento

def asegurar_modelo():
    if not os.path.exists(MODELO):
        os.makedirs(os.path.dirname(MODELO), exist_ok=True)
        print("Descargando el modelo de gestos de MediaPipe (~8 MB)...")
        urllib.request.urlretrieve(MODELO_URL, MODELO)
    return MODELO


def crear_reconocedor(umbral):
    opciones = vision.GestureRecognizerOptions(
        base_options=mp_python.BaseOptions(model_asset_path=asegurar_modelo()),
        running_mode=vision.RunningMode.VIDEO,
        num_hands=1,
        min_hand_detection_confidence=0.4,     # a 320x240 la mano se ve chica: un poco mas permisivo
        min_hand_presence_confidence=0.4,
        min_tracking_confidence=0.5,
        canned_gesture_classifier_options=mp_python.components.processors.ClassifierOptions(score_threshold=umbral),
    )
    return vision.GestureRecognizer.create_from_options(opciones)


def girar(img, grados, espejo):
    giros = {90: cv2.ROTATE_90_CLOCKWISE, 180: cv2.ROTATE_180, 270: cv2.ROTATE_90_COUNTERCLOCKWISE}
    if grados in giros:
        img = cv2.rotate(img, giros[grados])
    return cv2.flip(img, 1) if espejo else img


# ------------------------------------------------------------------ pantalla

def texto(img, t, org, escala, color, grosor=2, centro=False):
    (w, h), _ = cv2.getTextSize(t, cv2.FONT_HERSHEY_DUPLEX, escala, grosor)
    x, y = org
    if centro:
        x -= w // 2
    cv2.putText(img, t, (x, y), cv2.FONT_HERSHEY_DUPLEX, escala, color, grosor, cv2.LINE_AA)
    return w, h


def pulgar(img, c, r, arriba, color):
    """Icono (OpenCV no dibuja emojis): flecha arriba = encender, puño (cuadro) = apagar."""
    cv2.circle(img, c, r, color, -1, cv2.LINE_AA)
    x, y = c
    if not arriba:
        a = int(r * 0.45)
        cv2.rectangle(img, (x - a, y - a), (x + a, y + a), FONDO, -1, cv2.LINE_AA)
        return
    s = -1 if arriba else 1                       # -1 = la punta va hacia arriba
    flecha = [(0, 0.6), (-0.45, 0.05), (-0.17, 0.05), (-0.17, -0.55), (0.17, -0.55), (0.17, 0.05), (0.45, 0.05)]
    pts = np.array([(x + dx * r, y + s * dy * r) for dx, dy in flecha], np.int32)
    cv2.fillPoly(img, [pts], FONDO, cv2.LINE_AA)


def recuadro(vid, pts, gesto, score, etiqueta=None):
    """Recuadro de deteccion alrededor de la mano, con el gesto y su confianza."""
    h, w = vid.shape[:2]
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    m = 28
    x0, y0, x1, y1 = max(min(xs) - m, 2), max(min(ys) - m, 2), min(max(xs) + m, w - 3), min(max(ys) + m, h - 3)
    color = {ENCENDER: VERDE, APAGAR: NARANJA}.get(gesto, BLANCO)
    cv2.rectangle(vid, (x0, y0), (x1, y1), color, 2, cv2.LINE_AA)
    L = max(18, min(x1 - x0, y1 - y0) // 4)                       # esquinas gruesas
    for cx, cy, dx, dy in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        cv2.line(vid, (cx, cy), (cx + dx * L, cy), color, 6, cv2.LINE_AA)
        cv2.line(vid, (cx, cy), (cx, cy + dy * L), color, 6, cv2.LINE_AA)
    if etiqueta is None:
        etiqueta = {ENCENDER: "PULGAR ARRIBA", APAGAR: "MANO CERRADA"}.get(gesto, "MANO")
    if gesto and score:
        etiqueta += f"  {score * 100:.0f}%"
    (tw, th), _ = cv2.getTextSize(etiqueta, cv2.FONT_HERSHEY_DUPLEX, 0.7, 2)
    ty = y0 - 10 if y0 - th - 18 > 0 else y1 + th + 16
    cv2.rectangle(vid, (x0, ty - th - 10), (x0 + tw + 16, ty + 8), color, -1)
    cv2.putText(vid, etiqueta, (x0 + 8, ty), cv2.FONT_HERSHEY_DUPLEX, 0.7, FONDO, 2, cv2.LINE_AA)


def aviso_comando(vid, ev):
    """Banda arriba del video cuando se manda el comando a la ESP32 (dura 3 s) + destello del borde."""
    edad = time.monotonic() - ev["t"]
    if edad > 3:
        return
    h, w = vid.shape[:2]
    color = VERDE if ev["encender"] else NARANJA
    if edad < 0.6:
        cv2.rectangle(vid, (0, 0), (w - 1, h - 1), color, 10)
    capa = vid.copy()
    cv2.rectangle(capa, (0, 0), (w, 96), FONDO, -1)
    cv2.addWeighted(capa, 0.78, vid, 0.22, 0, vid)
    accion = "ENCENDER" if ev["encender"] else "APAGAR"
    texto(vid, f"COMANDO: {accion}", (20, 40), 1.0, color, 2)
    texto(vid, f"GET /carga?estado={int(ev['encender'])}", (20, 78), 0.65, BLANCO, 1)
    if ev["estado"] == "enviando":
        estado, c = "enviando...", TENUE
    elif ev["estado"] == "ok":
        estado, c = (f"OK  ESP32 respondio en {ev['ms']} ms" if ev["ms"] else "OK"), VERDE
    else:
        estado, c = "SIN RESPUESTA", NARANJA
    (tw, _), _ = cv2.getTextSize(estado, cv2.FONT_HERSHEY_DUPLEX, 0.7, 2)
    texto(vid, estado, (w - tw - 20, 78), 0.7, c, 2)


def dibujar(img, mano, gesto, avance, carga, conectado, fps, score=0.0, ancho=960, modo="gestos", armado=True):
    h0, w0 = img.shape[:2]
    alto = int(ancho * h0 / w0)
    vid = cv2.resize(img, (ancho, alto))
    if mano:
        pts = [(int(p.x * ancho), int(p.y * alto)) for p in mano]
        for a, b in CONEXIONES:
            cv2.line(vid, pts[a], pts[b], BLANCO, 3, cv2.LINE_AA)
        for p in pts:
            cv2.circle(vid, p, 6, AMBAR, -1, cv2.LINE_AA)
        recuadro(vid, pts, gesto, score, "MANO DETECTADA" if modo == "mano" else None)
    if carga.evento:
        aviso_comando(vid, carga.evento)

    barra, pie = 64, 150
    lienzo = np.full((alto + barra + pie, ancho, 3), FONDO, np.uint8)
    lienzo[barra:barra + alto] = vid

    # barra superior
    texto(lienzo, "TOSTATRONIC", (20, 42), 0.9, AMBAR)
    texto(lienzo, "GESTOS -> ESP32 -> 74LS07 -> IRFZ44N", (230, 40), 0.6, TENUE, 1)
    color = VERDE if conectado else NARANJA
    cv2.circle(lienzo, (ancho - 150, 33), 8, color, -1, cv2.LINE_AA)
    texto(lienzo, f"{fps:4.1f} FPS" if conectado else "SIN VIDEO", (ancho - 130, 40), 0.6, color, 1)

    # gesto detectado
    y = barra + alto
    if modo == "mano" and mano:
        accion = "encender" if gesto == ENCENDER else "apagar"
        c = VERDE if gesto == ENCENDER else NARANJA
        pulgar(lienzo, (70, y + 62), 38, gesto == ENCENDER, c)
        texto(lienzo, "MANO DETECTADA", (125, y + 58), 1.1, c)
        texto(lienzo, f"sostenla para {accion}" if armado else "retira la mano para el siguiente cambio",
              (125, y + 92), 0.7, TENUE, 1)
    elif modo == "mano":
        texto(lienzo, "Muestra tu mano: enciende / apaga", (30, y + 70), 0.9, TENUE, 1)
    elif gesto == ENCENDER:
        pulgar(lienzo, (70, y + 62), 38, True, VERDE)
        texto(lienzo, "PULGAR ARRIBA", (125, y + 58), 1.1, VERDE)
        texto(lienzo, "encender", (125, y + 92), 0.7, TENUE, 1)
    elif gesto == APAGAR:
        pulgar(lienzo, (70, y + 62), 38, False, NARANJA)
        texto(lienzo, "MANO CERRADA", (125, y + 58), 1.1, NARANJA)
        texto(lienzo, "apagar", (125, y + 92), 0.7, TENUE, 1)
    else:
        texto(lienzo, "Muestra tu mano a la camara", (30, y + 70), 0.9, TENUE, 1)

    # barra de confirmacion (hay que sostener la seña un momento)
    if avance > 0:
        cv2.rectangle(lienzo, (30, y + 118), (ancho // 2, y + 130), (60, 50, 42), -1)
        cv2.rectangle(lienzo, (30, y + 118), (30 + int((ancho // 2 - 30) * min(avance, 1)), y + 130),
                      VERDE if gesto == ENCENDER else NARANJA, -1)

    # estado del LED
    cx = ancho - 290
    cv2.circle(lienzo, (cx, y + 75), 42, AMBAR if carga.encendida else (60, 50, 42), -1, cv2.LINE_AA)
    if carga.encendida:
        cv2.circle(lienzo, (cx, y + 75), 54, AMBAR, 3, cv2.LINE_AA)
    texto(lienzo, "LED", (cx + 62, y + 62), 0.7, TENUE, 1)
    texto(lienzo, "ENCENDIDO" if carga.encendida else "APAGADO", (cx + 62, y + 96), 0.8,
          AMBAR if carga.encendida else TENUE, 2)
    if carga.error:
        texto(lienzo, carga.error, (30, y + 145), 0.5, NARANJA, 1)
    return lienzo


# ------------------------------------------------------------------ principal

def main():
    ap = argparse.ArgumentParser(description="Prende la carga de la ESP32 con el pulgar arriba y la apaga con el puño cerrado.")
    ap.add_argument("--cam", default="tostacam.local", help="IP o nombre de la ESP32, opcional :puerto (default: tostacam.local)")
    ap.add_argument("--puerto-stream", type=int, default=81, help="puerto del video MJPEG (default: 81)")
    ap.add_argument("--modo", default="mano", choices=["mano", "gestos"],
                    help="mano: cada aparicion de la mano cambia el estado (default); gestos: pulgar arriba / puño")
    ap.add_argument("--fuente", help="probar sin ESP32: indice de webcam (0) o archivo de video")
    ap.add_argument("--simular", action="store_true", help="no manda nada a la ESP32 (solo prueba)")
    ap.add_argument("--umbral", type=float, default=0.6, help="confianza minima del gesto (0-1)")
    ap.add_argument("--sostener", type=float, default=0.5, help="segundos que hay que sostener la seña")
    ap.add_argument("--girar", type=int, default=0, choices=[0, 90, 180, 270], help="rota la imagen de la camara")
    ap.add_argument("--espejo", action="store_true", help="voltea la imagen como espejo")
    ap.add_argument("--sin-ventana", action="store_true", help="sin ventana (solo consola)")
    a = ap.parse_args()

    host = a.cam.split(":")[0]
    lector = LectorLocal(a.fuente) if a.fuente is not None else LectorMJPEG(f"http://{host}:{a.puerto_stream}/stream")
    lector.start()
    carga = Carga(a.cam, a.simular)
    reconocedor = crear_reconocedor(a.umbral)

    candidato, desde, ultimo_ts = None, 0.0, 0
    # modo mano: la mano debe verse `sostener` s; luego hay que retirarla `SOLTAR` s para el siguiente cambio
    SOLTAR, HUECO = 0.8, 0.3
    armado, vista_desde, ultima_vista = True, None, -1e9
    fps, t_fps, n_fps = 0.0, time.time(), 0
    if a.modo == "mano":
        print("Listo. Muestra la mano = cambia encendido/apagado; retirala para el siguiente cambio.  (Q para salir)")
    else:
        print("Listo. Pulgar arriba = encender, puño cerrado = apagar.  (Q para salir)")

    while True:
        img = lector.leer()
        if img is None:
            if not a.sin_ventana:
                espera = np.full((540, 960, 3), FONDO, np.uint8)
                texto(espera, "Esperando el video de la ESP32...", (480, 270), 1, TENUE, 1, centro=True)
                cv2.imshow("Tostatronic - Gestos", espera)
                if cv2.waitKey(100) & 0xFF in (ord("q"), 27):
                    break
            else:
                time.sleep(0.1)
            continue

        img = girar(img, a.girar, a.espejo)
        rgb = np.ascontiguousarray(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
        ts = max(int(time.monotonic() * 1000), ultimo_ts + 1)     # MediaPipe exige tiempos crecientes
        ultimo_ts = ts
        res = reconocedor.recognize_for_video(mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb), ts)

        gesto, mano, score = None, None, 0.0
        if res.hand_landmarks:
            mano = res.hand_landmarks[0]
            if res.gestures and res.gestures[0]:
                g = res.gestures[0][0]
                if g.category_name in (ENCENDER, APAGAR) and g.score >= a.umbral:
                    gesto, score = g.category_name, g.score

        ahora = time.monotonic()
        if a.modo == "mano":
            # Cada aparicion de la mano cambia el estado. Se toleran huecos cortos de la deteccion.
            if mano:
                ultima_vista = ahora
            presente = ahora - ultima_vista < HUECO
            gesto = APAGAR if carga.encendida else ENCENDER      # lo que va a pasar (colores y textos)
            avance = 0
            if presente and armado:
                vista_desde = vista_desde or ahora
                avance = (ahora - vista_desde) / a.sostener
                if avance >= 1:
                    quiere = not carga.encendida
                    carga.poner(quiere)
                    carga.encendida = quiere
                    armado, vista_desde = False, None
            elif not presente:
                vista_desde = None
                if not armado and ahora - ultima_vista >= SOLTAR:
                    armado = True
        else:
            # La seña tiene que sostenerse: un pulgar a medio movimiento no dispara nada.
            if gesto != candidato:
                candidato, desde = gesto, ahora
            avance = (ahora - desde) / a.sostener if gesto else 0
            quiere = {ENCENDER: True, APAGAR: False}.get(gesto)
            if quiere is not None and avance >= 1 and carga.encendida != quiere:
                carga.poner(quiere)
                carga.encendida = quiere          # optimista; la respuesta de la ESP32 lo confirma

        n_fps += 1
        if time.time() - t_fps >= 1:
            fps, n_fps, t_fps = n_fps / (time.time() - t_fps), 0, time.time()

        if a.sin_ventana:
            continue
        cv2.imshow("Tostatronic - Gestos", dibujar(img, mano, gesto, avance, carga, lector.conectado, fps, score,
                                                   modo=a.modo, armado=armado))
        tecla = cv2.waitKey(1) & 0xFF
        if tecla in (ord("q"), 27):
            break
        if tecla == ord("e"):
            carga.poner(True)
        if tecla == ord("a"):
            carga.poner(False)

    lector.vivo = False
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()

"""
Servicio de gestos - Tostatronic (www.tostatronic.com)
=======================================================

Lee el video de la ESP32-WROVER-CAM por WiFi, reconoce tus señas con
MediaPipe y prende o apaga la carga del circuito:

    pulgar arriba  ->  GET http://<camara>/carga?estado=1  ->  LED encendido
    pulgar abajo   ->  GET http://<camara>/carga?estado=0  ->  LED apagado

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

ENCENDER, APAGAR = "Thumb_Up", "Thumb_Down"

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
        threading.Thread(target=self._pedir, args=(None,), daemon=True).start()   # estado inicial

    def poner(self, encender):
        if self.simular:
            self.encendida = encender
            print(f"[carga] (simulada) {'ENCENDIDA' if encender else 'apagada'}")
            return
        threading.Thread(target=self._pedir, args=(encender,), daemon=True).start()

    def _pedir(self, encender):
        if self.simular:
            return
        url = self.base + "/carga" + ("" if encender is None else f"?estado={int(encender)}")
        try:
            with urllib.request.urlopen(url, timeout=2) as r:
                self.encendida = b"true" in r.read()
                self.error = ""
            if encender is not None:
                print(f"[carga] {'ENCENDIDA' if self.encendida else 'apagada'}")
        except Exception as e:
            self.error = "sin respuesta de la ESP32"
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
        min_hand_detection_confidence=0.5,
        min_hand_presence_confidence=0.5,
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
    """Icono: circulo con flecha arriba/abajo (OpenCV no dibuja emojis)."""
    cv2.circle(img, c, r, color, -1, cv2.LINE_AA)
    x, y = c
    s = -1 if arriba else 1                       # -1 = la punta va hacia arriba
    flecha = [(0, 0.6), (-0.45, 0.05), (-0.17, 0.05), (-0.17, -0.55), (0.17, -0.55), (0.17, 0.05), (0.45, 0.05)]
    pts = np.array([(x + dx * r, y + s * dy * r) for dx, dy in flecha], np.int32)
    cv2.fillPoly(img, [pts], FONDO, cv2.LINE_AA)


def dibujar(img, mano, gesto, avance, carga, conectado, fps, ancho=960):
    h0, w0 = img.shape[:2]
    alto = int(ancho * h0 / w0)
    vid = cv2.resize(img, (ancho, alto))
    if mano:
        pts = [(int(p.x * ancho), int(p.y * alto)) for p in mano]
        for a, b in CONEXIONES:
            cv2.line(vid, pts[a], pts[b], BLANCO, 3, cv2.LINE_AA)
        for p in pts:
            cv2.circle(vid, p, 6, AMBAR, -1, cv2.LINE_AA)

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
    if gesto == ENCENDER:
        pulgar(lienzo, (70, y + 62), 38, True, VERDE)
        texto(lienzo, "PULGAR ARRIBA", (125, y + 58), 1.1, VERDE)
        texto(lienzo, "encender", (125, y + 92), 0.7, TENUE, 1)
    elif gesto == APAGAR:
        pulgar(lienzo, (70, y + 62), 38, False, NARANJA)
        texto(lienzo, "PULGAR ABAJO", (125, y + 58), 1.1, NARANJA)
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
    ap = argparse.ArgumentParser(description="Prende y apaga la carga de la ESP32 con pulgar arriba / abajo.")
    ap.add_argument("--cam", default="tostacam.local", help="IP o nombre de la ESP32, opcional :puerto (default: tostacam.local)")
    ap.add_argument("--puerto-stream", type=int, default=81, help="puerto del video MJPEG (default: 81)")
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
    fps, t_fps, n_fps = 0.0, time.time(), 0
    print("Listo. Pulgar arriba = encender, pulgar abajo = apagar.  (Q para salir)")

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

        gesto, mano = None, None
        if res.hand_landmarks:
            mano = res.hand_landmarks[0]
            if res.gestures and res.gestures[0]:
                g = res.gestures[0][0]
                if g.category_name in (ENCENDER, APAGAR) and g.score >= a.umbral:
                    gesto = g.category_name

        # La seña tiene que sostenerse: un pulgar a medio movimiento no dispara nada.
        ahora = time.monotonic()
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
        cv2.imshow("Tostatronic - Gestos", dibujar(img, mano, gesto, avance, carga, lector.conectado, fps))
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

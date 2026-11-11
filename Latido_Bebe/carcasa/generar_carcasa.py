"""generar_carcasa.py - Carcasa de "Latido" para imprimir en 3D.
Desarrollado por Tostatronic - Ing. Jorge Alvarado

Torre de 59 x 96 x 41 mm que se para sola. Al frente: la pantalla redonda arriba y la bocina abajo, detras de
una rejilla en forma de corazon; cada una se aprieta por atras con un puente. En la tapa trasera van el holder
de la 18650 (parado, a la izquierda, dentro de su marco) y, en columna a la derecha, el TP4056 (en rieles), la
ESP32-C3 Super Mini con sus pines soldados (sobre un pedestal, con una grapa) y el MAX98357A (atornillado).
Los dos USB-C salen por el costado. Todo el interior es la caja acustica de la bocina.

Tornillos: M4 x 16 con TUERCA M4 ATRAPADA en la tapa (3), los puentes (2) y la grapa de la C3 (1); M3 en el holder
y el amplificador. Las tuercas se meten en una pausa de la impresion: las alturas salen en validacion_carcasa.json.

Coordenadas: X = ancho, Y = alto, Z = fondo (0 = cara del frente, FONDO = espalda), viendo el aparato DE FRENTE.
Las cotas ix / iy son interiores (desde la esquina inferior izquierda del hueco).

Uso:  python generar_carcasa.py   -> STL, vistas y validacion en esta carpeta
Requiere: numpy, manifold3d, trimesh, matplotlib
"""
import json
from pathlib import Path
import numpy as np
import manifold3d as md
import trimesh
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle

O = Path(__file__).parent
M = md.Manifold

# ============================================================
#  MEDIDAS DE LAS PIEZAS  (las marcadas (*) son tipicas del modulo: CONFIRMALAS CON VERNIER)
# ============================================================
HOLDER = (20.6, 75.8, 21.3)    # holder 18650 de 1 pila: ancho y largo del marco ya probado en la bascula (76.0 x 20.8 con
                               # 0.1 de holgura por lado); (*) el alto es el de la ficha
HOLDER_BARRENO = 55.61         # ficha: separacion entre los dos barrenos de 3.2 mm (ademas va uno al centro)
HOLDER_PIN = 2.4               # las terminales salen a 2.4 mm de cada orilla y bajan 3.3 mm
BOCINA_D = 40.0                # bocina 8 ohms 0.5 W
BOCINA_H = 6.0                 # alto total de la bocina
LCD_VENTANA = 33.2             # area activa de 32.4 mm + holgura
LCD_CUERPO_D = 35.6            # (*) vidrio de la pantalla
LCD_PCB_D = 38.0               # (*) tarjeta redonda (hasta este diametro cabe)
LCD_GRUESO = 5.2               # (*) del vidrio a la espalda de la tarjeta
LCD_OREJA = (19.0, 26.0)       # (*) pestana de los pines: ancho, y cuanto sube desde el centro
LCD_PINES = (20.5, 8.5)        # (*) fila de pines: distancia al centro, y cuanto salen hacia atras
AMP = (18.0, 19.0)             # MAX98357A
AMP_BARRENOS = (12.6, 2.7)     # (*) sus dos barrenos: separacion, y distancia a la orilla de arriba
TP4056 = (27.9, 17.2)          # TP4056 tipo C: tarjeta medida en el encapsulado de la bascula (cavidad probada 28.3 x 17.6)
C3 = (22.5, 18.0)              # ESP32-C3 Super Mini (largo con el USB hacia la pared)
C3_PINES = 9.2                 # la C3 va CON pines soldados: lo que cuelgan debajo de la tarjeta (2.5 de plastico + 6 de pin)
SW_CORTE = (19.4, 13.2)        # switch balancin 117S (15 x 21): corte del panel, para pared de 0.8 a 3.0 mm
SW_FONDO = (13.5, 20.5)        # cuerpo detras del marco, y total hasta la punta de las terminales

# --- tornilleria
M4_PASO, M4_CABEZA = 4.6, (7.2, 4.0)      # M4 x 16 de cabeza cilindrica o redonda
TUERCA = (7.4, 3.6)                       # hueco de la tuerca M4 (7 entre caras x 3.2 de alto): entre caras, alto
CAPA = 0.2                                # altura de capa, para decir en que capa va cada pausa
M4_LARGO = 16.0
M3_PILOTO = 2.5

# ============================================================
#  CAJA
# ============================================================
PARED = 2.4
FRENTE = 2.4
TAPA = 3.0
IN_W, IN_H = 54.5, 91.0        # hueco interior
FONDO = 41.0                   # fondo total con la tapa puesta
R_EXT = 4.4                    # radio de las esquinas
HOLG = 0.25                    # holgura de ajuste

W, H = IN_W + 2 * PARED, IN_H + 2 * PARED
Z_TAPA = FONDO - TAPA          # cara interior de la tapa

# --- frente
BOC = (IN_W / 2, 23.5)                     # centro de la bocina (ix, iy)
LCD = (IN_W / 2, IN_H - LCD_OREJA[1] - 1)  # centro de la pantalla
PUENTE = (8.0, 3.0, 10.2)                  # puentes: ancho, grueso de la barra, alto del bloque del tornillo
PUENTE_X = IN_W - 3.7                      # su tornillo M4 va pegado a la pared derecha; la otra punta entra en la pared izquierda
Z_TUERCA_PUENTE = FRENTE + 0.2             # las tuercas de los puentes quedan casi sobre la cara del frente
#          iy del puente, z donde asienta
P_LCD = (LCD[1] + 6.0, FRENTE + LCD_GRUESO + 1.0)   # 1 mm libre sobre la tarjeta: ahi va cinta de espuma
P_BOC = (BOC[1] - 6.0, FRENTE + BOCINA_H + 0.3)     # 6 mm abajo del centro: todavia pisa el iman

# --- tapa (posiciones interiores, vistas de frente)
PEDESTAL = 2.0                             # el holder va levantado para librar sus terminales
MARCO = (1.6, 5.0)                         # "casita" del holder: grueso y alto de sus paredes
HOLDER_XY = (2.2, 2.0)                     # esquina inferior izquierda del holder
TP_Y, C3_Y, AMP_Y = 12.75, 34.4, 62.8      # columna derecha, de abajo hacia arriba
AMP_X = 26.0
AMP_PISO = 3.0                             # postes del amplificador
CUNA = (1.2, 2.7)                          # rieles del TP4056: grueso, alto
PISO_CUNA = 1.0                            # costillas donde asienta el TP4056
USB = (12.6, 7.0)                          # ventana del USB-C: ancho (en Y), alto (en Z)
USB_Z_TP = Z_TAPA - PISO_CUNA - 1.6 - 1.6  # centro del conector del TP4056
Z_C3 = Z_TAPA - C3_PINES                   # cara de abajo de la tarjeta de la C3
USB_Z_C3 = Z_C3 - 1.6 - 1.6
Z_GRAPA = Z_C3 - 1.6 - 3.2 - 0.2           # la grapa de la C3 pasa justo sobre su conector USB
GRAPA_X = 50.0                             # tornillo de la grapa
GRAPA_GANCHO = (C3_Y - 2.8, C3_Y - 0.3)    # abajo de la C3: gancho donde entra la punta de la grapa
GRAPA_POSTE = (C3_Y + C3[1] + 0.3, C3_Y + C3[1] + 10.1)   # arriba: poste con la tuerca
Z_TUERCA_GRAPA = Z_GRAPA + 6.2
R_POSTE = 5.5                              # postes de la tapa (llevan tuerca)
Z_TUERCA_TAPA = FONDO - 1.6 - M4_LARGO + 3.6   # la punta del tornillo pasa 3.6 mm de la tuerca
POSTES = [(R_POSTE + 0.2, IN_H - R_POSTE - 0.2), (IN_W - R_POSTE - 0.2, IN_H - R_POSTE - 0.2), (IN_W - R_POSTE - 0.2, R_POSTE + 0.2)]
SW = (32.0, 23.0)                              # switch en la pared de arriba: centro (ix, z)


# ============================================================
#  AYUDAS
# ============================================================
def caja(ix0, ix1, iy0, iy1, z0, z1):
    return M.cube([ix1 - ix0, iy1 - iy0, z1 - z0]).translate([PARED + ix0, PARED + iy0, z0])


def cil(ix, iy, r, z0, z1, r1=None, seg=96):
    return M.cylinder(z1 - z0, r, r if r1 is None else r1, circular_segments=seg).translate([PARED + ix, PARED + iy, z0])


def redondeado(x0, y0, x1, y1, r, z0, z1, n=16):
    pts = []
    for cx, cy, a0 in ((x1 - r, y1 - r, 0), (x0 + r, y1 - r, 90), (x0 + r, y0 + r, 180), (x1 - r, y0 + r, 270)):
        for a in np.radians(np.linspace(a0, a0 + 90, n)):
            pts.append((cx + r * np.cos(a), cy + r * np.sin(a)))
    return md.CrossSection([np.array(pts)], md.FillRule.Positive).extrude(z1 - z0).translate([0, 0, z0])


def hexa(ix, iy, z0, z1):
    """hueco hexagonal de una tuerca M4, con dos caras paralelas a las paredes laterales"""
    r = TUERCA[0] / 2 / np.cos(np.pi / 6)
    pts = np.array([(r * np.cos(a), r * np.sin(a)) for a in np.radians(np.arange(30, 390, 60))])
    return md.CrossSection([pts], md.FillRule.Positive).extrude(z1 - z0).translate([PARED + ix, PARED + iy, z0])


def corazon():
    """centros de los barrenos de la rejilla: malla hexagonal recortada con un corazon"""
    paso, esc, pts = 3.3, 13.5, []
    for j in range(-8, 9):
        for i in range(-8, 9):
            x, y = (i + 0.5 * (j % 2)) * paso, j * paso * 0.866
            u, v = x / esc, (y + 1.5) / esc
            if (u * u + v * v - 1) ** 3 - u * u * v ** 3 <= 0 and np.hypot(x, y) <= BOCINA_D / 2 - 2.6:
                pts.append((x, y))
    return pts


# ============================================================
#  FRENTE (cara + 4 paredes)
# ============================================================
def frente():
    s = redondeado(0, 0, W, H, R_EXT, 0, Z_TAPA) - redondeado(PARED, PARED, W - PARED, H - PARED, R_EXT - PARED, FRENTE, FONDO)
    # anillo de asiento de la bocina
    s += cil(*BOC, BOCINA_D / 2 + HOLG + 1.4, FRENTE - 0.1, FRENTE + 3.0)
    # puentes de la pantalla y la bocina: poste del tornillo M4 junto a la pared derecha
    for py, z in (P_LCD, P_BOC):
        s += caja(PUENTE_X - 4.5, IN_W, py - 5.0, py + 5.0, FRENTE - 0.1, z)
    # postes de los tornillos de la tapa
    for px, py in POSTES:
        s += cil(px, py, R_POSTE, FRENTE - 0.1, Z_TAPA) + \
            caja(0 if px < IN_W / 2 else px, px if px < IN_W / 2 else IN_W, py - R_POSTE, py + R_POSTE, FRENTE - 0.1, Z_TAPA) + \
            caja(px - R_POSTE, px + R_POSTE, 0 if py < IN_H / 2 else py, py if py < IN_H / 2 else IN_H, FRENTE - 0.1, Z_TAPA)
        s -= cil(px, py, M4_PASO / 2, Z_TUERCA_TAPA - 4.5, Z_TAPA + 1) + hexa(px, py, Z_TUERCA_TAPA, Z_TUERCA_TAPA + TUERCA[1])
    s = s ^ redondeado(0, 0, W, H, R_EXT, 0, Z_TAPA)          # nada sale del contorno
    # pantalla: ventana con chaflan hacia afuera
    s -= cil(*LCD, LCD_VENTANA / 2, -1, FRENTE + 1)
    s -= cil(*LCD, LCD_VENTANA / 2 + 1.2, 0, 1.2, r1=LCD_VENTANA / 2)
    # bocina: su hueco y la rejilla de corazon
    s -= cil(*BOC, BOCINA_D / 2 + HOLG, FRENTE, FRENTE + BOCINA_H + 3)
    for x, y in corazon():
        s -= cil(BOC[0] + x, BOC[1] + y, 1.2, -1, FRENTE + 1, seg=24)
    # puentes: piloto del tornillo y, en la pared izquierda, la bolsa donde entra la punta
    for py, z in (P_LCD, P_BOC):
        s -= cil(PUENTE_X, py, M4_PASO / 2, 2.0, z + 1) + hexa(PUENTE_X, py, Z_TUERCA_PUENTE, Z_TUERCA_PUENTE + TUERCA[1])
        s -= caja(-1.4, 0.2, py - PUENTE[0] / 2 - 0.3, py + PUENTE[0] / 2 + 0.3, z - 0.2, z + PUENTE[1] + 0.5)
    # ventanas de los USB-C (pared derecha, abiertas hacia la tapa)
    for y0, ancho, zc in ((TP_Y, TP4056[1], USB_Z_TP), (C3_Y, C3[1], USB_Z_C3)):
        yc = y0 + ancho / 2
        s -= caja(IN_W - 0.5, IN_W + PARED + 1, yc - USB[0] / 2, yc + USB[0] / 2, zc - USB[1] / 2, FONDO)
    # switch balancin 117S (pared de arriba): entra a presion por fuera, lo largo a lo ancho del aparato
    s -= caja(SW[0] - SW_CORTE[0] / 2, SW[0] + SW_CORTE[0] / 2, IN_H - 0.5, IN_H + PARED + 1, SW[1] - SW_CORTE[1] / 2, SW[1] + SW_CORTE[1] / 2)
    return s


def puente(py, z):
    """barra que aprieta por atras: la punta izquierda entra en la pared y el bloque derecho lleva un M4 x 16"""
    A, G, B = PUENTE
    p = caja(-1.2, IN_W - 0.3, py - A / 2, py + A / 2, z, z + G) + caja(PUENTE_X - 4.5, IN_W - 0.3, py - A / 2, py + A / 2, z, z + B)
    return p - cil(PUENTE_X, py, M4_PASO / 2, z - 1, z + B + 1)


# ============================================================
#  TAPA TRASERA (lleva el holder y las tres tarjetas)
# ============================================================
def tapa():
    Z = Z_TAPA + 0.1
    t = redondeado(0, 0, W, H, R_EXT, Z_TAPA, FONDO)
    # ceja que entra en el frente y centra la tapa
    c = redondeado(PARED + HOLG, PARED + HOLG, W - PARED - HOLG, H - PARED - HOLG, R_EXT - PARED - HOLG, Z_TAPA - 3, Z) - \
        redondeado(PARED + HOLG + 1.6, PARED + HOLG + 1.6, W - PARED - HOLG - 1.6, H - PARED - HOLG - 1.6, 0.6, Z_TAPA - 4, Z_TAPA + 1)
    for px, py in POSTES:
        c -= caja(px - R_POSTE - 0.5, px + R_POSTE + 0.5, py - R_POSTE - 0.5, py + R_POSTE + 0.5, 0, FONDO)
        c -= caja(0 if px < IN_W / 2 else px, px if px < IN_W / 2 else IN_W, py - R_POSTE - 0.5, py + R_POSTE + 0.5, 0, FONDO)
        c -= caja(px - R_POSTE - 0.5, px + R_POSTE + 0.5, 0 if py < IN_H / 2 else py, py if py < IN_H / 2 else IN_H, 0, FONDO)
    c -= caja(IN_W - 3, IN_W, TP_Y - 2, TP_Y + TP4056[1] + 2, 0, FONDO)          # frente a los USB no hay ceja
    c -= caja(IN_W - 3, IN_W, C3_Y - 2, C3_Y + C3[1] + 2, 0, FONDO)
    t += c

    # --- holder: pedestal + "casita" que lo abraza, con dos canales para sus terminales y los cables
    hx, hy = HOLDER_XY
    g, h = MARCO
    zp = Z_TAPA - PEDESTAL
    marco = caja(hx - 0.1 - g, hx + HOLDER[0] + 0.1 + g, hy - 0.1 - g, hy + HOLDER[1] + 0.1 + g, zp - h, Z) - \
        caja(hx - 0.1, hx + HOLDER[0] + 0.1, hy - 0.1, hy + HOLDER[1] + 0.1, zp - h - 1, zp)
    t += marco ^ redondeado(PARED + HOLG, PARED + HOLG, W - PARED - HOLG, H - PARED - HOLG, R_EXT - PARED - HOLG, 0, FONDO)   # la esquina sigue el redondeo de la caja
    for y in (hy + HOLDER_PIN, hy + HOLDER[1] - HOLDER_PIN):
        t -= caja(hx + 3, hx + HOLDER[0] + 3, y - 2.5, y + 2.5, zp - h - 1, Z_TAPA + 1.8)
    for y in (hy + HOLDER[1] / 2, hy + HOLDER[1] / 2 - HOLDER_BARRENO / 2, hy + HOLDER[1] / 2 + HOLDER_BARRENO / 2):
        t -= cil(hx + HOLDER[0] / 2, y, M3_PILOTO / 2, zp - 1, FONDO - 0.8)

    # --- TP4056: entra deslizando desde la orilla (antes de cerrar), bajo dos rieles; atras tiene tope
    x0, y0, (L, A) = IN_W - TP4056[0], TP_Y, TP4056
    g, h = CUNA
    zs = Z_TAPA - PISO_CUNA                     # donde asienta la tarjeta
    t += caja(x0 + 2, x0 + 4, y0, y0 + A, zs, Z) + caja(x0 + L - 6, x0 + L - 4, y0, y0 + A, zs, Z)
    t += caja(x0 - HOLG - g, x0 - HOLG, y0 - HOLG - g, y0 + A + HOLG + g, zs - 1.6, Z)          # tope: a ras de la tarjeta, los cables pasan por encima
    for s in (-1, 1):
        ya = y0 - HOLG - g if s < 0 else y0 + A + HOLG
        t += caja(x0 - HOLG - g, x0 + L - 4, ya, ya + g, zs - h, Z)
        yl = y0 - HOLG - 0.2 if s < 0 else y0 + A + HOLG - 0.9                                # riel: 0.65 mm sobre la orilla, solo al centro
        t += caja(x0 + 8, x0 + L - 8, yl, yl + 1.1, zs - h, zs - 1.6 - 0.3)

    # --- ESP32-C3 con pines soldados: pedestal entre las dos filas de pines, tope atras y postes de la grapa
    x0, y0, (L, A) = IN_W - C3[0], C3_Y, C3
    yc = y0 + A / 2
    t += caja(x0 - HOLG - 1.6, x0 + L - 3, yc - 6.0, yc + 6.0, Z_C3, Z)
    t += caja(x0 - HOLG - 1.6, x0 - HOLG, yc - 6.0, yc + 6.0, Z_C3 - 2.6, Z_C3 + 0.1)
    # gancho (abajo) y poste con tuerca atrapada (arriba) para la grapa
    t += caja(45.8, IN_W - HOLG, GRAPA_GANCHO[0], GRAPA_GANCHO[1], Z_GRAPA - 5.3, Z) - \
        caja(47.0, IN_W - HOLG - 1.2, GRAPA_GANCHO[0] - 1, GRAPA_GANCHO[1] + 1, Z_GRAPA - 3.3, Z_GRAPA + 0.3)
    t += caja(GRAPA_X - 5.0, IN_W - HOLG, GRAPA_POSTE[0], GRAPA_POSTE[1], Z_GRAPA, Z)
    gy = sum(GRAPA_POSTE) / 2
    t -= cil(GRAPA_X, gy, M4_PASO / 2, Z_GRAPA - 1, Z_GRAPA + M4_LARGO - 2.6) + hexa(GRAPA_X, gy, Z_TUERCA_GRAPA, Z_TUERCA_GRAPA + TUERCA[1])
    # relleno de la ventana del USB de la C3 (la ventana del frente llega hasta la tapa)
    t += caja(IN_W + HOLG, IN_W + PARED, yc - USB[0] / 2 + HOLG, yc + USB[0] / 2 - HOLG, USB_Z_C3 + USB[1] / 2 + 0.3, Z)

    # --- amplificador: dos postes M3 en sus barrenos y una costilla del lado de los pines
    ax, ay = AMP_X, AMP_Y
    za = Z_TAPA - AMP_PISO
    for bx in (ax + AMP[0] / 2 - AMP_BARRENOS[0] / 2, ax + AMP[0] / 2 + AMP_BARRENOS[0] / 2):
        t += cil(bx, ay + AMP[1] - AMP_BARRENOS[1], 2.8, za, Z)
        t -= cil(bx, ay + AMP[1] - AMP_BARRENOS[1], M3_PILOTO / 2, za - 1, FONDO - 0.8)
    t += caja(ax, ax + AMP[0], ay + 3.4, ay + 5.0, za, Z)
    t += caja(ax - HOLG - 1.2, ax - HOLG, ay, ay + 8, za - 1.6, Z) + caja(ax + AMP[0] + HOLG, ax + AMP[0] + HOLG + 1.2, ay, ay + 8, za - 1.6, Z)

    # --- tornillos M4 de la tapa (cabeza embutida 1.6 mm)
    for px, py in POSTES:
        t -= cil(px, py, M4_PASO / 2, Z_TAPA - 5, FONDO + 1)
        t -= cil(px, py, M4_CABEZA[0] / 2 + 0.5, FONDO - 1.6, FONDO + 1)
    return t


def grapa():
    """barra que cruza sobre el conector USB de la C3: la punta entra en el gancho y del otro lado lleva un M4 x 16"""
    gp = caja(47.3, IN_W - HOLG - 1.5, GRAPA_GANCHO[0] + 0.3, GRAPA_POSTE[1] - 0.2, Z_GRAPA - 3.0, Z_GRAPA) + \
        caja(GRAPA_X - 4.5, IN_W - 0.5, GRAPA_POSTE[0] + 0.2, GRAPA_POSTE[1] - 0.2, Z_GRAPA - 3.0, Z_GRAPA)
    return gp - cil(GRAPA_X, sum(GRAPA_POSTE) / 2, M4_PASO / 2, Z_GRAPA - 4, Z_GRAPA + 1)


# ============================================================
#  COMPONENTES (bultos para revisar que nada choque)
# ============================================================
def componentes():
    hx, hy = HOLDER_XY
    zs, za = Z_TAPA - PISO_CUNA, Z_TAPA - AMP_PISO
    tx, cx = IN_W - TP4056[0], IN_W - C3[0]

    def cabeza(ix, iy, z):
        return cil(ix, iy, M4_CABEZA[0] / 2, z, z + M4_CABEZA[1])
    return {
        'holder': caja(hx, hx + HOLDER[0], hy, hy + HOLDER[1], Z_TAPA - PEDESTAL - HOLDER[2], Z_TAPA - PEDESTAL),
        'bocina': cil(*BOC, BOCINA_D / 2, FRENTE, FRENTE + BOCINA_H),
        'pantalla': cil(*LCD, LCD_CUERPO_D / 2, FRENTE, FRENTE + LCD_GRUESO - 1.6) + cil(*LCD, LCD_PCB_D / 2, FRENTE + LCD_GRUESO - 1.6, FRENTE + LCD_GRUESO)
                    + caja(LCD[0] - LCD_OREJA[0] / 2, LCD[0] + LCD_OREJA[0] / 2, LCD[1], LCD[1] + LCD_OREJA[1], FRENTE + LCD_GRUESO - 1.6, FRENTE + LCD_GRUESO)
                    + caja(LCD[0] - 10.2, LCD[0] + 10.2, LCD[1] + LCD_PINES[0] - 1.3, LCD[1] + LCD_PINES[0] + 1.3, FRENTE + LCD_GRUESO, FRENTE + LCD_GRUESO + LCD_PINES[1]),
        'tp4056': caja(tx, IN_W, TP_Y, TP_Y + TP4056[1], zs - 1.6, zs) + caja(tx, IN_W, TP_Y + 1.0, TP_Y + TP4056[1] - 1.0, zs - 1.6 - 3.3, zs - 1.6),
        'c3': caja(cx, IN_W, C3_Y, C3_Y + C3[1], Z_C3 - 1.6 - 3.2, Z_C3)
              + caja(cx + 1, cx + 21.4, C3_Y, C3_Y + 2.6, Z_C3, Z_TAPA - 0.1) + caja(cx + 1, cx + 21.4, C3_Y + C3[1] - 2.6, C3_Y + C3[1], Z_C3, Z_TAPA - 0.1),
        'amp': caja(AMP_X, AMP_X + AMP[0], AMP_Y, AMP_Y + AMP[1], za - 1.6 - 3.0, za),
        'switch': caja(SW[0] - SW_CORTE[0] / 2, SW[0] + SW_CORTE[0] / 2, IN_H + PARED - SW_FONDO[0], IN_H + PARED, SW[1] - SW_CORTE[1] / 2, SW[1] + SW_CORTE[1] / 2)
                  + caja(SW[0] - 6, SW[0] + 6, IN_H + PARED - SW_FONDO[1] + 2, IN_H + PARED - SW_FONDO[0], SW[1] - 3, SW[1] + 3),
        'tornillos_puentes': cabeza(PUENTE_X, P_LCD[0], P_LCD[1] + PUENTE[2]) + cabeza(PUENTE_X, P_BOC[0], P_BOC[1] + PUENTE[2]),
        'tornillo_grapa': cabeza(GRAPA_X, sum(GRAPA_POSTE) / 2, Z_GRAPA - 3.0 - M4_CABEZA[1]),
    }


def malla(m):
    mm = m.to_mesh()
    return trimesh.Trimesh(vertices=np.asarray(mm.vert_properties)[:, :3], faces=np.asarray(mm.tri_verts), process=False)


# ============================================================
#  VISTAS
# ============================================================
def vistas(comp):
    fig, ax = plt.subplots(1, 3, figsize=(15, 8.2))
    for a, t in zip(ax, ('Frente', 'Interior del frente (visto desde atras)', 'Tapa trasera (vista desde adentro)')):
        a.set_aspect('equal'); a.set_title(t); a.set_xlim(-3, W + 3); a.set_ylim(-3, H + 3); a.axis('off')
        a.add_patch(Rectangle((0, 0), W, H, fill=False, lw=1.6))
    P = PARED
    ax[0].add_patch(Circle((P + LCD[0], P + LCD[1]), LCD_VENTANA / 2, fc='#223', ec='k'))
    for x, y in corazon():
        ax[0].add_patch(Circle((P + BOC[0] + x, P + BOC[1] + y), 1.2, fc='#d33', ec='none'))
    ax[0].add_patch(Circle((P + BOC[0], P + BOC[1]), BOCINA_D / 2, fill=False, ls=':', ec='gray'))
    ax[0].text(W / 2, H + 0.6, 'switch 117S arriba', ha='center', fontsize=8)
    ax[0].text(W + 0.5, P + C3_Y + 9, 'USB C3', rotation=90, va='center', fontsize=8)
    ax[0].text(W + 0.5, P + TP_Y + 9, 'USB carga', rotation=90, va='center', fontsize=8)

    def esp(x):          # visto desde atras, X se voltea
        return W - x
    a = ax[1]
    a.add_patch(Rectangle((P, P), IN_W, IN_H, fill=False, ec='gray'))
    a.add_patch(Circle((esp(P + LCD[0]), P + LCD[1]), LCD_PCB_D / 2, fc='#9cf', ec='k'))
    a.add_patch(Rectangle((esp(P + LCD[0] + LCD_OREJA[0] / 2), P + LCD[1]), LCD_OREJA[0], LCD_OREJA[1], fc='#9cf', ec='k'))
    a.add_patch(Circle((esp(P + BOC[0]), P + BOC[1]), BOCINA_D / 2, fc='#ccc', ec='k'))
    for py, _ in (P_LCD, P_BOC):
        a.add_patch(Rectangle((esp(P + IN_W), P + py - PUENTE[0] / 2), IN_W, PUENTE[0], fc='#fc6', ec='k', alpha=.8))
        a.add_patch(Circle((esp(P + PUENTE_X), P + py), 2.2, fc='w', ec='k'))
    for px, py in POSTES:
        a.add_patch(Circle((esp(P + px), P + py), R_POSTE, fc='#888', ec='k'))
    a.add_patch(Rectangle((esp(P + SW[0] + SW_CORTE[0] / 2), P + IN_H + PARED - SW_FONDO[0]), SW_CORTE[0], SW_FONDO[0], fc='#6c6', ec='k', alpha=.8))
    a.text(esp(P + SW[0]), P + IN_H - 4, 'switch 117S', ha='center', fontsize=7)
    a.text(esp(P + LCD[0]), P + LCD[1] - 9, 'pantalla', ha='center'); a.text(esp(P + BOC[0]), P + BOC[1] - 9, 'bocina', ha='center')
    a.text(esp(P + LCD[0]), P + P_LCD[0], 'puente (M4)', ha='center', va='center', fontsize=7)
    a.text(esp(P + BOC[0]), P + P_BOC[0], 'puente (M4)', ha='center', va='center', fontsize=7)
    a = ax[2]
    a.add_patch(Rectangle((P, P), IN_W, IN_H, fill=False, ec='gray'))
    hx, hy = HOLDER_XY
    a.add_patch(Rectangle((P + hx - 1.7, P + hy - 1.7), HOLDER[0] + 3.4, HOLDER[1] + 3.4, fc='#b97', ec='k'))
    for n, (x, y, w, h, c) in {'holder\n18650': (hx, hy, HOLDER[0], HOLDER[1], '#fd8'), 'TP4056\n(rieles)': (IN_W - TP4056[0], TP_Y, *TP4056, '#8bf'),
                              'ESP32-C3\n(pedestal)': (IN_W - C3[0], C3_Y, *C3, '#8d8'), 'MAX\n98357A': (AMP_X, AMP_Y, *AMP, '#c9f')}.items():
        a.add_patch(Rectangle((P + x, P + y), w, h, fc=c, ec='k')); a.text(P + x + w / 2, P + y + h / 2, n, ha='center', va='center', fontsize=8)
    a.add_patch(Rectangle((P + 47.3, P + GRAPA_GANCHO[0]), 5.5, GRAPA_POSTE[1] - GRAPA_GANCHO[0], fc='#fc6', ec='k', alpha=.7))
    a.add_patch(Circle((P + GRAPA_X, P + sum(GRAPA_POSTE) / 2), 2.2, fc='w', ec='k'))
    for px, py in POSTES:
        a.add_patch(Circle((P + px, P + py), M4_PASO / 2, fc='w', ec='k'))
    a.text(W + 0.5, P + C3_Y + 9, 'USB ->', rotation=90, va='center', fontsize=8)
    a.text(W + 0.5, P + TP_Y + 9, 'USB ->', rotation=90, va='center', fontsize=8)
    fig.suptitle(f'Latido - carcasa {W:.1f} x {H:.1f} x {FONDO:.1f} mm', fontsize=14)
    fig.tight_layout(); fig.savefig(O / 'Vista_Carcasa.png', dpi=110); plt.close(fig)

    # corte lateral (Y-Z)
    fig, a = plt.subplots(figsize=(6, 9)); a.set_aspect('equal'); a.axis('off')
    a.set_title('Corte lateral: frente a la izquierda, tapa a la derecha')
    a.add_patch(Rectangle((0, 0), FONDO, H, fill=False, lw=1.6)); a.add_patch(Rectangle((Z_TAPA, 0), TAPA, H, fc='#bbb'))
    a.add_patch(Rectangle((0, 0), FRENTE, H, fc='#bbb'))
    for n, c in (('holder', '#fd8'), ('bocina', '#ccc'), ('pantalla', '#9cf'), ('c3', '#8d8'), ('tp4056', '#8bf'), ('amp', '#c9f'), ('switch', '#6c6')):
        b = malla(comp[n]).bounds
        a.add_patch(Rectangle((b[0][2], b[0][1]), b[1][2] - b[0][2], b[1][1] - b[0][1], fc=c, ec='k', alpha=.75))
        a.text((b[0][2] + b[1][2]) / 2, (b[0][1] + b[1][1]) / 2, n, ha='center', va='center', fontsize=8)
    a.set_xlim(-3, FONDO + 3); a.set_ylim(-3, H + 3)
    fig.tight_layout(); fig.savefig(O / 'Vista_Corte.png', dpi=110); plt.close(fig)


# ============================================================
if __name__ == '__main__':
    # el STL del puente es uno solo (se imprimen 2): el de la pantalla y el de la bocina son iguales
    piezas = {'Latido_Frente': frente(), 'Latido_Tapa': tapa(), 'Latido_Puente': puente(*P_LCD), 'Latido_Grapa_C3': grapa()}
    ensamble = dict(piezas, puente_bocina=puente(*P_BOC))
    comp = componentes()
    rep = {'exterior_mm': [round(W, 1), round(H, 1), FONDO], 'piezas': {}, 'choques_mm3': {}}
    for n, m in piezas.items():
        t = malla(m)
        rep['piezas'][n] = {'cerrada': bool(t.is_watertight), 'cuerpos': int(t.body_count), 'volumen_cm3': round(t.volume / 1000, 1),
                            'medidas_mm': [round(float(v), 1) for v in t.extents]}
        imp = t.copy()
        if n in ('Latido_Tapa', 'Latido_Grapa_C3'):   # se imprimen con la cara de afuera / de arriba en la cama
            imp.apply_transform(trimesh.transformations.rotation_matrix(np.pi, [1, 0, 0]))
        imp.apply_translation(-imp.bounds[0])
        imp.export(O / f'{n}.stl')
    # choques: todo contra todo
    todo = {**ensamble, **comp}
    nombres = list(todo)
    for i, a in enumerate(nombres):
        for b in nombres[i + 1:]:
            v = (todo[a] ^ todo[b]).volume()
            if v > 0.5: rep['choques_mm3'][f'{a} / {b}'] = round(v, 1)
    hueco = IN_W * IN_H * (Z_TAPA - FRENTE) - sum(c.volume() for c in comp.values()) \
        - sum(m.volume() for n, m in ensamble.items() if n != 'Latido_Frente') + W * H * TAPA
    rep['aire_para_la_bocina_cm3'] = round(hueco / 1000)
    rep['rejilla_barrenos'] = len(corazon())
    def pausa(z):                # la pausa va ANTES de la primera capa que tapa el hexagono
        return {'altura_mm': round(z, 2), 'pausa_antes_de_la_capa': int(round(z / CAPA)) + 1}
    rep['pausas_para_tuercas_M4'] = {
        'Latido_Frente': [dict(pausa(Z_TUERCA_PUENTE + TUERCA[1]), tuercas=2, donde='postes de los puentes'),
                          dict(pausa(Z_TUERCA_TAPA + TUERCA[1]), tuercas=3, donde='postes de la tapa')],
        'Latido_Tapa': [dict(pausa(FONDO - Z_TUERCA_GRAPA), tuercas=1, donde='poste de la grapa de la C3')],
        'nota': f'capas de {CAPA} mm, primera capa incluida; con otro perfil guiate por la altura'}
    (O / 'validacion_carcasa.json').write_text(json.dumps(rep, indent=1, ensure_ascii=False), encoding='utf-8')
    vistas(comp)
    print(json.dumps(rep, indent=1, ensure_ascii=False))

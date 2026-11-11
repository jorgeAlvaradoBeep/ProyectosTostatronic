"""generar_carcasa.py - Carcasa de "Latido" (frente + tapa trasera + puente de la pantalla) para imprimir en 3D.
Desarrollado por Tostatronic - Ing. Jorge Alvarado

Torre de 57 x 96 x 41 mm que se para sola. Al frente: la pantalla redonda arriba y la bocina abajo, detras de
una rejilla en forma de corazon. En la tapa trasera van el holder de la 18650 (parado, a la izquierda) y, en
columna a la derecha, el TP4056, la ESP32-C3 Super Mini y el MAX98357A. Los dos USB-C salen por el costado.
Todo el interior es la caja acustica de la bocina (unos 90 cm3 libres).

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
#  MEDIDAS DE LAS PIEZAS  (de las fichas de tostatronic.com; las marcadas (*) son
#  tipicas del modulo: CONFIRMALAS CON VERNIER antes de imprimir)
# ============================================================
HOLDER = (20.9, 77.7, 21.3)    # holder 18650 de 1 pila p/ soldar: ancho, largo, alto
HOLDER_BARRENO = (10.68, 55.61)  # primer barreno de 3.2 mm desde la orilla, y separacion entre los dos
HOLDER_PIN = 2.4               # las terminales salen a 2.4 mm de cada orilla y bajan 3.3 mm
BOCINA_D = 40.0                # bocina 8 ohms 0.5 W
BOCINA_H = 6.0                 # (*) alto total de la bocina
LCD_VENTANA = 33.2             # area activa de 32.4 mm + holgura
LCD_CUERPO_D = 35.6            # (*) vidrio de la pantalla
LCD_PCB_D = 38.0               # (*) tarjeta redonda (hasta este diametro cabe)
LCD_GRUESO = 5.2               # (*) del vidrio a la espalda de la tarjeta
LCD_OREJA = (19.0, 26.0)       # (*) pestana de los pines: ancho, y cuanto sube desde el centro
LCD_PINES = (20.5, 8.5)        # (*) fila de pines: distancia al centro, y cuanto salen hacia atras
AMP = (18.0, 19.0)             # MAX98357A
TP4056 = (26.2, 17.6)          # TP4056 tipo C con proteccion (largo con el USB hacia la pared)
C3 = (22.5, 18.0)              # ESP32-C3 Super Mini, SIN pines soldados (largo con el USB hacia la pared)
SW_CUERPO = (8.8, 4.6)         # micro switch deslizable SS12D00G4: cuerpo
SW_RANURA = (5.4, 2.4)         # ranura para la palanca

# ============================================================
#  CAJA
# ============================================================
PARED = 2.4
FRENTE = 2.4
TAPA = 3.0
IN_W, IN_H = 52.0, 91.0        # hueco interior
FONDO = 41.0                   # fondo total con la tapa puesta
R_EXT = 4.4                    # radio de las esquinas
HOLG = 0.25                    # holgura de ajuste
PILOTO_M3 = 2.5                # barreno para tornillo M3 autorroscante en PLA
PASO_M3 = 3.4

W, H = IN_W + 2 * PARED, IN_H + 2 * PARED
Z_TAPA = FONDO - TAPA          # cara interior de la tapa

# --- frente
BOC = (IN_W / 2, 23.5)                     # centro de la bocina (ix, iy)
LCD = (IN_W / 2, IN_H - LCD_OREJA[1] - 1)  # centro de la pantalla
PUENTE_X = 22.5                            # postes del puente a cada lado de la pantalla
PUENTE = (2 * PUENTE_X + 6.5, 8.0, 3.0)    # largo, ancho, grueso

# --- tapa (posiciones interiores, vistas de frente)
PEDESTAL = 2.0                             # el holder va levantado para librar sus terminales
HOLDER_XY = (2.2, 2.5)                     # esquina inferior izquierda del holder
TP_Y, C3_Y, AMP_Y = 9.0, 33.0, 57.0        # columna derecha, de abajo hacia arriba
AMP_X = 30.0
CUNA = (1.2, 2.6)                          # paredes de las cunas: grueso, alto
PISO_CUNA = 1.0                            # costillas donde asienta cada tarjeta
USB = (12.6, 7.0)                          # ventana del USB-C: ancho (en Y), alto (en Z)
USB_Z = Z_TAPA - PISO_CUNA - 1.6 - 1.6     # centro del conector
POSTES = [(3.4, IN_H - 3.4), (IN_W - 3.4, IN_H - 3.4), (IN_W - 3.4, 3.4)]   # tornillos de la tapa
R_POSTE = 3.2
SW = (41.0, 24.0)                          # switch en la pared de arriba: ix, z


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
    # pantalla: ventana con chaflan hacia afuera y dos postes para el puente
    s -= cil(*LCD, LCD_VENTANA / 2, -1, FRENTE + 1)
    s -= cil(*LCD, LCD_VENTANA / 2 + 1.2, 0, 1.2, r1=LCD_VENTANA / 2)
    for sx in (-1, 1):
        s += cil(LCD[0] + sx * PUENTE_X, LCD[1], 3.0, FRENTE - 0.1, FRENTE + LCD_GRUESO) + \
            caja(LCD[0] + sx * PUENTE_X - (0 if sx > 0 else 3.5), LCD[0] + sx * PUENTE_X + (3.5 if sx > 0 else 0),
                 LCD[1] - 3.0, LCD[1] + 3.0, FRENTE - 0.1, FRENTE + LCD_GRUESO)
        s -= cil(LCD[0] + sx * PUENTE_X, LCD[1], PILOTO_M3 / 2, FRENTE + 0.6, FRENTE + LCD_GRUESO + 1)
    # bocina: anillo de asiento y rejilla de corazon
    s += cil(*BOC, BOCINA_D / 2 + HOLG + 1.4, FRENTE - 0.1, FRENTE + 3.0) - cil(*BOC, BOCINA_D / 2 + HOLG, FRENTE, FRENTE + 4)
    for x, y in corazon():
        s -= cil(BOC[0] + x, BOC[1] + y, 1.2, -1, FRENTE + 1, seg=24)
    # postes de los tornillos de la tapa
    for px, py in POSTES:
        s += cil(px, py, R_POSTE, FRENTE - 0.1, Z_TAPA) + \
            caja(min(px, IN_W - px) * 0 + (0 if px < IN_W / 2 else px), px if px < IN_W / 2 else IN_W,
                 py - R_POSTE, py + R_POSTE, FRENTE - 0.1, Z_TAPA) + \
            caja(px - R_POSTE, px + R_POSTE, 0 if py < IN_H / 2 else py, py if py < IN_H / 2 else IN_H, FRENTE - 0.1, Z_TAPA)
        s -= cil(px, py, PILOTO_M3 / 2, Z_TAPA - 14, Z_TAPA + 1)
    s = s ^ redondeado(0, 0, W, H, R_EXT, 0, Z_TAPA)          # nada sale del contorno
    # ventanas de los USB-C (pared derecha, abiertas hacia la tapa)
    for y0, ancho in ((TP_Y, TP4056[1]), (C3_Y, C3[1])):
        yc = y0 + ancho / 2
        s -= caja(IN_W - 0.5, IN_W + PARED + 1, yc - USB[0] / 2, yc + USB[0] / 2, USB_Z - USB[1] / 2, FONDO)
    # switch deslizable (pared de arriba): cavidad por dentro y ranura para la palanca
    s -= caja(SW[0] - SW_CUERPO[0] / 2, SW[0] + SW_CUERPO[0] / 2, IN_H - 0.5, IN_H + PARED - 1.0, SW[1] - SW_CUERPO[1] / 2, SW[1] + SW_CUERPO[1] / 2)
    s -= caja(SW[0] - SW_RANURA[0] / 2, SW[0] + SW_RANURA[0] / 2, IN_H - 0.5, IN_H + PARED + 1, SW[1] - SW_RANURA[1] / 2, SW[1] + SW_RANURA[1] / 2)
    return s


# ============================================================
#  TAPA TRASERA (lleva el holder y las tres tarjetas)
# ============================================================
def cuna(ix0, iy0, largo, ancho, abierta_derecha):
    """costillas de asiento + paredes bajas alrededor de una tarjeta"""
    g, h = CUNA
    z0, zp = Z_TAPA - PISO_CUNA - h, Z_TAPA - PISO_CUNA
    c = caja(ix0 + 2, ix0 + 4, iy0, iy0 + ancho, zp, Z_TAPA + 0.1) + caja(ix0 + largo - 6, ix0 + largo - 4, iy0, iy0 + ancho, zp, Z_TAPA + 0.1)
    x1 = ix0 + largo - (4.0 if abierta_derecha else -HOLG - g)
    c += caja(ix0 - HOLG - g, x1, iy0 - HOLG - g, iy0 - HOLG, z0, Z_TAPA + 0.1)
    c += caja(ix0 - HOLG - g, x1, iy0 + ancho + HOLG, iy0 + ancho + HOLG + g, z0, Z_TAPA + 0.1)
    c += caja(ix0 - HOLG - g, ix0 - HOLG, iy0 - HOLG - g, iy0 + ancho + HOLG + g, z0, Z_TAPA + 0.1)
    if not abierta_derecha:
        c += caja(ix0 + largo + HOLG, ix0 + largo + HOLG + g, iy0 - HOLG - g, iy0 + ancho + HOLG + g, z0, Z_TAPA + 0.1)
        c -= caja(ix0 + 5, ix0 + largo - 5, iy0 - 3, iy0 + ancho + 3, z0 - 1, zp)      # paso de cables arriba y abajo
    return c


def tapa():
    t = redondeado(0, 0, W, H, R_EXT, Z_TAPA, FONDO)
    # ceja que entra en el frente y centra la tapa
    c = redondeado(PARED + HOLG, PARED + HOLG, W - PARED - HOLG, H - PARED - HOLG, R_EXT - PARED - HOLG, Z_TAPA - 3, Z_TAPA + 0.1) - \
        redondeado(PARED + HOLG + 1.6, PARED + HOLG + 1.6, W - PARED - HOLG - 1.6, H - PARED - HOLG - 1.6, 0.6, Z_TAPA - 4, Z_TAPA + 1)
    for px, py in POSTES:
        c -= caja(px - R_POSTE - 0.5, px + R_POSTE + 0.5, py - R_POSTE - 0.5, py + R_POSTE + 0.5, 0, FONDO)
        c -= caja(0 if px < IN_W / 2 else px, px if px < IN_W / 2 else IN_W, py - R_POSTE - 0.5, py + R_POSTE + 0.5, 0, FONDO)
        c -= caja(px - R_POSTE - 0.5, px + R_POSTE + 0.5, 0 if py < IN_H / 2 else py, py if py < IN_H / 2 else IN_H, 0, FONDO)
    c -= caja(IN_W - 3, IN_W, TP_Y - 2, TP_Y + TP4056[1] + 2, 0, FONDO)          # frente a los USB no hay ceja
    c -= caja(IN_W - 3, IN_W, C3_Y - 2, C3_Y + C3[1] + 2, 0, FONDO)
    c -= caja(SW[0] - 8, SW[0] + 8, IN_H - 3, IN_H, 0, FONDO)                    # ni frente al switch
    t += c
    # pedestal del holder, con dos canales para sus terminales y los cables
    hx, hy = HOLDER_XY
    t += caja(hx, hx + HOLDER[0], hy, hy + HOLDER[1], Z_TAPA - PEDESTAL, Z_TAPA + 0.1)
    t += caja(hx + HOLDER[0] + HOLG, hx + HOLDER[0] + HOLG + 1.2, hy + 12, hy + HOLDER[1] - 12, Z_TAPA - PEDESTAL - 4, Z_TAPA + 0.1)   # guia lateral
    for y in (hy + HOLDER_PIN, hy + HOLDER[1] - HOLDER_PIN):
        t -= caja(hx + 3, hx + HOLDER[0] + 3, y - 2.5, y + 2.5, Z_TAPA - PEDESTAL - 1, Z_TAPA + 1.8)
    for y in (hy + HOLDER_BARRENO[0], hy + HOLDER_BARRENO[0] + HOLDER_BARRENO[1]):
        t -= cil(hx + HOLDER[0] / 2, y, PILOTO_M3 / 2, Z_TAPA - PEDESTAL - 1, FONDO - 0.8)
    # cunas de las tarjetas
    t += cuna(IN_W - TP4056[0], TP_Y, TP4056[0], TP4056[1], True)
    t += cuna(IN_W - C3[0], C3_Y, C3[0], C3[1], True)
    t += cuna(AMP_X, AMP_Y, AMP[0], AMP[1], False)
    # tornillos de la tapa (cabeza embutida)
    for px, py in POSTES:
        t -= cil(px, py, PASO_M3 / 2, Z_TAPA - 5, FONDO + 1)
        t -= cil(px, py, 3.2, FONDO - 1.6, FONDO + 1)
    return t


def puente():
    """barra que aprieta la pantalla por atras (ponle un pedacito de cinta de espuma en medio)"""
    L, A, G = PUENTE
    z0 = FRENTE + LCD_GRUESO
    p = caja(LCD[0] - L / 2, LCD[0] + L / 2, LCD[1] - A / 2, LCD[1] + A / 2, z0, z0 + G)
    for sx in (-1, 1):
        p -= cil(LCD[0] + sx * PUENTE_X, LCD[1], PASO_M3 / 2, z0 - 1, z0 + G + 1)
    return p


# ============================================================
#  COMPONENTES (bultos para revisar que nada choque)
# ============================================================
def componentes():
    hx, hy = HOLDER_XY
    zc = Z_TAPA - PISO_CUNA
    return {
        'holder': caja(hx, hx + HOLDER[0], hy, hy + HOLDER[1], Z_TAPA - PEDESTAL - HOLDER[2], Z_TAPA - PEDESTAL),
        'bocina': cil(*BOC, BOCINA_D / 2, FRENTE, FRENTE + BOCINA_H),
        'pantalla': cil(*LCD, LCD_CUERPO_D / 2, FRENTE, FRENTE + LCD_GRUESO - 1.6) + cil(*LCD, LCD_PCB_D / 2, FRENTE + LCD_GRUESO - 1.6, FRENTE + LCD_GRUESO)
                    + caja(LCD[0] - LCD_OREJA[0] / 2, LCD[0] + LCD_OREJA[0] / 2, LCD[1], LCD[1] + LCD_OREJA[1], FRENTE + LCD_GRUESO - 1.6, FRENTE + LCD_GRUESO)
                    + caja(LCD[0] - 10.2, LCD[0] + 10.2, LCD[1] + LCD_PINES[0] - 1.3, LCD[1] + LCD_PINES[0] + 1.3, FRENTE + LCD_GRUESO, FRENTE + LCD_GRUESO + LCD_PINES[1]),
        'tp4056': caja(IN_W - TP4056[0], IN_W, TP_Y, TP_Y + TP4056[1], zc - 1.6 - 3.3, zc),
        'c3': caja(IN_W - C3[0], IN_W, C3_Y, C3_Y + C3[1], zc - 1.6 - 3.3, zc),
        'amp': caja(AMP_X, AMP_X + AMP[0], AMP_Y, AMP_Y + AMP[1], zc - 1.6 - 3.0, zc),
        'switch': caja(SW[0] - 4.3, SW[0] + 4.3, IN_H - 6.5, IN_H + 1.3, SW[1] - 2.2, SW[1] + 2.2),
        'tornillos_puente': cil(LCD[0] - PUENTE_X, LCD[1], 3.0, FRENTE + LCD_GRUESO + PUENTE[2], FRENTE + LCD_GRUESO + PUENTE[2] + 2.2)
                            + cil(LCD[0] + PUENTE_X, LCD[1], 3.0, FRENTE + LCD_GRUESO + PUENTE[2], FRENTE + LCD_GRUESO + PUENTE[2] + 2.2),
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
    # frente
    ax[0].add_patch(Circle((P + LCD[0], P + LCD[1]), LCD_VENTANA / 2, fc='#223', ec='k'))
    for x, y in corazon():
        ax[0].add_patch(Circle((P + BOC[0] + x, P + BOC[1] + y), 1.2, fc='#d33', ec='none'))
    ax[0].add_patch(Circle((P + BOC[0], P + BOC[1]), BOCINA_D / 2, fill=False, ls=':', ec='gray'))
    ax[0].text(W / 2, H + 0.6, 'switch arriba  ->', ha='center', fontsize=8)
    ax[0].text(W + 0.5, P + C3_Y + 9, 'USB C3', rotation=90, va='center', fontsize=8)
    ax[0].text(W + 0.5, P + TP_Y + 9, 'USB carga', rotation=90, va='center', fontsize=8)

    def esp(x):          # visto desde atras, X se voltea
        return W - x
    a = ax[1]
    a.add_patch(Rectangle((P, P), IN_W, IN_H, fill=False, ec='gray'))
    a.add_patch(Circle((esp(P + LCD[0]), P + LCD[1]), LCD_PCB_D / 2, fc='#9cf', ec='k'))
    a.add_patch(Rectangle((esp(P + LCD[0] + LCD_OREJA[0] / 2), P + LCD[1]), LCD_OREJA[0], LCD_OREJA[1], fc='#9cf', ec='k'))
    a.add_patch(Rectangle((esp(P + LCD[0] + PUENTE[0] / 2), P + LCD[1] - PUENTE[1] / 2), PUENTE[0], PUENTE[1], fc='#fc6', ec='k', alpha=.8))
    a.add_patch(Circle((esp(P + BOC[0]), P + BOC[1]), BOCINA_D / 2, fc='#ccc', ec='k'))
    for px, py in POSTES:
        a.add_patch(Circle((esp(P + px), P + py), R_POSTE, fc='#888', ec='k'))
    a.add_patch(Rectangle((esp(P + SW[0] + 4.4), P + IN_H - 6), 8.8, 6, fc='#6c6', ec='k'))
    a.text(esp(P + LCD[0]), P + LCD[1] + 8, 'pantalla', ha='center'); a.text(esp(P + BOC[0]), P + BOC[1], 'bocina', ha='center')
    a = ax[2]
    a.add_patch(Rectangle((P, P), IN_W, IN_H, fill=False, ec='gray'))
    hx, hy = HOLDER_XY
    for n, (x, y, w, h, c) in {'holder\n18650': (hx, hy, HOLDER[0], HOLDER[1], '#fd8'), 'TP4056': (IN_W - TP4056[0], TP_Y, *TP4056, '#8bf'),
                              'ESP32-C3': (IN_W - C3[0], C3_Y, *C3, '#8d8'), 'MAX\n98357A': (AMP_X, AMP_Y, *AMP, '#c9f')}.items():
        a.add_patch(Rectangle((P + x, P + y), w, h, fc=c, ec='k')); a.text(P + x + w / 2, P + y + h / 2, n, ha='center', va='center', fontsize=8)
    for px, py in POSTES:
        a.add_patch(Circle((P + px, P + py), PASO_M3 / 2, fc='w', ec='k'))
    a.text(W + 0.5, P + C3_Y + 9, 'USB ->', rotation=90, va='center', fontsize=8)
    a.text(W + 0.5, P + TP_Y + 9, 'USB ->', rotation=90, va='center', fontsize=8)
    fig.suptitle(f'Latido - carcasa {W:.1f} x {H:.1f} x {FONDO:.1f} mm', fontsize=14)
    fig.tight_layout(); fig.savefig(O / 'Vista_Carcasa.png', dpi=110); plt.close(fig)

    # corte lateral (Y-Z) por el centro
    fig, a = plt.subplots(figsize=(6, 9)); a.set_aspect('equal'); a.axis('off')
    a.set_title('Corte lateral: frente a la izquierda, tapa a la derecha')
    a.add_patch(Rectangle((0, 0), FONDO, H, fill=False, lw=1.6)); a.add_patch(Rectangle((Z_TAPA, 0), TAPA, H, fc='#bbb'))
    a.add_patch(Rectangle((0, 0), FRENTE, H, fc='#bbb'))
    for n, c in (('holder', '#fd8'), ('bocina', '#ccc'), ('pantalla', '#9cf'), ('c3', '#8d8'), ('tp4056', '#8bf'), ('amp', '#c9f')):
        b = malla(comp[n]).bounds
        a.add_patch(Rectangle((b[0][2], b[0][1]), b[1][2] - b[0][2], b[1][1] - b[0][1], fc=c, ec='k', alpha=.75))
        a.text((b[0][2] + b[1][2]) / 2, (b[0][1] + b[1][1]) / 2, n, ha='center', va='center', fontsize=8)
    a.set_xlim(-3, FONDO + 3); a.set_ylim(-3, H + 3)
    fig.tight_layout(); fig.savefig(O / 'Vista_Corte.png', dpi=110); plt.close(fig)


# ============================================================
if __name__ == '__main__':
    piezas = {'Latido_Frente': frente(), 'Latido_Tapa': tapa(), 'Latido_Puente_Pantalla': puente()}
    comp = componentes()
    rep = {'exterior_mm': [round(W, 1), round(H, 1), FONDO], 'piezas': {}, 'choques_mm3': {}}
    for n, m in piezas.items():
        t = malla(m)
        rep['piezas'][n] = {'cerrada': bool(t.is_watertight), 'cuerpos': int(t.body_count), 'volumen_cm3': round(t.volume / 1000, 1),
                            'medidas_mm': [round(float(v), 1) for v in t.extents]}
        imp = t.copy()
        if n == 'Latido_Tapa':                       # la tapa se imprime con la cara de afuera en la cama
            imp.apply_transform(trimesh.transformations.rotation_matrix(np.pi, [1, 0, 0]))
        imp.apply_translation(-imp.bounds[0])
        imp.export(O / f'{n}.stl')
    # choques: componente contra pieza y componente contra componente
    nombres = list(comp)
    for i, a in enumerate(nombres):
        for n, m in piezas.items():
            v = (comp[a] ^ m).volume()
            if v > 0.5: rep['choques_mm3'][f'{a} / {n}'] = round(v, 1)
        for b in nombres[i + 1:]:
            v = (comp[a] ^ comp[b]).volume()
            if v > 0.5: rep['choques_mm3'][f'{a} / {b}'] = round(v, 1)
    v = (piezas['Latido_Frente'] ^ piezas['Latido_Tapa']).volume()
    if v > 0.5: rep['choques_mm3']['frente / tapa'] = round(v, 1)
    hueco = IN_W * IN_H * (Z_TAPA - FRENTE) - sum(c.volume() for c in comp.values())
    rep['aire_para_la_bocina_cm3'] = round(hueco / 1000)
    rep['rejilla_barrenos'] = len(corazon())
    (O / 'validacion_carcasa.json').write_text(json.dumps(rep, indent=1, ensure_ascii=False), encoding='utf-8')
    vistas(comp)
    print(json.dumps(rep, indent=1, ensure_ascii=False))

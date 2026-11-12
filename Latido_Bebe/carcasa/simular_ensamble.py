"""simular_ensamble.py - Simulacion 3D del ensamble de la carcasa de "Latido".
Desarrollado por Tostatronic - Ing. Jorge Alvarado

Toma la geometria de generar_carcasa.py (las mismas piezas que se imprimen) y escribe
Simulacion_Ensamble.html: una pagina con el armado paso a paso que se abre en cualquier navegador.

Uso:  python simular_ensamble.py
"""
import base64
import json
from pathlib import Path
import numpy as np
import manifold3d as md
import generar_carcasa as g

O = Path(__file__).parent
M = md.Manifold
P = g.PARED

# colores
ROSA, ROSA2, CREMA = '#efb3bf', '#e59aab', '#f6f0e4'
ACERO, LATON, NEGRO, ORO = '#b9bec6', '#d2a63c', '#23232a', '#e0b84a'


def datos(m):
    mm = m.to_mesh()
    v = np.asarray(mm.vert_properties, dtype=np.float32)[:, :3]
    i = np.asarray(mm.tri_verts, dtype=np.uint32)
    return {'v': base64.b64encode(np.ascontiguousarray(v).tobytes()).decode(),
            'i': base64.b64encode(np.ascontiguousarray(i).tobytes()).decode()}


def tornillo(ix, iy, z, sentido, d=4.0, largo=16.0, cabeza=(7.2, 4.0)):
    """z = donde asienta la cabeza; sentido = hacia donde apunta la rosca (+1 o -1 en Z)"""
    if sentido < 0:
        return g.cil(ix, iy, cabeza[0] / 2, z, z + cabeza[1], seg=32) + g.cil(ix, iy, d / 2, z - largo, z, seg=24)
    return g.cil(ix, iy, cabeza[0] / 2, z - cabeza[1], z, seg=32) + g.cil(ix, iy, d / 2, z, z + largo, seg=24)


def tuerca(ix, iy, z):
    r = 7.0 / 2 / np.cos(np.pi / 6)
    pts = np.array([(r * np.cos(a), r * np.sin(a)) for a in np.radians(np.arange(30, 390, 60))])
    h = md.CrossSection([pts], md.FillRule.Positive).extrude(3.2).translate([P + ix, P + iy, z])
    return h - g.cil(ix, iy, 2.0, z - 1, z + 4, seg=24)


def corazon_lcd():
    t = np.linspace(0, 2 * np.pi, 80, endpoint=False)
    x = 16 * np.sin(t) ** 3
    y = 13 * np.cos(t) - 5 * np.cos(2 * t) - 2 * np.cos(3 * t) - np.cos(4 * t)
    pts = np.column_stack([x, y])[::-1] * 0.55       # sentido antihorario
    return md.CrossSection([pts], md.FillRule.Positive).extrude(0.15).translate([P + g.LCD[0], P + g.LCD[1] + 1, g.FRENTE - 0.15])


piezas = []


def pieza(id, grupo, paso, mallas, ruta=None, pivote=None, eje=None, ini=0.0, fin=0.85, carcasa=False):
    piezas.append({'id': id, 'grupo': grupo, 'paso': paso, 'ruta': ruta or [], 'pivote': pivote, 'eje': eje,
                   'ini': ini, 'fin': fin, 'carcasa': carcasa,
                   'mallas': [dict(datos(m), color=c, metal=bool(mt)) for m, c, mt in mallas]})


Z_TAPA, FRENTE, IN_W, IN_H, FONDO = g.Z_TAPA, g.FRENTE, g.IN_W, g.IN_H, g.FONDO

# ---------------- piezas impresas
pieza('frente', 'frente', -1, [(g.frente(), ROSA, 0)], carcasa=True)
pieza('tapa', 'tapa', -1, [(g.tapa(), ROSA2, 0)], carcasa=True)

# ---------------- paso 0: tuercas atrapadas
for n, (py, _) in enumerate((g.P_LCD, g.P_BOC)):
    pieza(f'tuerca_puente{n}', 'frente', 0, [(tuerca(g.PUENTE_X, py, g.Z_TUERCA_PUENTE), LATON, 1)], [[0, 0, 45, 0]], fin=0.6)
for n, (px, py) in enumerate(g.POSTES):
    pieza(f'tuerca_tapa{n}', 'frente', 0, [(tuerca(px, py, g.Z_TUERCA_TAPA), LATON, 1)], [[0, 0, 25, 0]], ini=0.3, fin=0.9)
GY = sum(g.GRAPA_POSTE) / 2
pieza('tuerca_grapa', 'tapa', 0, [(tuerca(g.GRAPA_X, GY, g.Z_TUERCA_GRAPA + 0.4), LATON, 1)], [[0, 0, -30, 0]], ini=0.3, fin=0.9)

# ---------------- pantalla
z0 = FRENTE
lcd = [
    (g.cil(*g.LCD, g.LCD_CUERPO_D / 2, z0, z0 + g.LCD_GRUESO - 1.6), '#15161c', 0),
    (corazon_lcd(), '#ff4d6d', 0),
    (g.cil(*g.LCD, g.LCD_PCB_D / 2, z0 + g.LCD_GRUESO - 1.6, z0 + g.LCD_GRUESO)
     + g.caja(g.LCD[0] - g.LCD_OREJA[0] / 2, g.LCD[0] + g.LCD_OREJA[0] / 2, g.LCD[1], g.LCD[1] + g.LCD_OREJA[1], z0 + g.LCD_GRUESO - 1.6, z0 + g.LCD_GRUESO), '#1d4f91', 0),
    (g.caja(g.LCD[0] - 10.2, g.LCD[0] + 10.2, g.LCD[1] + g.LCD_PINES[0] - 1.3, g.LCD[1] + g.LCD_PINES[0] + 1.3, z0 + g.LCD_GRUESO, z0 + g.LCD_GRUESO + 2.5), NEGRO, 0),
]
pines = M()
for k in range(8):
    x = g.LCD[0] - 8.89 + 2.54 * k
    pines += g.caja(x - 0.32, x + 0.32, g.LCD[1] + g.LCD_PINES[0] - 0.32, g.LCD[1] + g.LCD_PINES[0] + 0.32, z0 + g.LCD_GRUESO + 2.5, z0 + g.LCD_GRUESO + g.LCD_PINES[1])
lcd.append((pines, ORO, 1))
pieza('pantalla', 'frente', 1, lcd, [[0, 0, 50, 0]])


# ---------------- puentes (la punta entra inclinada en la bolsa de la pared y luego baja el bloque)
def paso_puente(nombre, paso, py, z):
    pieza(nombre, 'frente', paso, [(g.puente(py, z), CREMA, 0)], [[5, 0, 38, -22], [2.5, 0, 1.5, -22], [0, 0, 0, -22]],
          pivote=[P - 1.2, P + py, z], eje=[0, 1, 0], fin=0.6)
    pieza(nombre + '_tornillo', 'frente', paso, [(tornillo(g.PUENTE_X, py, z + g.PUENTE[2], -1), ACERO, 1)], [[0, 0, 35, 0]], ini=0.55, fin=0.95)


paso_puente('puente_pantalla', 2, *g.P_LCD)

# ---------------- bocina
b = g.BOC
pieza('bocina', 'frente', 3, [
    (g.cil(*b, 20.0, z0, z0 + 1.4) - g.cil(*b, 18.6, z0 - 1, z0 + 2), '#8c9098', 1),
    (g.cil(*b, 18.6, z0, z0 + 1.4), NEGRO, 0),
    (g.cil(*b, 19.0, z0 + 1.4, z0 + 4.2, r1=11.5), NEGRO, 0),
    (g.cil(*b, 10.0, z0 + 4.2, z0 + g.BOCINA_H), ACERO, 1),
], [[0, 0, 50, 0]])
paso_puente('puente_bocina', 4, *g.P_BOC)

# ---------------- switch 117S (entra por fuera de la pared de arriba)
sx, sz = g.SW
yw = IN_H + P
pieza('switch', 'frente', 5, [
    (g.caja(sx - g.SW_CORTE[0] / 2, sx + g.SW_CORTE[0] / 2, yw - g.SW_FONDO[0], yw, sz - g.SW_CORTE[1] / 2, sz + g.SW_CORTE[1] / 2)
     + g.caja(sx - 10.5, sx + 10.5, yw, yw + 2.0, sz - 7.5, sz + 7.5), NEGRO, 0),
    (g.caja(sx - 7.5, sx + 7.5, yw + 2.0, yw + 3.6, sz - 5.2, sz + 5.2), '#c8283c', 0),
    (g.caja(sx - 5.5, sx - 4.7, yw - g.SW_FONDO[1] + 2, yw - g.SW_FONDO[0], sz - 2.4, sz + 2.4)
     + g.caja(sx + 4.7, sx + 5.5, yw - g.SW_FONDO[1] + 2, yw - g.SW_FONDO[0], sz - 2.4, sz + 2.4), ORO, 1),
], [[0, 35, 0, 0]])

# ---------------- holder 18650 en su casita
hx, hy = g.HOLDER_XY
hz0, hz1 = Z_TAPA - g.PEDESTAL - g.HOLDER[2], Z_TAPA - g.PEDESTAL
holder = g.caja(hx, hx + g.HOLDER[0], hy, hy + g.HOLDER[1], hz0, hz1) - g.caja(hx + 1.2, hx + g.HOLDER[0] - 1.2, hy + 2.5, hy + g.HOLDER[1] - 2.5, hz0 - 1, hz1 - 1.5)
ys_m3 = (hy + g.HOLDER[1] / 2, hy + g.HOLDER[1] / 2 - g.HOLDER_BARRENO / 2, hy + g.HOLDER[1] / 2 + g.HOLDER_BARRENO / 2)
for y in ys_m3:
    holder -= g.cil(hx + g.HOLDER[0] / 2, y, 1.6, hz1 - 3, hz1 + 1, seg=24)
pieza('holder', 'tapa', 6, [(holder, NEGRO, 0)], [[0, 0, -45, 0]], fin=0.55)
m3 = M()
for y in ys_m3:
    m3 += tornillo(hx + g.HOLDER[0] / 2, y, hz1 - 1.5, +1, d=3.0, largo=6.0, cabeza=(5.5, 2.0))
pieza('holder_m3', 'tapa', 6, [(m3, ACERO, 1)], [[0, 0, -40, 0]], ini=0.5, fin=0.95)

# ---------------- TP4056: se desliza desde la orilla bajo los rieles
tx, zs = IN_W - g.TP4056[0], Z_TAPA - g.PISO_CUNA
yc = g.TP_Y + g.TP4056[1] / 2
pieza('tp4056', 'tapa', 7, [
    (g.caja(tx, IN_W, g.TP_Y, g.TP_Y + g.TP4056[1], zs - 1.6, zs), '#1f5fb0', 0),
    (g.caja(IN_W - 7.3, IN_W + 1.2, yc - 4.5, yc + 4.5, zs - 1.6 - 3.2, zs - 1.6), ACERO, 1),
    (g.caja(tx + 9, tx + 14, yc - 2, yc + 2, zs - 1.6 - 1.5, zs - 1.6) + g.caja(tx + 3, tx + 6, yc - 5, yc - 2, zs - 1.6 - 1.2, zs - 1.6), NEGRO, 0),
], [[36, 0, 0, 0]])

# ---------------- ESP32-C3 con pines, sobre su pedestal
cx, zc = IN_W - g.C3[0], g.Z_C3
yc = g.C3_Y + g.C3[1] / 2
pin_c3 = M()
for k in range(8):
    x = cx + 2.4 + 2.54 * k
    for y in (g.C3_Y + 1.3, g.C3_Y + g.C3[1] - 1.3):
        pin_c3 += g.caja(x - 0.32, x + 0.32, y - 0.32, y + 0.32, zc + 2.5, Z_TAPA - 0.1)
pieza('c3', 'tapa', 8, [
    (g.caja(cx, IN_W, g.C3_Y, g.C3_Y + g.C3[1], zc - 1.6, zc), '#1b1d26', 0),
    (g.caja(IN_W - 7.3, IN_W + 1.2, yc - 4.5, yc + 4.5, zc - 1.6 - 3.2, zc - 1.6), ACERO, 1),
    (g.caja(cx + 4, cx + 11, yc - 3.5, yc + 3.5, zc - 1.6 - 1.0, zc - 1.6), '#3a3d48', 1),
    (g.caja(cx + 1, cx + 21.4, g.C3_Y, g.C3_Y + 2.6, zc, zc + 2.5) + g.caja(cx + 1, cx + 21.4, g.C3_Y + g.C3[1] - 2.6, g.C3_Y + g.C3[1], zc, zc + 2.5), NEGRO, 0),
    (pin_c3, ORO, 1),
], [[0, 0, -45, 0]])

# ---------------- grapa de la C3 + su tornillo
pieza('grapa', 'tapa', 9, [(g.grapa(), CREMA, 0)], [[0, 5, -32, -22], [0, 2, -1.5, -22], [0, 0, 0, -22]],
      pivote=[P + 50, P + g.GRAPA_GANCHO[0] + 0.3, g.Z_GRAPA - 1.5], eje=[1, 0, 0], fin=0.6)
pieza('grapa_tornillo', 'tapa', 9, [(tornillo(g.GRAPA_X, GY, g.Z_GRAPA - 3.0, +1), ACERO, 1)], [[0, 0, -35, 0]], ini=0.55, fin=0.95)

# ---------------- amplificador MAX98357A
ax, ay, za = g.AMP_X, g.AMP_Y, Z_TAPA - g.AMP_PISO
bxs = (ax + g.AMP[0] / 2 - g.AMP_BARRENOS[0] / 2, ax + g.AMP[0] / 2 + g.AMP_BARRENOS[0] / 2)
by = ay + g.AMP[1] - g.AMP_BARRENOS[1]
pcb = g.caja(ax, ax + g.AMP[0], ay, ay + g.AMP[1], za - 1.6, za)
m3 = M()
for bx in bxs:
    pcb -= g.cil(bx, by, 1.6, za - 3, za + 1, seg=24)
    m3 += tornillo(bx, by, za - 1.6, +1, d=3.0, largo=6.0, cabeza=(5.5, 2.0))
pieza('amp', 'tapa', 10, [
    (pcb, '#6b3fa0', 0),
    (g.caja(ax + 6.5, ax + 11.5, ay + 7, ay + 12, za - 1.6 - 1.0, za - 1.6), NEGRO, 0),
    (g.caja(ax + 4, ax + 14, ay + 0.3, ay + 3.2, za - 1.6 - 3.0, za - 1.6), '#2c8a5a', 0),
], [[0, 0, -45, 0]], fin=0.55)
pieza('amp_m3', 'tapa', 10, [(m3, ACERO, 1)], [[0, 0, -40, 0]], ini=0.5, fin=0.95)

# ---------------- bateria
bat = M.cylinder(65.0, 9.1, 9.1, circular_segments=48).rotate([-90, 0, 0]).translate([P + hx + g.HOLDER[0] / 2, P + hy + (g.HOLDER[1] - 65) / 2, hz1 - 1.5 - 9.2])
polo = M.cylinder(1.2, 3.0, 3.0, circular_segments=24).rotate([-90, 0, 0]).translate([P + hx + g.HOLDER[0] / 2, P + hy + (g.HOLDER[1] - 65) / 2 + 65, hz1 - 1.5 - 9.2])
pieza('bateria', 'tapa', 12, [(bat, '#2f9c8f', 0), (polo, ACERO, 1)], [[0, 0, -45, 0]])

# ---------------- tornillos de la tapa
for n, (px, py) in enumerate(g.POSTES):
    pieza(f'tornillo_tapa{n}', 'frente', 14, [(tornillo(px, py, FONDO - 1.6, -1), ACERO, 1)], [[0, 0, 35, 0]], ini=0.25 * n, fin=0.45 + 0.25 * n)

pasos = [
    {'t': 'Tuercas M4 atrapadas', 'vista': 'ambos', 'rayos': True,
     'd': 'Esto pasa durante la impresión, no en el armado. En las pausas se meten las 6 tuercas: 2 en los postes de los puentes '
          '(pausa a 6.2 mm), 3 en los postes de las esquinas (30.6 mm) y 1 en el poste de la grapa de la tapa (11.0 mm).'},
    {'t': 'Pantalla GC9A01', 'vista': 'frente',
     'd': 'Con el frente boca abajo, la pantalla entra en su ventana con los pines hacia arriba. Céntrala antes de apretar.'},
    {'t': 'Puente de la pantalla', 'vista': 'frente',
     'd': 'Primero la punta entra inclinada en la bolsa de la pared; luego baja el bloque sobre su poste y se aprieta con un M4 × 16. '
          'Queda 1 mm libre sobre la tarjeta: ahí va un pedacito de cinta de espuma.'},
    {'t': 'Bocina de 40 mm', 'vista': 'frente',
     'd': 'Entra en su anillo, detrás de la rejilla de corazón. Un cordón de silicón en la orilla sella el frente y mejora el sonido.'},
    {'t': 'Puente de la bocina', 'vista': 'frente',
     'd': 'Igual que el de la pantalla: punta en la bolsa, bloque sobre el poste y un M4 × 16. Pisa el imán 6 mm abajo del centro.'},
    {'t': 'Switch 117S', 'vista': 'frente',
     'd': 'Entra a presión por fuera de la pared de arriba, en el corte de 19.4 × 13.2 mm.'},
    {'t': 'Holder 18650', 'vista': 'tapa',
     'd': 'Con la tapa boca arriba, el holder entra en su casita (0.1 mm por lado) y las terminales caen en los dos canales. '
          'Se fija con M3 × 6: usa los pilotos que coincidan con tu holder.'},
    {'t': 'TP4056', 'vista': 'tapa',
     'd': 'Se desliza desde la orilla, con el USB hacia afuera, bajo los dos rieles hasta el tope. Suelda sus cables después de meterlo: '
          'las bolitas de soldadura no pasan bajo los rieles.'},
    {'t': 'ESP32-C3 Super Mini', 'vista': 'tapa',
     'd': 'Con los pines ya soldados se sienta en el pedestal, que queda entre las dos filas de pines, con el USB hacia la pared.'},
    {'t': 'Grapa de la ESP32', 'vista': 'tapa',
     'd': 'Cruza sobre el conector USB: una punta entra en el gancho y la otra se aprieta con un M4 × 16 contra su tuerca atrapada.'},
    {'t': 'MAX98357A', 'vista': 'tapa',
     'd': 'Se asienta sobre sus dos postes y se atornilla con 2 M3 × 6.'},
    {'t': 'Cableado', 'vista': 'ambos',
     'd': 'Con las dos mitades abiertas se suelda todo: pantalla y amplificador a la ESP32, bocina al amplificador, y la alimentación '
          'holder → TP4056 → switch → 5V. Deja cable suficiente para abrir la tapa como libro. (Los cables no se dibujan.)'},
    {'t': 'Batería 18650', 'vista': 'tapa',
     'd': 'La pila entra en el holder. Revisa la polaridad y deja el switch apagado.'},
    {'t': 'Cerrar la tapa', 'vista': 'cierre',
     'd': 'La tapa se voltea y su ceja entra en el frente. Al cerrar, la pared ya no deja salir al TP4056 y los dos USB-C quedan en sus ventanas.'},
    {'t': 'Tornillos de la tapa', 'vista': 'atras',
     'd': 'Tres M4 × 16 por la espalda, con la cabeza embutida 1.6 mm, entran en las tuercas atrapadas de los postes.'},
    {'t': 'Latido armado', 'vista': 'final',
     'd': 'El aparato se para solo: 59.3 × 95.8 × 41 mm. Usa “Ver por dentro” para revisar cómo quedó todo acomodado.'},
]

escena = {'W': g.W, 'H': g.H, 'FONDO': FONDO, 'piezas': piezas, 'pasos': pasos, 'PASO_CIERRE': 13, 'PASO_FINAL': 15}
html = (O / 'simulacion_plantilla.html').read_text(encoding='utf-8').replace('/*DATOS*/null', json.dumps(escena, ensure_ascii=False))
CABEZA = '<!doctype html>\n<html lang="es">\n<meta charset="utf-8">\n<meta name="viewport" content="width=device-width, initial-scale=1">\n'
(O / 'Simulacion_Ensamble.html').write_text(CABEZA + html, encoding='utf-8')
print(f'Simulacion_Ensamble.html: {len(html) / 1024:.0f} KB, {len(piezas)} piezas, {len(pasos)} pasos')

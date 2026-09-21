#!/usr/bin/env python3
"""
============================================================
 GENERADOR DE RECURSOS - Bascula Multiusos
 Desarrollado por Tostatronic - Ing. Jorge Alvarado
 www.tostatronic.com
============================================================

Genera dos archivos del sketch a partir de los originales:

  fuentes.h  <- tipografia Barlow (SIL Open Font License) convertida
                al formato GFXfont de 1 bit que usa Arduino_GFX.
  logo.h     <- logo de Tostatronic en RGB565, ya mezclado sobre el
                color de fondo de la pantalla.

Uso (desde la carpeta Bascula_Multiusos/):

    python3 docs/herramientas/generar_recursos.py

Requiere:  pip install pillow fonttools

Por que fuentes propias: las que trae Arduino_GFX no tienen acentos
ni un tamano grande para el peso. Aqui se generan con el rango
Latin-1 completo (a, e, n con tilde, etc.).

Las fuentes de NUMEROS usan las cifras tabulares de Barlow (la
funcion OpenType "tnum"): todos los digitos miden lo mismo, y el "1"
es un dibujo hecho para ese ancho. Asi el peso no "baila" cuando
cambia de 1 a 8, y tampoco quedan huecos alrededor del 1.
"""

import tempfile
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
from fontTools.ttLib import TTFont

AQUI    = Path(__file__).resolve().parent
RAIZ    = AQUI.parent.parent                       # Bascula_Multiusos/
SKETCH  = RAIZ / "Bascula_Multiusos"
FUENTES = AQUI / "fuentes"
LOGO    = RAIZ / "docs" / "logo-tostatronic-claro.png"

FONDO_RGB = (0x10, 0x15, 0x19)     # mismo valor que COLOR_FONDO en pantalla.h

# Caracteres fuera de Latin-1 que la interfaz necesita. Se guardan en
# el hueco 0x80-0x9F (que en Latin-1 son controles sin dibujo). La capa
# de pantalla hace la misma traduccion al decodificar UTF-8.
EXTRAS = {0x80: "≈",   # aproximadamente igual (calorias)
          0x81: "•",   # bala
          0x82: "…"}   # puntos suspensivos

# nombre, archivo, alto en px (None = autoajuste), primer y ultimo caracter,
# cifras tabulares (True en las fuentes con las que se escriben cantidades)
TIPOS = [
    ("FuenteNumero", "BarlowSemiCondensed-Bold.ttf", None, 0x20, 0x3A, True),
    ("FuenteTitulo", "Barlow-SemiBold.ttf",          24,   0x20, 0xFF, True),
    ("FuenteTexto",  "Barlow-Medium.ttf",            18,   0x20, 0xFF, False),
    ("FuenteChica",  "Barlow-Medium.ttf",            14,   0x20, 0xFF, False),
]

# El numero grande debe caber en la cuerda del circulo a la altura
# del centro, con el anillo perimetral y un margen: 7 caracteres
# ("1000.00" con celda de 1 kg, "20000.0" con la de 20 kg).
NUMERO_MUESTRA   = "8888.88"
NUMERO_ANCHO_MAX = 192


def caracter(codigo):
    if codigo in EXTRAS:
        return EXTRAS[codigo]
    if 0x7F <= codigo <= 0x9F:
        return None
    return bytes([codigo]).decode("latin-1")


_temporales = tempfile.TemporaryDirectory()


def ruta_fuente(archivo, tabular):
    """Ruta del TTF a usar. Con tabular=True devuelve una copia temporal en
    la que los digitos apuntan a sus variantes "tnum". Pillow solo aplica
    funciones OpenType si trae libraqm (casi nunca), asi que se resuelve
    antes: se cambia el mapa de caracteres y listo."""
    original = FUENTES / archivo
    if not tabular:
        return str(original)

    tt = TTFont(original)
    gsub = tt["GSUB"].table
    cambios = {}
    for registro in gsub.FeatureList.FeatureRecord:
        if registro.FeatureTag != "tnum":
            continue
        for indice in registro.Feature.LookupListIndex:
            for sub in gsub.LookupList.Lookup[indice].SubTable:
                sub = getattr(sub, "ExtSubTable", sub)
                cambios.update(getattr(sub, "mapping", {}))
    if not cambios:
        raise SystemExit(f"{archivo} no trae cifras tabulares (tnum)")

    for tabla in tt["cmap"].tables:
        for codigo, glifo in list(tabla.cmap.items()):
            if glifo in cambios:
                tabla.cmap[codigo] = cambios[glifo]
    destino = Path(_temporales.name) / f"tnum-{archivo}"
    tt.save(destino)
    return str(destino)


def ancho_texto(fuente, texto, avance_digito):
    return sum(avance_digito if c.isdigit() else round(fuente.getlength(c)) for c in texto)


def avance_digitos(fuente):
    return max(round(fuente.getlength(d)) for d in "0123456789")


def autoajustar(ruta):
    """El alto mas grande con el que NUMERO_MUESTRA cabe en NUMERO_ANCHO_MAX."""
    for px in range(90, 20, -1):
        f = ImageFont.truetype(ruta, px)
        if ancho_texto(f, NUMERO_MUESTRA, avance_digitos(f)) <= NUMERO_ANCHO_MAX:
            return px
    raise SystemExit("No se pudo ajustar la fuente del numero")


def convertir(nombre, archivo, px, primero, ultimo, tabular):
    ruta = ruta_fuente(archivo, tabular)
    if px is None:
        px = autoajustar(ruta)
    fuente = ImageFont.truetype(ruta, px)
    av_dig = avance_digitos(fuente)

    bits, glifos = [], []
    for codigo in range(primero, ultimo + 1):
        c = caracter(codigo)
        desplazamiento = len(bits) // 8
        if c is None:
            glifos.append((desplazamiento, 0, 0, 0, 0, 0))
            continue

        avance = round(fuente.getlength(c))
        # Caja relativa a la linea base (ancla 'ls' = left, baseline)
        x0, y0, x1, y1 = fuente.getbbox(c, anchor="ls")
        w, h = x1 - x0, y1 - y0

        if tabular and c.isdigit():           # el redondeo no debe romper el ancho fijo
            x0 += (av_dig - avance) // 2
            avance = av_dig

        if w <= 0 or h <= 0:                  # espacio y similares
            glifos.append((desplazamiento, 0, 0, avance, 0, 0))
            continue

        lienzo = Image.new("1", (w, h), 0)
        lapiz = ImageDraw.Draw(lienzo)
        lapiz.fontmode = "1"                  # sin suavizado: la fuente es de 1 bit
        lapiz.text((-fuente.getbbox(c, anchor="ls")[0], -y0), c, font=fuente, fill=1, anchor="ls")

        # Formato Adafruit/Arduino_GFX: bits corridos, SIN alinear cada renglon
        # a byte; solo se rellena al final de cada glifo.
        px_glifo = lienzo.load()
        for y in range(h):
            for x in range(w):
                bits.append(1 if px_glifo[x, y] else 0)
        while len(bits) % 8:
            bits.append(0)

        assert w < 256 and h < 256 and -128 <= x0 < 128 and -128 <= y0 < 128
        glifos.append((desplazamiento, w, h, avance, x0, y0))

    datos = bytes(int("".join(map(str, bits[i:i + 8])), 2) for i in range(0, len(bits), 8))
    assert len(datos) < 65536, f"{nombre}: el mapa de bits no cabe en bitmapOffset (uint16)"

    ascenso, descenso = fuente.getmetrics()
    salida = [f"// {nombre}: {archivo} a {px} px{', cifras tabulares' if tabular else ''}  "
              f"({len(datos)} bytes de mapa, {len(glifos)} glifos)"]
    salida.append(f"const uint8_t {nombre}Mapa[] PROGMEM = {{")
    for i in range(0, len(datos), 16):
        salida.append("  " + ", ".join(f"0x{b:02X}" for b in datos[i:i + 16]) + ",")
    salida.append("};")
    salida.append(f"const GFXglyph {nombre}Glifos[] PROGMEM = {{")
    for i, g in enumerate(glifos):
        salida.append("  {{ {:5d}, {:3d}, {:3d}, {:3d}, {:4d}, {:4d} }},   // 0x{:02X}".format(*g, primero + i))
    salida.append("};")
    salida.append(f"const GFXfont {nombre} PROGMEM = {{ (uint8_t*){nombre}Mapa, (GFXglyph*){nombre}Glifos, "
                  f"0x{primero:02X}, 0x{ultimo:02X}, {ascenso + descenso} }};")
    # Alto de las mayusculas/digitos: sirve para centrar texto en vertical.
    alto_may = -fuente.getbbox("8", anchor="ls")[1]
    salida.append(f"const uint8_t {nombre}AltoMayus = {alto_may};")
    print(f"  {nombre:13s} {px:3d} px  alto de digito {alto_may:2d} px  {len(datos):6d} bytes")
    return "\n".join(salida), px


def generar_fuentes():
    print("Fuentes:")
    bloques = [convertir(*t)[0] for t in TIPOS]
    extras = ", ".join(f"U+{ord(v):04X} -> 0x{k:02X}" for k, v in EXTRAS.items())
    cabecera = f"""/*
 * ============================================================
 *  FUENTES DE LA BASCULA MULTIUSOS  (archivo GENERADO, no editar)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Se genera con docs/herramientas/generar_recursos.py
 *
 *  Tipografia: Barlow, de Jeremy Tribby (The Barlow Project Authors),
 *  bajo SIL Open Font License 1.1 - ver docs/herramientas/fuentes/OFL.txt
 *
 *  Formato GFXfont de 1 bit. Rango Latin-1, con estos extras en el
 *  hueco 0x80-0x9F:  {extras}
 *  FuenteNumero y FuenteTitulo usan cifras tabulares (ancho fijo) para
 *  que las cantidades no "bailen"; FuenteTexto y FuenteChica, no.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include <gfxfont.h>
"""
    (SKETCH / "fuentes.h").write_text(cabecera + "\n" + "\n\n".join(bloques) + "\n", encoding="utf8")


def generar_logo(ancho=150):
    if not LOGO.exists():
        print(f"AVISO: no existe {LOGO}; no se genera logo.h")
        return
    original = Image.open(LOGO).convert("RGBA")
    original = original.crop(original.getbbox())          # fuera el margen transparente
    alto = round(original.height * ancho / original.width)
    chico = original.resize((ancho, alto), Image.LANCZOS)
    fondo = Image.new("RGBA", chico.size, FONDO_RGB + (255,))
    rgb = Image.alpha_composite(fondo, chico).convert("RGB")

    crudo = rgb.tobytes()                                  # R, G, B, R, G, B...
    valores = [((crudo[i] & 0xF8) << 8) | ((crudo[i + 1] & 0xFC) << 3) | (crudo[i + 2] >> 3)
               for i in range(0, len(crudo), 3)]
    lineas = ["  " + ", ".join(f"0x{v:04X}" for v in valores[i:i + 12]) + "," for i in range(0, len(valores), 12)]
    texto = f"""/*
 * ============================================================
 *  LOGO DE TOSTATRONIC EN RGB565  (archivo GENERADO, no editar)
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  Se genera con docs/herramientas/generar_recursos.py a partir de
 *  docs/logo-tostatronic-claro.png, ya mezclado sobre COLOR_FONDO
 *  (la pantalla no maneja transparencia).
 * ============================================================
 */

#pragma once
#include <Arduino.h>

const uint16_t LOGO_ANCHO = {ancho};
const uint16_t LOGO_ALTO  = {alto};

const uint16_t LOGO_PIXELES[] PROGMEM = {{
{chr(10).join(lineas)}
}};
"""
    (SKETCH / "logo.h").write_text(texto, encoding="utf8")
    print(f"Logo: {ancho}x{alto} px, {len(valores) * 2} bytes")


if __name__ == "__main__":
    generar_fuentes()
    generar_logo()
    print("Listo.")

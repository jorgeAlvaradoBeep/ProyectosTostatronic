#!/usr/bin/env bash
# Compila y corre el simulador; deja las capturas en salida/*.png
# Desarrollado por Tostatronic - Ing. Jorge Alvarado
set -e
cd "$(dirname "$0")"
SK=../../../Bascula_Multiusos
mkdir -p salida
c++ -std=c++17 -O1 -w -DCONFIG_IDF_TARGET_ESP32C6 -Ifalsos -I"$SK" simulador.cpp "$SK/ui.cpp" "$SK/pantalla.cpp" -o salida/simulador
./salida/simulador
python3 - <<'PY'
# PPM -> PNG con la mascara circular de la GC9A01 y una hoja con todas.
import glob
from PIL import Image, ImageDraw
pngs = []
for ruta in sorted(glob.glob("salida/*.ppm")):
    im = Image.open(ruta).convert("RGB").resize((480, 480), Image.NEAREST)
    mascara = Image.new("L", im.size, 0); ImageDraw.Draw(mascara).ellipse((0, 0, 479, 479), fill=255)
    fondo = Image.new("RGB", im.size, (60, 60, 60)); fondo.paste(im, mask=mascara)
    fondo.save(ruta.replace(".ppm", ".png")); pngs.append(fondo)
hoja = Image.new("RGB", (480 * 3, 480 * ((len(pngs) + 2) // 3)), (60, 60, 60))
for i, p in enumerate(pngs): hoja.paste(p, ((i % 3) * 480, (i // 3) * 480))
hoja.save("salida/hoja.png"); print("hoja:", hoja.size)
PY
rm -f salida/*.ppm salida/simulador

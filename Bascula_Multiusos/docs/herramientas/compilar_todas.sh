#!/usr/bin/env bash
# ============================================================
#  Compila el sketch para las 4 placas soportadas.
#  Desarrollado por Tostatronic - Ing. Jorge Alvarado
# ============================================================
#  Uso (desde cualquier carpeta):
#      bash docs/herramientas/compilar_todas.sh
#
#  Requiere arduino-cli con el core esp32 >= 3.3.0. La placa se
#  elige pasando -DPLACA=n, sin tocar config.h.
# ============================================================

set -u
SKETCH="$(cd "$(dirname "$0")/../../Bascula_Multiusos" && pwd)"

#        PLACA  chip     opciones extra del FQBN          nombre
PLACAS=( "1     esp32c6  -                                C6-DevKit"
         "2     esp32c6  CDCOnBoot=cdc                    C6-Mini"
         "3     esp32c5  -                                C5-DevKit"
         "4     esp32c5  CDCOnBoot=cdc                    C5-Mini" )

fallas=0
for fila in "${PLACAS[@]}"; do
  read -r placa chip extra nombre <<< "$fila"
  fqbn="esp32:esp32:${chip}:PartitionScheme=huge_app"
  [ "$extra" != "-" ] && fqbn="${fqbn},${extra}"

  echo "=== ${nombre}  (${fqbn})"
  salida=$(arduino-cli compile --fqbn "$fqbn" --warnings default \
           --build-property "compiler.cpp.extra_flags=-DPLACA=${placa}" "$SKETCH" 2>&1)
  estado=$?

  # Solo lo nuestro: los avisos de las librerias ajenas no se pueden arreglar aqui.
  echo "$salida" | grep -E "error|warning" | grep -E "Bascula_Multiusos/" | head -20
  echo "$salida" | grep -E "Sketch usa|Sketch uses|Globales|Global variables"
  if [ $estado -ne 0 ]; then
    echo "$salida" | tail -15
    echo ">>> FALLO ${nombre}"
    fallas=$((fallas + 1))
  fi
done

echo
[ $fallas -eq 0 ] && echo "Las 4 placas compilan." || echo "${fallas} placa(s) con error."
exit $fallas

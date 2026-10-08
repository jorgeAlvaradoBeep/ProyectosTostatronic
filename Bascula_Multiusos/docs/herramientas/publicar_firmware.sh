#!/usr/bin/env bash
# ============================================================
#  Publica el firmware en el servidor de actualizaciones (VPS)
#  Desarrollado por Tostatronic - Ing. Jorge Alvarado
# ============================================================
#  Uso (desde cualquier carpeta; en Windows, desde Git Bash):
#      bash docs/herramientas/publicar_firmware.sh --notas "Qué trae de nuevo"
#
#  Opciones:
#    --notas "texto"      Una línea con lo nuevo. Sale en la pantalla de la
#                         báscula al ofrecer la actualización (máx. 40 letras).
#    --placas 1,2         Solo estas placas (1 C6-DevKit, 2 C6-Mini, 3 C5-DevKit,
#                         4 C5-Mini). Por omisión, las 4. Las que no se publican
#                         conservan su entrada del versiones.json del servidor.
#    --sin-compilar       No compila: usa los .bin que ya están en la carpeta de
#                         salida (build/publicar/Bascula_v<versión>_<placa>.bin).
#    --sin-subir          Compila y arma versiones.json, pero no toca el VPS.
#    --vps usuario@host   Por omisión root@143.198.173.21
#    --ruta /carpeta      En el VPS. Por omisión /var/www/html/updates/bascula
#    --url http://.../    Dónde queda publicado (para verificar y para fusionar).
#                         Por omisión la misma que OTA_SERVIDOR_URL de config.h.
#
#  Qué hace:
#    1. Lee PROYECTO_VERSION de config.h: esa es la versión que se publica.
#    2. Compila cada placa con arduino-cli (Minimal SPIFFS, -DPLACA=n) y
#       deja build/publicar/Bascula_v<versión>_<placa>.bin
#    3. Calcula tamaño y SHA-256 de cada .bin y arma versiones.json, con
#       la misma forma que espera actualizador.cpp.
#    4. Sube primero los .bin y AL FINAL versiones.json (así ninguna
#       báscula ve una versión anunciada cuyo archivo aún no está).
#    5. Verifica por HTTP que el servidor entregue lo que se subió.
#
#  Los .bin viejos se quedan en el VPS (el nombre lleva la versión): para
#  regresar a una versión anterior basta volver a apuntar versiones.json a
#  ellos (ver docs/ACTUALIZACION_VPS.md).
#
#  Requiere: arduino-cli (sirve el que trae el Arduino IDE 2), ssh/scp con
#  llave para el VPS, curl, sha256sum. Para --placas parcial hace falta
#  python (python3) para fusionar con el versiones.json del servidor.
# ============================================================

set -u

AQUI="$(cd "$(dirname "$0")" && pwd)"
SKETCH="$(cd "$AQUI/../../Bascula_Multiusos" && pwd)"
SALIDA="$SKETCH/build/publicar"

NOTAS=""
PLACAS_PEDIDAS="1,2,3,4"
COMPILAR=1
SUBIR=1
VPS="root@143.198.173.21"
RUTA="/var/www/html/updates/bascula"
URL=""

while [ $# -gt 0 ]; do
  case "$1" in
    --notas)        NOTAS="$2"; shift 2 ;;
    --placas)       PLACAS_PEDIDAS="$2"; shift 2 ;;
    --sin-compilar) COMPILAR=0; shift ;;
    --sin-subir)    SUBIR=0; shift ;;
    --vps)          VPS="$2"; shift 2 ;;
    --ruta)         RUTA="$2"; shift 2 ;;
    --url)          URL="$2"; shift 2 ;;
    -h|--help)      sed -n '2,40p' "$0"; exit 0 ;;
    *) echo "Opción desconocida: $1 (usa --help)"; exit 2 ;;
  esac
done

# ------------------ herramientas ------------------

if [ -z "${ARDUINO_CLI:-}" ]; then
  if command -v arduino-cli >/dev/null 2>&1; then
    ARDUINO_CLI="arduino-cli"
  elif [ -n "${LOCALAPPDATA:-}" ] && [ -x "$LOCALAPPDATA/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe" ]; then
    ARDUINO_CLI="$LOCALAPPDATA/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe"
  else
    ARDUINO_CLI=""
  fi
fi

# sha archivo   (o)   algo | sha     -> solo el hexadecimal
if command -v sha256sum >/dev/null 2>&1; then  sha() { sha256sum "$@" | cut -d' ' -f1; }
elif command -v shasum >/dev/null 2>&1; then   sha() { shasum -a 256 "$@" | cut -d' ' -f1; }
else echo "Falta sha256sum (o shasum)."; exit 1; fi

PYTHON=""
for p in python3 python; do command -v "$p" >/dev/null 2>&1 && { PYTHON="$p"; break; }; done

# ------------------ versión ------------------

VERSION=$(sed -n 's/^#define PROYECTO_VERSION *"\([^"]*\)".*/\1/p' "$SKETCH/config.h")
[ -z "$VERSION" ] && { echo "No encuentro PROYECTO_VERSION en config.h"; exit 1; }
if [ -z "$URL" ]; then
  URL=$(sed -n 's/^#define OTA_SERVIDOR_URL *"\([^"]*\)".*/\1/p' "$SKETCH/config.h")
fi
ARCHIVO_VERSIONES=$(sed -n 's/^#define OTA_ARCHIVO_VERSIONES *"\([^"]*\)".*/\1/p' "$SKETCH/config.h")
ARCHIVO_VERSIONES="${ARCHIVO_VERSIONES:-versiones.json}"

if [ ${#NOTAS} -gt 40 ]; then
  echo "Las notas tienen ${#NOTAS} letras; en la pantalla redonda caben unas 40. Recórtalas."
  exit 1
fi

echo "Versión a publicar: v$VERSION"
echo "Carpeta de salida:  $SALIDA"
mkdir -p "$SALIDA"

#        PLACA  chip     opciones extra del FQBN   nombre      id (ID_PLACA en config.h)
PLACAS=( "1     esp32c6  -                         C6-DevKit   c6-devkit"
         "2     esp32c6  CDCOnBoot=cdc             C6-Mini     c6-mini"
         "3     esp32c5  -                         C5-DevKit   c5-devkit"
         "4     esp32c5  CDCOnBoot=cdc             C5-Mini     c5-mini" )

pedida() { case ",$PLACAS_PEDIDAS," in *",$1,"*) return 0 ;; *) return 1 ;; esac; }

# ------------------ compilar ------------------

if [ $COMPILAR -eq 1 ]; then
  [ -z "$ARDUINO_CLI" ] && { echo "No encuentro arduino-cli. Instálalo, o exporta ARDUINO_CLI=/ruta/arduino-cli (o usa --sin-compilar)."; exit 1; }
  for fila in "${PLACAS[@]}"; do
    read -r placa chip extra nombre id <<< "$fila"
    pedida "$placa" || continue
    fqbn="esp32:esp32:${chip}:PartitionScheme=min_spiffs"
    [ "$extra" != "-" ] && fqbn="${fqbn},${extra}"
    temporal="$SALIDA/tmp_${nombre}"
    rm -rf "$temporal"

    echo
    echo "=== Compilando ${nombre}  (${fqbn})"
    if ! salida=$("$ARDUINO_CLI" compile --fqbn "$fqbn" --warnings default \
                  --build-property "compiler.cpp.extra_flags=-DPLACA=${placa}" \
                  --output-dir "$temporal" "$SKETCH" 2>&1); then
      echo "$salida" | tail -25
      echo ">>> FALLÓ ${nombre}. No se publica nada."
      exit 1
    fi
    echo "$salida" | grep -E "Sketch usa|Sketch uses" || true
    bin=$(ls "$temporal"/*.ino.bin 2>/dev/null | head -1)
    [ -z "$bin" ] && { echo ">>> No salió el .bin de ${nombre}"; exit 1; }
    mv -f "$bin" "$SALIDA/Bascula_v${VERSION}_${nombre}.bin"
    rm -rf "$temporal"
  done
fi

# ------------------ versiones.json ------------------

# Escapa comillas y diagonales para meter texto en JSON.
json_texto() { printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g'; }

ENTRADAS=""
ARCHIVOS=()
for fila in "${PLACAS[@]}"; do
  read -r placa chip extra nombre id <<< "$fila"
  pedida "$placa" || continue
  archivo="Bascula_v${VERSION}_${nombre}.bin"
  ruta="$SALIDA/$archivo"
  [ -f "$ruta" ] || { echo "Falta $ruta (compila, o quita la placa $placa de --placas)."; exit 1; }
  tamano=$(wc -c < "$ruta" | tr -d ' ')
  suma=$(sha "$ruta")
  echo "$nombre: $archivo  $tamano bytes  sha256 $suma"
  [ -n "$ENTRADAS" ] && ENTRADAS="$ENTRADAS,"
  ENTRADAS="$ENTRADAS
  \"$id\": {
    \"version\": \"$(json_texto "$VERSION")\",
    \"archivo\": \"$archivo\",
    \"tamano\": $tamano,
    \"sha256\": \"$suma\",
    \"notas\": \"$(json_texto "$NOTAS")\",
    \"fecha\": \"$(date +%Y-%m-%d)\"
  }"
  ARCHIVOS+=("$ruta")
done

NUEVO="$SALIDA/$ARCHIVO_VERSIONES"
printf '{%s\n}\n' "$ENTRADAS" > "$NUEVO"

# Publicación parcial: las placas que no se tocan conservan lo que ya dice el servidor.
if [ "$PLACAS_PEDIDAS" != "1,2,3,4" ] && [ $SUBIR -eq 1 ]; then
  [ -z "$PYTHON" ] && { echo "Para publicar solo algunas placas hace falta python (fusiona con el versiones.json del servidor)."; exit 1; }
  ANTERIOR="$SALIDA/versiones.servidor.json"
  if curl -fsS -m 20 -o "$ANTERIOR" "${URL}${ARCHIVO_VERSIONES}"; then
    "$PYTHON" - "$ANTERIOR" "$NUEVO" <<'PY' || exit 1
import json, sys
anterior = json.load(open(sys.argv[1], encoding="utf-8"))
nuevo    = json.load(open(sys.argv[2], encoding="utf-8"))
anterior.update(nuevo)
json.dump(anterior, open(sys.argv[2], "w", encoding="utf-8"), indent=2, ensure_ascii=False)
print("Fusionado con el versiones.json del servidor:", ", ".join(sorted(anterior)))
PY
  else
    echo "El servidor aún no tiene $ARCHIVO_VERSIONES: se publica solo con las placas pedidas."
  fi
fi

echo
echo "---- $ARCHIVO_VERSIONES ----"
cat "$NUEVO"
echo "----------------------------"

[ $SUBIR -eq 0 ] && { echo "Sin subir (--sin-subir). Todo quedó en $SALIDA"; exit 0; }

# ------------------ subir ------------------

echo
echo "Subiendo a $VPS:$RUTA"
ssh -o BatchMode=yes "$VPS" "mkdir -p '$RUTA' && chmod 755 '$RUTA'" || { echo "No pude entrar al VPS por SSH (¿llave?)."; exit 1; }
scp "${ARCHIVOS[@]}" "$VPS:$RUTA/" || { echo "Falló la subida de los .bin"; exit 1; }
# El índice va al final y se cambia de un golpe: nunca queda a medias.
scp "$NUEVO" "$VPS:$RUTA/$ARCHIVO_VERSIONES.nuevo" && \
ssh "$VPS" "cd '$RUTA' && chmod 644 *.bin '$ARCHIVO_VERSIONES.nuevo' && mv -f '$ARCHIVO_VERSIONES.nuevo' '$ARCHIVO_VERSIONES'" \
  || { echo "Falló la subida de $ARCHIVO_VERSIONES"; exit 1; }

# ------------------ verificar ------------------

echo
echo "Verificando en ${URL}"
fallas=0
if ! curl -fsS -m 20 "${URL}${ARCHIVO_VERSIONES}" | grep -q "\"$VERSION\""; then
  echo ">>> ${URL}${ARCHIVO_VERSIONES} no muestra la versión $VERSION"; fallas=1
fi
for ruta in "${ARCHIVOS[@]}"; do
  archivo=$(basename "$ruta")
  remoto=$(curl -fsS -m 120 "${URL}${archivo}" | sha 2>/dev/null) || true
  local_=$(sha "$ruta")
  if [ "$remoto" = "$local_" ]; then echo "ok  $archivo"
  else echo ">>> $archivo: el servidor entrega otro contenido (o no lo entrega)"; fallas=1; fi
done

if [ $fallas -eq 0 ]; then
  echo
  echo "Publicada la v$VERSION. Las básculas la ofrecerán al arrancar (MENÚ → Conexión → Buscar versión la busca al momento)."
else
  echo
  echo "Se subió, pero la verificación por HTTP falló. Revisa docs/ACTUALIZACION_VPS.md (sección 'Si algo falla')."
  exit 1
fi

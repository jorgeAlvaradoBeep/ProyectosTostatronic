# Actualización de la báscula desde el VPS — guía para ejecutar

Guía para dejar funcionando, **desde la máquina que tiene acceso SSH al VPS
`143.198.173.21`**, el servidor de actualizaciones de la Báscula Multiusos.
Escrita el 8 de octubre de 2026 para el firmware **v0.5**.

> Si quien lee esto es Claude Code: todo lo que hay que correr está en la
> sección **"Paso a paso"**. Corre los comandos en orden, verifica cada
> salida y, al final, deja la sección **"Resultado"** llena.

---

## Qué se va a montar

El mismo VPS donde el POS publica sus actualizaciones (DigitalOcean,
`143.198.173.21`, carpeta `/var/www/html/updates/` servida por HTTP en el
puerto 80) va a tener una carpeta más:

```
/var/www/html/updates/bascula/
├── versiones.json                 ← qué versión hay para cada placa
├── Bascula_v0.5_C6-DevKit.bin
├── Bascula_v0.5_C6-Mini.bin
├── Bascula_v0.5_C5-DevKit.bin
└── Bascula_v0.5_C5-Mini.bin
```

**Cómo lo usa la báscula (firmware ≥ 0.5):**

1. Al arrancar, en cuanto se conecta al WiFi, pide
   `http://143.198.173.21/updates/bascula/versiones.json` y lee la entrada de
   **su** placa (`c6-devkit`, `c6-mini`, `c5-devkit` o `c5-mini`).
2. Si la versión publicada es más nueva que la instalada, la pantalla muestra
   **"Nueva versión v0.6 — OK: instalar / MENÚ: ahora no"**. Nunca instala
   sola. Si se pospone, queda en **MENÚ → Conexión → Instalar v0.6**.
3. Con OK descarga el `.bin` directo a la segunda partición de la Flash,
   calculando el SHA-256 al vuelo. Si no coincide con el anunciado, o el chip
   no da por buena la imagen, no cambia nada.
4. Reinicia con el firmware nuevo "a prueba": si no corre 10 segundos, la
   placa regresa sola al anterior.

Sin internet, o con el VPS caído, el arranque es normal: la báscula no espera
ni avisa nada (el error solo se ve si uno entra a **Conexión → Buscar
versión**).

**`versiones.json`** (lo genera el script; aquí para saber qué esperar):

```json
{
  "c6-mini": {
    "version": "0.5",
    "archivo": "Bascula_v0.5_C6-Mini.bin",
    "tamano": 1318912,
    "sha256": "9f1c…",
    "notas": "Actualización desde el servidor",
    "fecha": "2026-10-08"
  },
  "c6-devkit": { … }, "c5-devkit": { … }, "c5-mini": { … }
}
```

- `version` se compara número a número (`0.10` es más nueva que `0.9`).
- `archivo` es relativo a la misma carpeta; no admite `/`.
- `tamano` y `sha256` (64 hexadecimales, minúsculas) son obligatorios: sin
  ellos la báscula reporta "Lista de versiones incompleta".
- `notas` es una línea de hasta 40 letras: sale en la pantalla redonda.

---

## Qué hace falta en la máquina

| | Cómo comprobarlo |
|---|---|
| Acceso SSH al VPS como `root` (la misma llave con la que se publica el POS) | `ssh -o BatchMode=yes root@143.198.173.21 'echo ok'` → `ok` |
| El repo `ProyectosTostatronic` actualizado (con `Bascula_Multiusos` en v0.5) | `grep PROYECTO_VERSION Bascula_Multiusos/Bascula_Multiusos/config.h` → `"0.5"` |
| `arduino-cli` con el core **esp32 ≥ 3.3.0** y las librerías **GFX Library for Arduino ≥ 1.6.5** y **ArduinoJson ≥ 7** | `arduino-cli core list` y `arduino-cli lib list` |
| Git Bash (Windows) o bash (Linux/mac), con `curl` y `sha256sum` | vienen con Git for Windows |

Sobre `arduino-cli`: si está instalado el **Arduino IDE 2**, ya trae uno y el
script lo encuentra solo en
`%LOCALAPPDATA%\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe`.
Si está en otro lado: `export ARDUINO_CLI=/ruta/al/arduino-cli`.

Si **no hay forma de compilar** en esa máquina, se puede compilar en otra con
el Arduino IDE (*Programa → Exportar binario compilado*, una vez por placa,
cambiando `PLACA` en `config.h`), copiar los `.bin` a
`Bascula_Multiusos/Bascula_Multiusos/build/publicar/` con los nombres
`Bascula_v0.5_C6-DevKit.bin`, `Bascula_v0.5_C6-Mini.bin`,
`Bascula_v0.5_C5-DevKit.bin`, `Bascula_v0.5_C5-Mini.bin`, y correr el script
con `--sin-compilar`.

---

## Paso a paso

Todo desde la raíz del repo `ProyectosTostatronic`, en bash / Git Bash.

### 1. Comprobar el acceso al VPS y el servidor web

```bash
ssh -o BatchMode=yes root@143.198.173.21 'echo ok; ls -la /var/www/html/updates | head'
curl -sI http://143.198.173.21/updates/tostatronic-pos.xml | head -5
```

Se espera `ok`, la lista con `TostatronicPOSSetUp.msi` y `tostatronic-pos.xml`,
y un `HTTP/1.1 200 OK` del `curl`. (Verificado el 2026-10-08: el servidor es
**Apache 2.4** sirviendo `/var/www/html` en el puerto 80; Caddy atiende el 443.
La llave `jorge-pc-tostatronic` de la PC de Jorge ya está dada de alta.)

### 2. Crear la carpeta de la báscula (una sola vez)

```bash
ssh root@143.198.173.21 'mkdir -p /var/www/html/updates/bascula && chmod 755 /var/www/html/updates/bascula && ls -ld /var/www/html/updates/bascula'
```

No hace falta configurar nada más en el servidor web: `.json` y `.bin` se
sirven como archivos estáticos igual que el `.msi` y el `.xml` del POS. Solo
hay un detalle a revisar en el paso 4: que el `.json` llegue como texto y el
`.bin` como binario, sin que el servidor le aplique nada raro (compresión,
redirección a HTTPS, etc.).

### 3. Compilar y publicar la v0.5

```bash
cd Bascula_Multiusos
bash docs/herramientas/publicar_firmware.sh --notas "Actualización desde el servidor"
```

Tarda unos **12 minutos** (3 por placa). El script:

- compila las 4 placas con *Minimal SPIFFS* y deja los `.bin` en
  `Bascula_Multiusos/build/publicar/`;
- arma `versiones.json` con tamaño y SHA-256 de cada uno;
- sube los `.bin` y al final el `versiones.json` (de un golpe, con `mv`);
- verifica por HTTP que el servidor entregue exactamente lo subido.

Debe terminar con **`Publicada la v0.5`**. Si alguna placa no compila, no
sube nada (ver "Si algo falla").

Para probar sin tocar el VPS: `--sin-subir`. Para publicar solo una placa (por
ejemplo la C6 Mini, que es la que hay físicamente): `--placas 2`; las demás
conservan su entrada del `versiones.json` del servidor (requiere `python`).

### 4. Verificar a mano

```bash
curl -s http://143.198.173.21/updates/bascula/versiones.json
curl -sI http://143.198.173.21/updates/bascula/Bascula_v0.5_C6-Mini.bin | grep -iE "HTTP/|content-length|content-type|content-encoding|location"
```

Se espera:

- el JSON con las 4 placas y `"version": "0.5"`;
- `HTTP/1.1 200 OK`, `Content-Length` igual al `tamano` del JSON,
  **sin** `Content-Encoding` (nada de gzip sobre el `.bin`) y **sin**
  `Location` (nada de redirigir a HTTPS: la báscula pide por HTTP plano).

### 5. Probar con la báscula

La báscula debe correr ya un firmware **≥ 0.5** (la v0.4 no sabe consultar el
servidor): cárgale la v0.5 una vez por USB o por la página web
(`http://tostabascula.local/actualizar`, con el permiso abierto en
**MENÚ → Conexión → OK → Subir firmware**).

Con la v0.5 puesta y el WiFi conectado:

- **MENÚ → Conexión → OK → Buscar versión** debe decir **"Ya tienes la versión
  más reciente · servidor: v0.5"**. Eso confirma que la báscula llega al VPS
  y entiende el `versiones.json`.
- Para ver la oferta completa, publica una **v0.6** (sube `PROYECTO_VERSION`
  en `config.h`, cualquier cambio menor, y repite el paso 3). Al reiniciar la
  báscula, a los pocos segundos de conectarse aparece **"Nueva versión v0.6"**;
  con OK descarga (porcentaje y anillo), reinicia y arranca ya en v0.6. En
  **Ajustes** de la página web, el renglón **Servidor** dice lo mismo.
- Por el monitor serie (115200) todo sale con el prefijo `[actualizador]`.

---

## Publicar versiones nuevas (de aquí en adelante)

```
subir PROYECTO_VERSION en config.h   (p. ej. "0.6")
   → bash docs/herramientas/publicar_firmware.sh --notas "Modo calorías"
   → las básculas lo ofrecen al arrancar
```

Nada más. La versión sale de `config.h`, así que no hay dos lugares que
mantener sincronizados.

**Regresar a una versión anterior.** Los `.bin` viejos se quedan en el VPS.
Edita `versiones.json` en el servidor para que `version`, `archivo`,
`tamano` y `sha256` de la placa vuelvan a los de la versión anterior (los
valores están en la copia local `build/publicar/versiones.json` de aquella
publicación, o se recalculan con `sha256sum` y `wc -c` sobre el `.bin`).
Ojo: la báscula solo ofrece versiones **más nuevas** que la instalada; una
báscula que ya está en 0.6 no va a "bajar" sola a 0.5. Para eso está la
subida manual por la página web.

**Forzar que no se ofrezca una versión** (salió mala): borra su entrada o
vuelve a apuntar a la anterior. Las básculas que aún no la instalaron dejan de
verla en el siguiente arranque.

---

## Seguridad: qué protege y qué no

- **Integridad.** La descarga va por HTTP sin cifrar, así que lo que impide
  que llegue algo distinto a lo publicado es el **SHA-256** anunciado en
  `versiones.json` más la verificación de imagen que hace el propio chip.
  Quien pueda alterar el tráfico puede impedir la actualización, pero no
  meter otro firmware sin cambiar también el `versiones.json`.
- **El punto sensible es el VPS.** Quien escriba en
  `/var/www/html/updates/bascula/` decide qué firmware se ofrece a todas las
  básculas. La llave de `root` del VPS es, por tanto, la llave del firmware:
  cuídala como la del POS.
- **Nunca se instala sin alguien enfrente.** Hace falta presionar OK en la
  báscula. La página web solo informa.
- **Si el firmware nuevo no arranca**, la placa vuelve sola al anterior.
- **Pendiente para después**, si se quiere cerrar del todo: servir por HTTPS
  con el certificado del dominio (habría que incluir la CA raíz en el firmware
  y cambiar `OTA_SERVIDOR_URL`), o firmar el `versiones.json` con una llave
  privada que viva fuera del VPS.

---

## Si algo falla

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| `publicar_firmware.sh`: "No pude entrar al VPS por SSH" | La llave no es la del POS, o no está en `~/.ssh/config` | `ssh -v root@143.198.173.21`; publica el POS desde esa máquina para confirmar que la llave sirve |
| `publicar_firmware.sh`: "FALLÓ C5-…" | Core esp32 < 3.3.0 (el C5 no existe antes) | `arduino-cli core upgrade esp32`; mientras, `--placas 1,2` publica solo las C6 |
| Báscula: **"El servidor no responde"** | Sin salida a internet en esa red, o el VPS caído | `curl` desde un teléfono en la misma red; revisa `docker ps` y el servidor web en el VPS |
| Báscula: **"Servidor respondió 404"** | Falta `versiones.json` o la carpeta tiene otro nombre | Paso 4 |
| Báscula: **"Servidor respondió 301/302"** | El servidor redirige a HTTPS | Excluir `/updates/` de la redirección en nginx/Apache (el POS también lo necesita por HTTP) |
| Báscula: **"Lista de versiones dañada"** | JSON inválido, o el servidor lo manda comprimido | `curl -s …/versiones.json \| python -m json.tool`; quita gzip para `.json` en esa carpeta |
| Báscula: **"Lista de versiones incompleta"** | Falta `tamano`, `sha256` (64 hex) o `archivo` | Vuelve a correr el script en vez de editar a mano |
| Báscula: **"Sin firmware para esta placa"** | `versiones.json` no trae la entrada de ese `ID_PLACA` | Publica las 4 placas (sin `--placas`) |
| Báscula: **"No coincide la firma SHA-256"** | El `.bin` del servidor no es el del JSON (subida a medias, caché) | Repite el paso 3; `curl …bin \| sha256sum` debe dar el del JSON |
| Báscula: **"Tamaño distinto al anunciado"** | Igual que arriba, o `Content-Encoding` sobre el `.bin` | Revisa cabeceras (paso 4) |
| Báscula: **"Se cortó la descarga"** | WiFi débil o la pérdida de paquetes hacia DigitalOcean que ya se vio con el POS | Reintenta con OK; acerca la báscula al router |
| Báscula: **"Placa sin lugar para OTA"** | Se cargó con *Huge APP* | Cárgala una vez por USB con *Minimal SPIFFS (1.9MB APP with OTA)* |
| Instala, reinicia y vuelve a la versión anterior | El firmware nuevo no sobrevivió 10 s (ver monitor serie) | Es el seguro funcionando: revisa la versión nueva antes de republicar |

---

## Resultado

- [x] Carpeta `/var/www/html/updates/bascula/` creada — 2026-10-08, desde la PC de Jorge (Claude Code)
- [x] v0.5 publicada para las 4 placas, verificación HTTP ok (SHA-256 de los 4 `.bin` coinciden; `Content-Type: application/octet-stream`, sin gzip ni redirección)
- [ ] Báscula C6 Mini en v0.5 dice "Ya tienes la versión más reciente"
- [ ] (opcional) v0.6 de prueba ofrecida e instalada desde la pantalla

Notas de quien lo ejecutó: la primera corrida del script subió todo bien pero
su verificación falló por un error del propio script (ya corregido: la función
`sha` no leía de la tubería). Se verificó a mano con `curl | sha256sum`.

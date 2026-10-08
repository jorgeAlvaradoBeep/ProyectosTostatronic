# Báscula Multiusos — ESP32-C6 / ESP32-C5

Proyecto demostrativo de **Tostatronic** — Ing. Jorge Alvarado.

Báscula con pantalla redonda GC9A01, contador de piezas por peso, calorías por
alimento y página web con el peso en vivo. Aprovecha el WiFi 6 del ESP32-C6 y
el WiFi 6 de doble banda (2.4 y 5 GHz) del ESP32-C5.

> **Estado: fase 4 de 6 — página web y contador de piezas.** Ya pesa en gramos,
> kilos u onzas (peso estable, tara, cero, calibración guardada), **cuenta
> piezas por peso** y se conecta al WiFi **sin claves en el código**. Ya
> conectada sirve una página con el **peso en vivo**, el contador de piezas y
> los ajustes, y el firmware se actualiza desde el navegador, sin cable. Desde
> la **v0.5**, además, al arrancar consulta el servidor de Tostatronic y, si
> hay una versión más nueva para su placa, **ofrece instalarla** desde la
> pantalla. El modo calorías llega en la fase 5; el README completo, en la
> fase 6.

---

## Cómo cargarlo

1. Librerías: **GFX Library for Arduino** (moononournation) ≥ 1.6.5 y
   **ArduinoJson** ≥ 7 (lee la lista de versiones del servidor).
   No uses TFT_eSPI: no compila para el ESP32-C6 con el core 3.x.
2. Core **esp32 de Espressif ≥ 3.3.0** (antes no existe el ESP32-C5).
3. En `Bascula_Multiusos/config.h` elige tu placa (`#define PLACA ...`).
4. En el Arduino IDE:

| Placa en `config.h` | Placa en el IDE | USB CDC On Boot |
|---|---|---|
| `PLACA_C6_DEVKIT` | ESP32C6 Dev Module | — |
| `PLACA_C6_MINI` | ESP32C6 Dev Module | **Enabled** |
| `PLACA_C5_DEVKIT` | ESP32C5 Dev Module | — |
| `PLACA_C5_MINI` | ESP32C5 Dev Module | **Enabled** |

Partition Scheme en todas: **Minimal SPIFFS (1.9MB APP with OTA)**. Es el que
deja lugar para actualizar el firmware por WiFi. Si ya la tenías cargada con
*Huge APP*, cárgala una vez por USB con el esquema nuevo: la calibración no se
pierde (la NVS queda en el mismo lugar).

---

## Conexiones

| Señal | C6 DevKitC-1 | C6 Super Mini | C5 DevKitC-1 | C5 Mini (NiceMCU) |
|---|---:|---:|---:|---:|
| GC9A01 SCK (SCL) | 18 | 18 | 6 | 6 |
| GC9A01 MOSI (SDA) | 19 | 19 | 7 | 7 |
| GC9A01 CS | 20 | 20 | 10 | 10 |
| GC9A01 DC | 21 | 14 | 8 | 8 |
| GC9A01 RST | 22 | → 3V3 | 9 | 9 |
| GC9A01 BL | 23 | → 3V3 | 25 | 15 |
| HX711 DOUT (DT) | 6 | 6 | 4 | 4 |
| HX711 SCK | 7 | 7 | 5 | 5 |
| Tecla 1 · MENÚ | 0 | 2 | 23 | 0 |
| Tecla 2 · ▲ | 1 | 1 | 24 | 1 |
| Tecla 3 · ▼ | 2 | 0 | 2 | 2 |
| Tecla 4 · OK | 3 | 3 | 3 | 3 |

- Pantalla y HX711 se alimentan a **3.3 V**. Común del teclado a **GND**.
- HX711 a **10 muestras por segundo**, como viene de fábrica (pin RATE en
  bajo). Si le pones RATE en alto (80 SPS), cambia `HX_MUESTRAS_POR_SEGUNDO`
  en `config.h`.
- **C6 Super Mini:** solo tiene 10 pines utilizables en el borde, así que BL y
  RST de la pantalla van fijos a 3V3 (sin control de brillo).

> **Pines que no se tocan.** En el C6: 4, 5, 8, 9, 15 (arranque), 12 y 13 (USB),
> 16 y 17 (monitor serie), 24–30 (Flash). En el C5 **son otros**: 26, 27, 28
> (arranque), 13 y 14 (USB), 11 y 12 (monitor serie), 16–22 (Flash). El detalle
> y el porqué de cada asignación están en `config.h`.

---

## Cómo se usa

**Primer arranque.** Sin calibración guardada, entra solo al asistente:

1. **Retira todo el peso** y presiona **OK**. El anillo se llena mientras la
   lectura se asienta y se pone verde al estar estable; si presionas OK antes,
   espera a que se asiente.
2. **Coloca un peso conocido** (lo que sepas cuánto pesa: una pesa, una lata,
   un vaso con agua pesado en otra báscula) y presiona **OK**.
3. **Escribe cuánto pesa** con ▲ / ▼ (sostenidas van más rápido) y **OK**.

El factor y el cero se guardan en la NVS: sobreviven a apagar la placa. Para
recalibrar: **MENÚ → Calibrar**. Usa un peso de al menos el 5 % de la
capacidad de la celda (50 g con la de 1 kg); entre más cerca de la capacidad,
mejor.

**Pesando:**

| Tecla | Qué hace |
|---|---|
| **OK** corta | **Tara**: lo que hay en el plato pasa a ser la tara (arriba dice *NETO*). Con el plato vacío, quita la tara |
| **OK** larga | **Cero**: el plato actual pasa a ser el cero; quita la tara |
| **▲ / ▼** | Unidad: g → kg → oz |
| **MENÚ** corta | Menú: Pesar · Contar piezas · Conexión · Calibrar · Ajustes · Diagnóstico · Afinar ruido |
| **MENÚ** larga | Desde cualquier pantalla, regresa a pesar |

Tara y cero solo se toman con el peso **estable** (• ESTABLE en verde). Al
encender, si el plato está vacío toma el cero solo; con el plato vacío,
las derivas menores a media división se corrigen solas. Ya estable, el
último dígito se **retiene**: solo cambia cuando la lectura se aleja más de
0.6 divisiones, así que en reposo no baila entre −0.1, 0.0 y 0.1.

**Afinar ruido (MENÚ → Afinar ruido).** Cada celda, mesa y fuente hacen un
ruido distinto; este asistente lo mide y ajusta los filtros a lo que hay:

1. **Plato vacío** y OK: escucha 6 segundos (no toques nada).
2. **Coloca cualquier objeto** de 2 g o más: lo detecta sola, mide cuánto
   tarda en asentarse y vuelve a escuchar con carga.
3. **Retira el objeto**: mide otra vez el asentamiento.

Al terminar dice el ruido (±0.08 g), el tamaño del filtro que eligió (de 5 a
15 muestras) y en cuánto se asienta. Con eso fija la ventana del filtro, la
banda de estabilidad (1 división, o 3 sigmas si la celda no da para tanto) y
el promedio largo. Se guarda en la NVS y se aplica al instante; hazlo una vez
en el lugar donde va a trabajar la báscula, y de nuevo si la mueves de mesa o
cambias de celda. La pestaña **Tiempo real** de la página web muestra el
efecto: la línea gris es cada muestra del HX711, la azul el filtro y la
blanca lo que ve la pantalla.

**Ajustes:** unidad, capacidad de la celda (1, 5, 10 o 20 kg: define la
división y el aviso de sobrecarga) y brillo, donde BL va a un GPIO. Si cambias
de celda, recalibra.

**Diagnóstico:** cuentas crudas, muestras por segundo, ruido y teclas.
**▲ enciende la prueba de WiFi**: la báscula transmite paquetes sin parar por
el WiFi que tenga en ese momento (al router si está conectada; por su propia
red si está en el portal). Con el radio así de ocupado, **tramas lentas** y
**atípicas** deben quedarse en 0 y el ruido no debe subir. La prueba sigue
activa al salir del Diagnóstico, para pesar y calibrar con el WiFi
transmitiendo.

Por el monitor serie (115200) sale cada segundo el crudo, el filtrado, el peso
neto, la estabilidad, el ruido, los contadores de tramas lentas y atípicas, y
el estado de la red.

---

## Contar piezas

**MENÚ → Contar piezas.** Una muestra nueva son tres pasos:

1. **Pon el contenedor vacío** y presiona **OK**: lo tara.
2. **Coloca unas cuantas piezas** y presiona **OK**: toma su peso.
3. **Indica cuántas son** con ▲ / ▼ y **OK**.

Con eso calcula el peso de una pieza y la pantalla cambia a **PIEZAS**: el
número grande es el conteo y abajo va el peso. **OK** tara (para cambiar de
contenedor) y **▲ / ▼** abre las opciones: *Guardar pieza*, *Otra muestra*,
*Elegir pieza* y *Terminar*.

Las piezas guardadas (hasta 12) quedan en la memoria de la placa: la próxima
vez se elige una de la lista y se cuenta de inmediato, sin tomar muestra. Desde
la báscula se guardan como "Pieza 1", "Pieza 2"…; el nombre ("Tornillo M3×10")
se pone desde la página web, porque con cuatro teclas no se puede escribir.

**Cuándo no confiar en el conteo.** La báscula lo avisa sola:

| Aviso | Qué significa | Qué hacer |
|---|---|---|
| *Mejor con N piezas o más* | La muestra pesó menos de 50 divisiones (5 g con la celda de 1 kg): el peso por pieza salió con error y el conteo se desvía al crecer | Repite la muestra con al menos N piezas |
| *pieza ligera: no confiable* | Una pieza pesa menos que la división de la báscula (0.1 g con la de 1 kg): no distingue una pieza de más o de menos | Usa una celda de menor capacidad, o cuenta por paquetes |
| *entre 12 y 13 piezas* | El conteo cayó cerca de la mitad entre dos enteros | Cuenta esas a mano, o toma una muestra más grande |

Entre más grande la muestra, más lejos llega el conteo sin error: con una
muestra de 50 divisiones el peso por pieza trae hasta 1 % de error, y el conteo
sale exacto hasta unas 50 piezas. Para contar cientos, usa muestras más grandes.

---

## Página web

Ya conectada al WiFi, abre `http://tostabascula.local` (o la IP que muestra la
pantalla de Conexión) desde el teléfono o la computadora. No pide internet ni
instalar nada.

| Pestaña | Qué hay |
|---|---|
| **Báscula** | Peso en vivo, estable / midiendo, **Tara**, **Cero** y la unidad |
| **Tiempo real** | Una réplica de la pantalla redonda (número, estado y anillo de estabilidad) y una gráfica de los últimos 30 s con la muestra cruda, el filtrado y lo que se muestra; abajo, el ruido pico a pico, las muestras por segundo y el filtro en uso |
| **Piezas** | El contador completo: tomar la muestra ("El peso actual es de X g. ¿Cuántas unidades son?"), el conteo en vivo con sus avisos, y las piezas guardadas (usar, poner nombre, borrar) |
| **Ajustes** | Datos de la conexión, versión instalada y si el **servidor** tiene una más nueva, **recalibrar**, subir un firmware y olvidar la red |

Lo que se hace en la página se ve al instante en la pantalla de la báscula, y
al revés: son la misma báscula. Hasta 4 navegadores a la vez.

**Sin WiFi en casa** también se puede: conéctate a la red `Tostatronic-Bascula`
y entra a `http://192.168.4.1/bascula`.

**Recalibrar desde la página** pide permiso desde las teclas, igual que el
firmware: **MENÚ → Conexión → OK → Calibrar web** abre 5 minutos. Tara, cero,
unidad y conteo no piden permiso: cualquiera en tu red puede usarlos.

---

## Conexión WiFi

La báscula **pesa completa sin WiFi**. El WiFi es un extra, y nada de la red
la detiene ni la reinicia: conectarse, fallar o cambiar de red pasa mientras
sigues pesando, sin perder la tara ni el cero.

**Conectarla (una sola vez):**

1. Sin red guardada, la báscula abre la red abierta **`Tostatronic-Bascula`**
   (la pantalla lo indica en **MENÚ → Conexión**).
2. Conéctate a esa red con el teléfono. El portal se abre solo; si no, entra a
   `http://192.168.4.1`.
3. Elige tu red, escribe la clave y toca **Conectar**. Las redes de 5 GHz
   (solo las ve el ESP32-C5) salen marcadas.
4. La IP aparece en el teléfono **y en la pantalla de la báscula**. La red del
   portal se apaga sola unos segundos después.

La clave se guarda en la NVS de la placa; en el código no hay ninguna.

**Pantalla de Conexión** (MENÚ → Conexión):

| Se ve | Qué es |
|---|---|
| Nombre de la red e **IP** | Para abrirla en el navegador |
| `tostabascula.local` | La misma página, sin recordar la IP (mDNS) |
| **WiFi 6 · 2.4 GHz** | Estándar negociado con el router y banda. En el C5 puede decir **5 GHz** |
| Señal en dBm, canal y anillo | El anillo se llena con la intensidad: verde buena, ámbar débil |

**OK** abre las opciones: **Apagar / Encender WiFi** (se recuerda al apagar la
báscula), **Olvidar red** (pide un segundo OK; vuelve a abrir el portal),
**Buscar versión** (o **Instalar vX** si el arranque ya encontró una nueva),
**Subir firmware** y **Calibrar web**. La red también se puede olvidar desde
la página web.

Si el router no está (se fue la luz, cambió la clave), la báscula abre el
portal y reintenta la red guardada cada 3 minutos mientras nadie lo use.

> **La precisión con el WiFi encendido.** La celda se lee en su propia tarea,
> con más prioridad que la red, y cada trama del HX711 va en una sección
> crítica: el radio no la puede interrumpir. La potencia de transmisión se baja
> a 15 dBm (`RED_POTENCIA_DBM` en `config.h`) para que las ráfagas del radio
> jalen menos corriente de los 3.3 V que comparte el HX711. Compruébalo en tu
> placa con la prueba de WiFi del Diagnóstico.

---

## Actualizar el firmware

Hay dos caminos, y los dos sin cable USB.

### Desde el servidor de Tostatronic (el normal)

Cada vez que arranca y se conecta al WiFi, la báscula consulta
`http://143.198.173.21/updates/bascula/versiones.json` y mira la entrada de
**su** placa (`c6-mini`, `c5-devkit`…). Si hay una versión más nueva que la
instalada, la pantalla lo ofrece:

| Pantalla | Qué hacer |
|---|---|
| **Nueva versión v0.6** · lo que trae · tamaño | **OK** la instala; **MENÚ** la pospone hasta el siguiente arranque (queda en **MENÚ → Conexión → Instalar v0.6**) |
| **63 %** · descargando… | Espera; no apagues la báscula. El WiFi sigue, la celda se sigue leyendo |
| **v0.6 instalada** · Reiniciando… | Arranca ya con la nueva. Si no lograra arrancar, regresa sola a la anterior |
| **No se actualizó** · motivo | Nada cambió. **OK** reintenta |

Nunca se instala sola: hace falta presionar OK. Sin internet, o con el
servidor caído, el arranque es normal y no avisa nada. Para consultar al
momento: **MENÚ → Conexión → OK → Buscar versión** ("Ya tienes la versión más
reciente" confirma que llega al servidor). En la página web, **Ajustes →
Servidor** dice lo mismo.

Lo descargado se compara con el **SHA-256** anunciado en `versiones.json` y,
además, el chip verifica la imagen antes de activarla. Como cada placa busca
su propio archivo, aquí no hay riesgo de instalar un firmware de DevKit en una
mini.

Publicar una versión: sube `PROYECTO_VERSION` en `config.h` y corre
`docs/herramientas/publicar_firmware.sh` desde una máquina con acceso al VPS.
La guía completa, incluido el montaje inicial del servidor, está en
[`docs/ACTUALIZACION_VPS.md`](docs/ACTUALIZACION_VPS.md).

### Subiendo el archivo desde el navegador

Para probar un firmware que aún no está publicado, o para regresar a uno
anterior:

1. En el Arduino IDE: **Programa → Exportar binario compilado**. El archivo
   queda en `Bascula_Multiusos/build/…/Bascula_Multiusos.ino.bin` (ese, no el
   `merged` ni el `bootloader`). Compílalo con la **misma `PLACA`** de
   `config.h` que tiene tu báscula.
2. En la báscula: **MENÚ → Conexión → OK → Subir firmware**. Abre el permiso
   por 5 minutos y muestra la dirección.
3. En el navegador: `http://tostabascula.local/actualizar` (o la IP), elige el
   `.bin` y toca **Actualizar**. La pantalla muestra el avance y la báscula se
   reinicia sola.

Tres seguros:

- **Hay que estar frente a la báscula.** Sin el permiso del paso 2, la página
  rechaza cualquier archivo.
- **El chip verifica el firmware** antes de activarlo. Uno dañado, o compilado
  para otro chip (C5 en un C6), se rechaza y la báscula sigue con el que tenía.
- **Si el firmware nuevo no arranca**, la placa regresa sola al anterior.

Lo que no se puede verificar al subirlo a mano es que el `.bin` sea de la
misma `PLACA`: un firmware de DevKit en una mini arranca, pero con los pines
cambiados. Se corrige cargando el correcto (por WiFi si la pantalla aún se ve,
o por USB). Las actualizaciones desde el servidor no tienen este problema.

La calibración y la red guardada sobreviven a la actualización, por cualquiera
de los dos caminos.

### Cómo filtra

El HX711 se lee dentro de una sección crítica (el WiFi no puede interrumpir a
media trama). Cada muestra pasa por un **filtro de mediana con descarte de
atípicos**: se toma la mediana de las últimas muestras, se descartan las que
se alejan más de 6 MAD y se promedian las demás. Un pico nunca llega a la
pantalla. Cuando el peso se asienta, un promedio más largo deja quieto el
último dígito, y la **retención** evita que cambie por menos de 0.6
divisiones. El tamaño de la ventana (5 muestras de fábrica), la banda de
estabilidad y el promedio largo los ajusta **Afinar ruido** al ruido real de
la celda (ver `ruido.h`).

## Herramientas (`docs/herramientas/`)

| | |
|---|---|
| `generar_recursos.py` | Genera `fuentes.h` (tipografía Barlow, licencia SIL OFL) y `logo.h` |
| `compilar_todas.sh` | Compila el sketch para las 4 placas con `arduino-cli` (el que trae el Arduino IDE sirve) |
| `publicar_firmware.sh` | Compila las 4 placas, arma `versiones.json` (tamaño y SHA-256) y lo sube al servidor de actualizaciones (ver `docs/ACTUALIZACION_VPS.md`) |
| `simulador/simular.sh` | Corre la interfaz real contra una pantalla simulada y guarda capturas |

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com) · Guadalajara, Jalisco

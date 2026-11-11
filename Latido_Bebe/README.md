# Latido — el ultrasonido en una pantalla redonda, con su sonido

Proyecto de **Tostatronic** — Ing. Jorge Alvarado.

Un aparatito de 57 × 96 × 41 mm que reproduce en bucle el video del ultrasonido en una
pantalla redonda, con el sonido del corazón del bebé. Funciona con una batería 18650
recargable y el video se sube **desde el celular**, sin cables ni programas: la ESP32
crea su propia red WiFi y sirve una página que recorta y convierte el video ahí mismo.

![Vistas de la carcasa](carcasa/Vista_Carcasa.png)

> **Estado (11 nov 2026):** el firmware compila (ESP32 core 3.3.10, 58 % de la partición)
> pero **todavía no se ha probado en la placa**, y la carcasa **no se ha impreso**.
> Arma primero en protoboard y mide tus piezas antes de imprimir (sección 6).

---

## 1. Materiales

| Pieza | En tostatronic.com |
|---|---|
| ESP32-C3 Super Mini (tipo C) | [BTESP32C3SM](https://www.tostatronic.com/?s=BTESP32C3SM&post_type=product) |
| Pantalla redonda GC9A01 de 1.28" | [BTTFTRLJGC9A01](https://www.tostatronic.com/product/pantalla-tft-lcd-circular-gc9a01-de-1-28-pulgadas/) |
| Amplificador I2S MAX98357A | [BTMAX98357](https://www.tostatronic.com/product/amplificador-i2s-max98357a-3w-esp32/) |
| Bocina 8 Ω 0.5 W de 40 mm | [BTBOC8O2P](https://www.tostatronic.com/product/bocinas-de-8-ohms-2/) |
| Cargador TP4056 tipo C **con protección** | [BTTP4056TC](https://www.tostatronic.com/product/tp4056-tipo-c/) |
| Holder 18650 de 1 pila para soldar | [Porta pilas 18650](https://www.tostatronic.com/product/porta-pilas-holder-para-bateria-18650-en-serie-p-soldar/) |
| Batería 18650 | — |
| Micro switch deslizable SS12D00G4 | [BTSWT3PSS12](https://www.tostatronic.com/product/micro-switch-3-posiciones-ss12d00g4/) |
| 2 resistencias de 100 kΩ (medidor de batería, opcional) | — |
| Tornillos M3 autorroscantes: 3 de 10 mm (tapa), 2 de 8 mm (puente), 2 de 6 mm (holder) | — |

---

## 2. Conexiones

Todos los pines están en [`Latido_Bebe/config.h`](Latido_Bebe/config.h).

| ESP32-C3 Super Mini | Va a | Nota |
|---|---|---|
| GPIO 4 | Pantalla **SCL** | |
| GPIO 6 | Pantalla **SDA** | |
| GPIO 5 | Pantalla **DC** | |
| GPIO 7 | Pantalla **CS** | |
| GPIO 8 | Pantalla **RST** | |
| GPIO 3 | Pantalla **BLK** | brillo; si tu módulo no lo trae, no se cablea |
| 3V3 | Pantalla **VCC** | |
| GPIO 20 | Amplificador **BCLK** | |
| GPIO 21 | Amplificador **LRC** | |
| GPIO 10 | Amplificador **DIN** | |
| GPIO 0 | punto medio del divisor 100 k + 100 k | opcional: `BAT+ —[100k]—●—[100k]— GND` |
| GPIO 1 | botón a GND | opcional (la carcasa no lo lleva) |
| GND | GND de todo | |

`SD` y `GAIN` del amplificador se quedan sin conectar. La bocina va a los dos pads de salida.

### Alimentación

```
18650 (holder) ──> B+ / B−  TP4056  OUT+ ──[ switch ]──┬──> pin 5V de la ESP32-C3
                                    OUT− ──────────────┼──> GND
                                                       └──> Vin del MAX98357A
```

> ### ⚠️ Antes de conectar el USB de la ESP32
>
> El pin **5V** de la Super Mini está unido al USB. Si conectas el USB de la ESP32 con el
> switch encendido, los 5 V del cable llegan directo a la batería, sin pasar por el cargador.
> **Apaga el switch (o saca la pila) siempre que programes la placa.** La batería se carga
> únicamente por el USB del TP4056.

`USB CDC On Boot` tiene que ir en **Enabled**: los GPIO 20 y 21 son el puerto serie de la
placa y aquí llevan el audio.

---

## 3. Cargar el firmware

1. Arduino IDE con el core **esp32 de Espressif ≥ 3.3.0**.
2. Librerías: **GFX Library for Arduino** (moononournation) ≥ 1.6.5 y **JPEGDEC** (bitbank2).
3. Abre `Latido_Bebe/Latido_Bebe.ino` y en *Herramientas*:

| Opción | Valor |
|---|---|
| Placa | ESP32C3 Dev Module |
| USB CDC On Boot | **Enabled** |
| Partition Scheme | No OTA (2MB APP/2MB SPIFFS) |

La partición real sale del `partitions.csv` que va junto al sketch: 1.4 MB de programa y
2.5 MB para el video. Con arduino-cli:

```
arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=no_ota Latido_Bebe
```

---

## 4. Uso

1. Enciende. La pantalla muestra un corazón latiendo con los datos del WiFi.
2. En el celular conéctate a la red **`Latido`**, clave **`corazon123`**
   (se cambian en `config.h`). Se abre sola la página; si no, entra a `http://192.168.4.1`.
3. Elige el video del ultrasonido, arrástralo para centrarlo dentro del círculo y escoge
   dónde empieza y cuánto dura. **Probar el tramo con sonido** te deja oírlo antes.
4. **Convertir y enviar**. El celular lo convierte (240 × 240, a 10, 12 o 15 cuadros por segundo,
   audio a 16 kHz) y lo guarda en la memoria de la ESP32. Empieza a reproducirse en bucle.
5. En la misma página están el **volumen**, el **brillo**, pausar y borrar el video.

Para ahorrar batería el WiFi se apaga solo a los 3 minutos sin nadie conectado. Para
volver a entrar: apaga y prende el switch (o deja presionado 1.5 s el botón, si lo pusiste).

**Cuánto cabe:** unos 2.5 MB, es decir alrededor de 30–40 s con buena calidad. Si pides más,
la página baja la calidad de la imagen para que quepa. Lo ideal es un tramo de 10 a 20 s
que empiece y termine parecido, para que el bucle no se note.

Con el divisor de batería cableado, a 3.2 V el aparato apaga pantalla y audio para cuidar
la celda (el TP4056 con protección la corta además a ~2.5 V).

---

## 5. Carcasa

En [`carcasa/`](carcasa/). Tres piezas en PLA, sin soportes:

| Archivo | Cómo se imprime |
|---|---|
| `Latido_Frente.stl` | con la cara del frente sobre la cama |
| `Latido_Tapa.stl` | con la cara de afuera sobre la cama (ya viene volteada) |
| `Latido_Puente_Pantalla.stl` | acostado |

![Corte lateral](carcasa/Vista_Corte.png)

- **Frente:** la pantalla arriba y la bocina abajo, detrás de una rejilla en forma de corazón.
  La pantalla se asienta contra la ventana y se aprieta por atrás con el puente (2 tornillos
  M3; ponle un pedacito de cinta de espuma en medio para no presionar los componentes).
  La bocina entra en su anillo y se fija con tres puntos de silicón: de paso sella el frente,
  que es lo que le da cuerpo al sonido.
- **Tapa:** el holder parado a la izquierda (2 tornillos M3 por sus barrenos; las terminales
  caen en dos canales por donde salen los cables) y, a la derecha, TP4056, ESP32-C3 y
  amplificador en sus cunas, con cinta doble cara. Los dos USB-C salen por el costado derecho:
  abajo el de **carga**, arriba el de la ESP32.
- **Switch:** en la pared de arriba, pegado por dentro.
- La ESP32-C3 va **sin pines soldados** en la versión final (cables directos).
- Todo el interior es la caja acústica de la bocina: quedan unos 110 cm³ de aire. No lo
  rellenes y, si quieres un sonido más redondo, pega un poco de guata detrás de la batería.

### Cambiar medidas

`python generar_carcasa.py` (requiere `numpy manifold3d trimesh matplotlib`) vuelve a generar
los STL, las vistas y `validacion_carcasa.json`, que revisa que ninguna pieza choque con otra.
Todas las medidas están al principio del script.

## 6. Mide antes de imprimir

Las medidas del holder, bocina, amplificador, TP4056 y ESP32 salen de las fichas de
tostatronic.com. Estas otras son las típicas del módulo y **hay que confirmarlas con vernier**:

| Medida | Valor usado | En el script |
|---|---|---|
| Alto total de la bocina | 6.0 mm | `BOCINA_H` |
| Tarjeta redonda de la pantalla | hasta Ø 38 mm | `LCD_PCB_D` |
| Grueso de la pantalla (vidrio + tarjeta) | 5.2 mm | `LCD_GRUESO` |
| Pestaña de los pines de la pantalla | 19 mm de ancho, sube 26 mm desde el centro | `LCD_OREJA` |
| Cuerpo y palanca del switch | 8.8 × 4.6 mm, ranura 5.4 × 2.4 mm | `SW_CUERPO`, `SW_RANURA` |

Si la imagen sale girada en la carcasa, cambia `TFT_ROTACION` en `config.h`.

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com) · Guadalajara, Jalisco

# Báscula Multiusos — ESP32-C6 / ESP32-C5

Proyecto demostrativo de **Tostatronic** — Ing. Jorge Alvarado.

Báscula con pantalla redonda GC9A01, contador de piezas por peso, calorías por
alimento y página web con el peso en vivo. Aprovecha el WiFi 6 del ESP32-C6 y
el WiFi 6 de doble banda (2.4 y 5 GHz) del ESP32-C5.

> **Estado: fase 1 de 6 — arranque del hardware.** Esta versión comprueba el
> cableado: muestra el peso *crudo* (cuentas del ADC, sin calibrar), las
> muestras por segundo, el ruido y el estado de cada tecla. El README completo
> (calibración, conteo, calorías, lista de materiales) llega en la fase 6.

---

## Cómo cargarlo

1. Librería: **GFX Library for Arduino** (moononournation) ≥ 1.6.5.
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

Partition Scheme en todas: **Huge APP (3MB No OTA/1MB SPIFFS)**.

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
| Tecla 1 · MENÚ | 0 | 0 | 23 | 0 |
| Tecla 2 · ▲ | 1 | 1 | 24 | 1 |
| Tecla 3 · ▼ | 2 | 2 | 2 | 2 |
| Tecla 4 · OK | 3 | 3 | 3 | 3 |

- Pantalla y HX711 se alimentan a **3.3 V**. Común del teclado a **GND**.
- HX711 a **80 muestras por segundo** (pin RATE en alto). Si el tuyo está a
  10 SPS, cambia `HX_MUESTRAS_POR_SEGUNDO` en `config.h`.
- **C6 Super Mini:** solo tiene 10 pines utilizables en el borde, así que BL y
  RST de la pantalla van fijos a 3V3 (sin control de brillo).

> **Pines que no se tocan.** En el C6: 4, 5, 8, 9, 15 (arranque), 12 y 13 (USB),
> 16 y 17 (monitor serie), 24–30 (Flash). En el C5 **son otros**: 26, 27, 28
> (arranque), 13 y 14 (USB), 11 y 12 (monitor serie), 16–22 (Flash). El detalle
> y el porqué de cada asignación están en `config.h`.

---

## Qué probar en esta fase

| Acción | Qué debe pasar |
|---|---|
| Encender | Logo de Tostatronic y atribución; luego la pantalla **PRUEBA** |
| Presionar la celda | El número cambia y el anillo azul crece |
| **OK** corta | Toma el cero: el número pasa a ser relativo |
| **OK** larga | Quita el cero |
| **▲ / ▼** | Brillo (donde BL va a un GPIO) |
| **MENÚ** larga | Dibuja el contorno de cada zona de texto y el círculo útil |
| Cualquier tecla | Se enciende su punto mientras está presionada |

El renglón inferior debe decir cerca de **80 SPS**. Por el monitor serie
(115200) sale cada segundo el crudo, el ruido y el conteo de *tramas lentas*
del HX711, que debe quedarse en 0.

---

## Herramientas (`docs/herramientas/`)

| | |
|---|---|
| `generar_recursos.py` | Genera `fuentes.h` (tipografía Barlow, licencia SIL OFL) y `logo.h` |
| `compilar_todas.sh` | Compila el sketch para las 4 placas con `arduino-cli` |
| `simulador/simular.sh` | Corre la interfaz real contra una pantalla simulada y guarda capturas |

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com) · Guadalajara, Jalisco

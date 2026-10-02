# Báscula Multiusos — ESP32-C6 / ESP32-C5

Proyecto demostrativo de **Tostatronic** — Ing. Jorge Alvarado.

Báscula con pantalla redonda GC9A01, contador de piezas por peso, calorías por
alimento y página web con el peso en vivo. Aprovecha el WiFi 6 del ESP32-C6 y
el WiFi 6 de doble banda (2.4 y 5 GHz) del ESP32-C5.

> **Estado: fase 2 de 6 — calibración, tara y filtrado.** Ya pesa en gramos,
> kilos u onzas, con indicador de peso estable, tara, cero y un asistente de
> calibración que guarda en la memoria de la placa. El WiFi (portal, página web)
> llega en las fases 3 y 4; el README completo, en la fase 6.

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

## Qué probar en esta fase

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
| **MENÚ** corta | Menú: Pesar · Calibrar · Ajustes · Diagnóstico |
| **MENÚ** larga | Desde cualquier pantalla, regresa a pesar |

Tara y cero solo se toman con el peso **estable** (• ESTABLE en verde). Al
encender, si el plato está vacío toma el cero solo; con el plato vacío,
las derivas menores a media división se corrigen solas.

**Ajustes:** unidad, capacidad de la celda (1, 5, 10 o 20 kg: define la
división y el aviso de sobrecarga) y brillo, donde BL va a un GPIO. Si cambias
de celda, recalibra.

**Diagnóstico** (antes pantalla PRUEBA): cuentas crudas, muestras por segundo,
ruido y teclas. **▲ enciende la prueba de WiFi**: la placa levanta la red
abierta `Tostatronic-Bascula-Prueba` y transmite paquetes sin parar. Con el
radio así de ocupado, **tramas lentas** y **atípicas** deben quedarse en 0 y
el ruido no debe subir. La prueba sigue activa al salir del Diagnóstico, para
pesar y calibrar con el WiFi transmitiendo.

Por el monitor serie (115200) sale cada segundo el crudo, el filtrado, el peso
neto, la estabilidad, el ruido y los contadores de tramas lentas y atípicas.

### Cómo filtra

El HX711 se lee dentro de una sección crítica (el WiFi no puede interrumpir a
media trama). Cada muestra pasa por un **filtro de mediana con descarte de
atípicos**: se toma la mediana de las últimas muestras, se descartan las que
se alejan más de 6 MAD y se promedian las demás. Un pico nunca llega a la
pantalla. Cuando el peso se asienta, un promedio más largo deja quieto el
último dígito.

## Herramientas (`docs/herramientas/`)

| | |
|---|---|
| `generar_recursos.py` | Genera `fuentes.h` (tipografía Barlow, licencia SIL OFL) y `logo.h` |
| `compilar_todas.sh` | Compila el sketch para las 4 placas con `arduino-cli` |
| `simulador/simular.sh` | Corre la interfaz real contra una pantalla simulada y guarda capturas |

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com) · Guadalajara, Jalisco

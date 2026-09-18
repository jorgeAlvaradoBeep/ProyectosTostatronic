# Recorrido de LEDs — ESP32 + 74HC595

Proyecto demostrativo de Tostatronic. Controla 8 LEDs (ampliable a 16, 24, 32…)
usando **solo 3 GPIO** del ESP32, mediante un registro de desplazamiento 74HC595.

Hay **un sketch por placa**, porque los pines no son los mismos:

| Placa | Sketch |
|---|---|
| ESP32 clásico (WROOM-32, DevKit V1…) | `ESP32_Recorrido_LEDs_595/` |
| ESP32-S3 (DevKitC-1 y similares) | `ESP32S3_Recorrido_LEDs_595/` |

La lógica es idéntica en los dos; solo cambian los pines. Cada sketch verifica
la placa al compilar: si eliges la equivocada en el Arduino IDE, marca un
`#error` que te dice cuál abrir, en vez de compilar y no hacer nada.

---

## El punto técnico que importa

El ESP32 entrega **3.3 V** en sus GPIO. Un **74HC595 alimentado a 5 V** exige
`VIH = 0.7 × VCC = 3.5 V` para reconocer un nivel alto. **3.3 V queda por debajo**,
así que el circuito opera fuera de especificación: puede funcionar en la mesa y
fallar al calentar, o meter bits basura en el registro.

**Solución usada en este proyecto:** alimentar el 74HC595 con **3.3 V** desde el
pin `3V3` del ESP32. El HC595 trabaja de 2 V a 6 V, y a 3.3 V su `VIH` baja a
2.31 V — sin conflicto de niveles.

Alternativas válidas si necesitas 5 V en el registro:

| Opción | VIH mínimo @ 5 V | ¿Sirve con ESP32? |
|---|---|---|
| 74HC595 @ 5 V | 3.5 V | ❌ Fuera de spec |
| 74HC595 @ 3.3 V | 2.31 V | ✅ **Recomendado** |
| 74HCT595 @ 5 V | 2.0 V | ✅ Sí (entradas TTL) |
| 74LS595 @ 5 V | 2.0 V | ✅ Sí (entradas TTL) |
| 74HC595 @ 5 V + level shifter | — | ✅ Sí |

---

## LEDs en modo sumidero

```
3.3V ──▶ ánodo LED ──▶ cátodo ──▶ R 220 Ω ──▶ salida Qx del 595
```

El LED enciende cuando la salida va en **BAJO**. La inversión se aplica dentro de
la función `escribir()`, así que los patrones se escriben en lógica positiva.

**Cálculo de la resistencia:** `(3.3 V − 2.0 V Vf − 0.3 V VOL) / 5 mA ≈ 200 Ω`
→ se usa la comercial de **220 Ω**, que da ~4.5 mA por LED.
Ocho LEDs = 36 mA, muy por debajo del límite del chip.

Para modo fuente (ánodo a la salida, cátodo a GND), cambia
`MODO_SUMIDERO` a `false` en el sketch.

---

## Conexiones

| ESP32 clásico | ESP32-S3 | 74HC595 (DIP-16) | Señal |
|---|---|---|---|
| GPIO 23 | **GPIO 11** | pin 14 | SER — datos serie |
| GPIO 18 | **GPIO 12** | pin 11 | SRCLK — reloj del shift register |
| GPIO 5 | **GPIO 10** | pin 12 | RCLK — latch |
| GND | GND | pin 13 | OE — salidas habilitadas |
| 3V3 | 3V3 | pin 10 | SRCLR — nunca borrar |
| 3V3 | 3V3 | pin 16 | VCC |
| GND | GND | pin 8 | GND |
| — | — | pins 15, 1–7 | QA…QH → LEDs con R de 220 Ω |
| — | — | pin 9 | QH′ → SER del siguiente 595 (cascada) |

> **El ESP32-S3 no tiene GPIO 23.** El S3 tiene un hueco en la numeración:
> existen GPIO 0–21 y 26–48, pero **22, 23, 24 y 25 no existen**. Un
> `pinMode(23, OUTPUT)` en S3 no hace nada y el registro nunca recibe datos.
> Por eso el S3 tiene su propio sketch, `ESP32S3_Recorrido_LEDs_595`, con los
> pines 11, 12 y 10.
>
> En el S3, además, evita GPIO 0, 3, 45 y 46 (strapping), 19 y 20 (USB nativo),
> 26–32 (flash/PSRAM interno) y, en módulos con memoria octal (N8R8, N16R8),
> también 33–37.

> **Capacitor de 0.1 µF cerámico entre pin 16 y pin 8, pegado al chip.**
> No es opcional. Sin él aparecen glitches en el latch y vas a ver LEDs
> parpadeando donde no deben.

En ambas placas los GPIO elegidos corresponden al **bus SPI por hardware**
(VSPI en el ESP32 clásico, SPI2/FSPI en el S3), así que migrar a SPI nativo
después implica cambiar únicamente la función `escribir()`.

---

## Cascada: de 8 a 16 LEDs

1. Conecta el **pin 9 (QH′)** del primer 595 al **pin 14 (SER)** del segundo.
2. Comparte `SRCLK` y `RCLK` entre ambos chips.
3. En el sketch, cambia `NUM_REGISTROS` de `1` a `2`.

Los patrones se recalculan solos. Siguen siendo los mismos 3 GPIO del ESP32.
El máximo es `NUM_REGISTROS = 4` (32 LEDs), porque el patrón viaja en un
`uint32_t`; el sketch lo valida con un `static_assert`.

---

## Estructura del código

El sketch **no usa `delay()`**. Cada patrón es una función pura que recibe el
número de paso y devuelve qué LEDs deben estar encendidos:

```cpp
uint32_t patronRecorrido(uint16_t paso) {
  return 1UL << paso;
}
```

El avance se maneja con `millis()`, así que el `loop()` queda libre para
agregarle WiFi, botones o sensores en proyectos posteriores.

Patrones incluidos: recorrido, ping-pong (Knight Rider), llenado, vaciado,
encuentro y contador binario.

---

## Lista de materiales

| Cant. | Componente |
|---|---|
| 1 | ESP32 DevKit **o** ESP32-S3 DevKitC-1 |
| 1 | 74HC595 (DIP-16) |
| 8 | LED 5 mm |
| 8 | Resistencia 220 Ω ¼ W |
| 1 | Capacitor cerámico 0.1 µF |
| 1 | Protoboard y jumpers |

---

## Uso

Abre en el Arduino IDE el sketch que corresponde a tu placa:

- **ESP32 clásico:** `ESP32_Recorrido_LEDs_595/ESP32_Recorrido_LEDs_595.ino`
  — placa *ESP32 Dev Module*.
- **ESP32-S3:** `ESP32S3_Recorrido_LEDs_595/ESP32S3_Recorrido_LEDs_595.ino`
  — placa *ESP32S3 Dev Module*.

Carga y abre el monitor serie a **115200 baudios**: indica qué patrón se está
reproduciendo.

> **Monitor serie en el S3:** si tu placa tiene dos puertos USB, usa el marcado
> `UART`/`COM`. Si solo tiene el USB nativo, activa
> *Herramientas → USB CDC On Boot → Enabled* antes de cargar.

---

**Tostatronic** — [tostatronic.com](https://www.tostatronic.com)

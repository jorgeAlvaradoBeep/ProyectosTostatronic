# Display de 7 segmentos — cátodo común vs ánodo común

Proyecto demostrativo de **Tostatronic** — Ing. Jorge Alvarado.

La duda número uno con estos displays: ¿es de cátodo común o de ánodo común?
Aquí está la diferencia, cómo saber cuál tienes y con qué se usa cada uno.

---

## Qué es

Un display de 7 segmentos son **siete LEDs en forma de 8**, nombrados de la
**a** a la **g** (a–f en sentido horario desde arriba, g al centro). Muchos
traen un octavo LED: el **punto decimal (DP)**; por eso a veces se les llama de
8 segmentos.

Para ahorrar pines, todos los LEDs comparten una de sus patas: **el común**. En
el formato más común de 1 dígito (10 pines), el común está en los **dos pines
centrales (3 y 8)**, unidos por dentro. Revisa la hoja de datos de tu modelo.

---

## Cátodo común vs ánodo común

| | Cátodo común | Ánodo común |
|---|---|---|
| Los LEDs comparten | el negativo (cátodo) | el positivo (ánodo) |
| El común va a | **GND** | **VCC** |
| Un segmento enciende con | **ALTO (1)** en su pin | **BAJO (0)** en su pin |
| Se lleva bien con | microcontrolador directo, **MAX7219**, **CD4511** | drivers que absorben corriente: **74LS47**, **ULN2803** |

La lógica se invierte: el mismo código que enciende un display de cátodo común
apaga el de ánodo común.

---

## Cómo saber cuál tienes

Con el multímetro en **modo diodo**:

1. Punta **roja** (positiva) en un pin común, punta **negra** en cualquier segmento.
2. **Si el segmento prende → ánodo común.**
3. Si no prende, invierte las puntas. **Si ahora prende → cátodo común.**

> En muchos displays del modelo 5161, **AS** suele ser cátodo común y **BS**
> ánodo común. No es un estándar: confírmalo siempre con el multímetro.

---

## Resistencias

**Una resistencia por segmento** si lo conectas directo. Con una sola
resistencia en el común, el brillo cambia según cuántos segmentos enciendas: un
8 se ve más tenue que un 1. (Excepción: el MAX7219 regula la corriente de todos
los segmentos con una sola resistencia, RSET.)

| Alimentación | Resistencia por segmento (LED rojo, ~10 mA) |
|---|---|
| 5 V | 220 – 330 Ω |
| 3.3 V | 100 – 150 Ω |

> **Ánodo común + ESP32:** si pones el común a 5 V y manejas los segmentos desde
> un pin de 3.3 V, un ALTO no apaga del todo el segmento (quedan ~1.7 V) y se ve
> encendido a medias. Pon el común a 3.3 V o usa transistores.

---

## Aplicaciones

Relojes, básculas, contadores, marcadores, termómetros, temporizadores de
cocina, indicadores de piso en elevadores, multímetros.

---

## Estructura

```
Display_7_Segmentos/
├── README.md
└── video/    ← guion, SRT de la narración, fotogramas y prompts del video con Tostabot
```

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com) · Guadalajara, Jalisco

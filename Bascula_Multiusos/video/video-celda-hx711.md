# Video explicativo — Celda de carga + HX711

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado
Formato: **9:16** · Duración objetivo: **~55 s** · 11 clips de 5 s encadenados

---

## Cómo funciona el encadenado

```
F0 ──clip 1──▶ F1 ──clip 2──▶ F2 ──clip 3──▶ F3 … ──clip 11──▶ F11
```

- **Clip 1:** inicio = F0, final = F1.
- **Clip 2 en adelante:** inicio = **último cuadro del clip anterior** (lo extraes
  del video), final = el siguiente fotograma de la lista.

### Regla para que no se rompa la continuidad

Genera cada fotograma en Nano Banana Pro **usando el fotograma anterior como
referencia** (F1 a partir de F0, F2 a partir de F1…). Como cada clip termina
casi exactamente en su fotograma final, el último cuadro del video y la
referencia de la siguiente imagen coinciden en estilo, luz y encuadre.

**Referencias de producto:** sube fotos reales de tu celda de carga, tu HX711 y
tu ESP32-C6, igual que hiciste con la WROVER. Úsalas junto con el fotograma
anterior en los cuadros donde aparece cada pieza (F0, F7, F10). Sin ellas, el
modelo inventa la placa.

### Lo que NO debe aparecer

Ningún fotograma lleva texto, números ni etiquetas. Los modelos los deforman.
Si quieres rotular "×128" o "24 bits", ponlo tú en la edición, encima del video.

---

## Narración — ElevenLabs, voz Ballesteros, una sola toma

~95 palabras. Con v3 esto debe salir en unos 50-55 s.

```
[curious] ¿Cómo sabe una báscula cuánto pesa algo?

Todo empieza en esta barra de aluminio: la celda de carga.

Cuando le pones peso, la barra se dobla... [short pause] tan poco que ni lo ves.

Pegadas en ella hay galgas extensométricas. Unas se estiran, otras se comprimen, y su resistencia cambia.

Las cuatro forman un puente de Wheatstone, que convierte ese cambio en voltaje. [short pause] Pero es diminuto: apenas unos milivoltios.

[excited] Ahí entra el HX711. Amplifica la señal ciento veintiocho veces y la convierte en un número de veinticuatro bits.

Por solo dos cables, el ESP32 recibe ese número, lo calibra... [cheerfully] y te muestra el peso en gramos.
```

**La narración manda.** Monta primero el audio y después ajusta cada clip a su
frase: recórtalo, alárgalo con cámara lenta o congela el último cuadro. Los
clips son de 5 s, las frases no.

### Qué frase va con qué clip

| Clip | Frase |
|---|---|
| 1 | ¿Cómo sabe una báscula cuánto pesa algo? |
| 2 | Todo empieza en esta barra de aluminio: la celda de carga. |
| 3 | Cuando le pones peso, la barra se dobla… tan poco que ni lo ves. |
| 4 | Pegadas en ella hay galgas extensométricas. |
| 5 | Unas se estiran, otras se comprimen, y su resistencia cambia. |
| 6 | Las cuatro forman un puente de Wheatstone, que convierte ese cambio en voltaje. |
| 7 | Pero es diminuto: apenas unos milivoltios. |
| 8 | Ahí entra el HX711. Amplifica la señal ciento veintiocho veces… |
| 9 | …y la convierte en un número de veinticuatro bits. |
| 10 | Por solo dos cables, el ESP32 recibe ese número, lo calibra… |
| 11 | …y te muestra el peso en gramos. |

---

## Fotogramas — Nano Banana Pro · 9:16

### F0 · La celda de carga (inicio del clip 1)

Referencias: foto de tu celda de carga.

```
Photorealistic cinematic product shot, vertical 9:16. A single aluminium bar-type
load cell rests diagonally on a dark charcoal workbench, seen in three-quarter
view. It is a solid rectangular silver bar with two joined holes through its
centre, a small white protective coating over the middle section where the strain
gauges sit, two threaded mounting holes at each end, and four thin wires — red,
black, green and white — coming out of one end. Keep the load cell exactly as in
the reference image. Cool blue rim light tracing its edges, soft warm key light,
shallow depth of field, dark background with subtle blue haze. No text, no
letters, no numbers, no labels anywhere in the frame.
```

### F1 · Montada como báscula

Referencia: F0.

```
Same style, lighting and colour grade as the reference image, vertical 9:16. The
same aluminium bar load cell is now mounted horizontally as a scale: its left end
is bolted onto a dark metal base block, and its right end holds a round brushed-
steel weighing platform on top. A polished steel calibration weight hovers a few
centimetres above the platform, about to drop. Cool blue rim light, warm key
light, dark charcoal background. No text, no letters, no numbers anywhere.
```

### F2 · El peso cae y la barra se dobla

Referencia: F1.

```
Same scene, style and lighting as the reference image, vertical 9:16. The steel
weight now rests on the platform. The free end of the load cell bar is bent
slightly downward — exaggerated just enough to be visible — and a soft blue glow
emanates from the two holes in the middle of the bar, where the metal is flexing.
Faint motion ripples in the air around the platform. No text, no letters, no
numbers anywhere.
```

### F3 · Acercamiento a las galgas

Referencia: F2.

```
Same visual style and colour grade as the reference image, vertical 9:16. Extreme
macro close-up of the middle section of the aluminium load cell, over the holes.
The white protective coating has become semi-transparent, revealing a thin metal
foil strain gauge bonded to the surface: a fine zig-zag grid of copper-coloured
traces with two solder tabs, glowing faintly. Very shallow depth of field, dark
background, cool blue rim light. No text, no letters, no numbers anywhere.
```

### F4 · Unas se estiran, otras se comprimen

Referencia: F3.

```
Same style as the reference image, vertical 9:16. A holographic x-ray view of the
load cell bar from the side, with four strain gauge grids visible inside it. The
two gauges on the stretched areas glow warm orange with their zig-zag lines
slightly elongated; the two gauges on the compressed areas glow cool blue with
their lines slightly squeezed. Subtle stress lines of light flow through the
bending metal between them. Dark background, cinematic, high-tech. No text, no
letters, no numbers, no labels anywhere.
```

### F5 · El puente de Wheatstone

Referencia: F4.

```
Same style as the reference image, vertical 9:16. The four glowing strain gauges
lift out of the metal and rearrange in mid-air into a diamond-shaped circuit: four
resistor elements forming the four sides of a diamond, joined at the corners by
thin lines of light — two glowing orange, two glowing blue. A gentle current of
blue light particles flows around the diamond. The load cell bar remains softly
out of focus below. Dark background, holographic, cinematic. No text, no letters,
no numbers, no symbols, no labels anywhere.
```

### F6 · Una señal diminuta

Referencia: F5.

```
Same style as the reference image, vertical 9:16. From the side corners of the
glowing diamond circuit, a very faint, thin, flickering thread of light — barely
visible, weak and dim — travels away along two thin wires, one green and one
white, toward the bottom of the frame. The contrast should make it obvious the
signal is tiny and fragile. Dark background, holographic, cinematic. No text, no
letters, no numbers anywhere.
```

### F7 · El HX711

Referencias: F6 + foto de tu HX711.

```
Same style and lighting as the first reference image, vertical 9:16. A small HX711
load cell amplifier breakout board sits on the dark workbench, shown at a slight
angle, kept exactly as in the product reference photo: a small PCB with the HX711
chip in the middle, a row of pin headers on each side. Four thin wires — red,
black, green and white — plug into one side of the board, and the faint thread of
light arrives along the green and white wires, reaching the chip. Cool blue rim
light, shallow depth of field. No text, no letters, no numbers, no labels.
```

### F8 · Amplificada 128 veces

Referencia: F7.

```
Same scene, style and lighting as the reference image, vertical 9:16. The HX711
chip glows brightly from within. On the incoming side the signal is still a faint
thin thread; on the outgoing side it bursts out as a thick, bright, strong beam of
blue-white light — dramatically amplified. Soft light bloom around the chip. The
board and wires stay exactly the same. No text, no letters, no numbers anywhere.
```

### F9 · Convertida a digital

Referencia: F8.

```
Same style as the reference image, vertical 9:16. The bright beam leaving the
HX711 turns into a stream of discrete, evenly spaced, glowing square pulses —
crisp rectangular blocks of light marching in a line, like a digital signal —
travelling along two thin wires, one carrying the square pulses and the other a
steady rhythmic ticking of small light points, like a clock. Dark background,
holographic, cinematic. No text, no letters, no numbers, no binary digits, no
symbols anywhere.
```

### F10 · Llega al ESP32

Referencias: F9 + foto de tu ESP32-C6.

```
Same style and lighting as the first reference image, vertical 9:16. The two wires
carrying the square light pulses arrive at an ESP32-C6 development board lying on
the dark workbench, kept exactly as in the product reference photo. The pulses
flow into the board and its main chip lights up with a soft blue glow, with tiny
light traces spreading through the circuit. Cool blue rim light, shallow depth of
field. No text, no letters, no numbers, no labels anywhere.
```

### F11 · El resultado en la pantalla

Referencia: F10.

```
Same style as the reference image, vertical 9:16. The finished scale on the dark
workbench: the load cell with the steel weight on its platform, the HX711 and the
ESP32-C6 connected by wires, and a small round glowing display facing the camera.
The round display shows only a clean luminous circular arc gauge filling around
its edge in brand blue, with a glowing dot at the end of the arc — no digits.
Cinematic, warm key light, cool blue rim light, shallow depth of field. No text,
no letters, no numbers anywhere in the frame.
```

> En F11 la pantalla **no lleva el número a propósito**: un modelo casi nunca
> dibuja dígitos limpios. En la edición pon encima el peso real ("500 g") con tu
> tipografía, o mejor aún, corta a una toma real de tu báscula mostrando el
> número.

---

## Movimiento — Kling 3.0 · 9:16 · 5 s · sin audio nativo

Todos los clips con **cámara lenta y suave**. Nada de cortes dentro del clip.

**Clip 1 · F0 → F1**
```
The camera slowly pulls back and rises as the load cell glides smoothly into
place and mounts onto the base, and the weighing platform settles on its free end.
A steel weight descends gently into view above the platform and stops, hovering.
Smooth, slow, cinematic motion. No text anywhere.
```

**Clip 2 · → F2**
```
The steel weight drops softly onto the platform. The free end of the bar dips
slightly under the load and a soft blue glow pulses from the holes in the middle
of the bar. Faint ripples spread through the air. Static camera. No text anywhere.
```

**Clip 3 · → F3**
```
The camera pushes in slowly toward the middle of the load cell until it becomes an
extreme macro. The white coating turns semi-transparent, revealing the fine
zig-zag metal foil of the strain gauge, which begins to glow. Smooth continuous
dolly-in. No text anywhere.
```

**Clip 4 · → F4**
```
The view turns into a holographic x-ray of the bar from the side. Four strain
gauges appear inside the metal; two slowly stretch and warm to orange, two slowly
compress and cool to blue, as lines of stress light flow through the bending
metal. Slow, smooth motion. No text anywhere.
```

**Clip 5 · → F5**
```
The four glowing strain gauges lift gently out of the metal and float upward,
rotating and rearranging in mid-air until they lock together into a diamond-shaped
circuit joined by lines of light, with particles beginning to flow around it.
Smooth, slow, elegant motion. No text anywhere.
```

**Clip 6 · → F6**
```
From two corners of the glowing diamond, a very faint, thin, flickering thread of
light emerges and slowly travels away along a green and a white wire toward the
bottom of the frame. The camera tilts down gently to follow it. No text anywhere.
```

**Clip 7 · → F7**
```
The camera follows the faint thread of light along the wires as it travels down
into a small HX711 amplifier board, which comes into focus as the light reaches its
chip. Smooth tracking motion. No text anywhere.
```

**Clip 8 · → F8**
```
The HX711 chip begins to glow brighter and brighter. The weak incoming thread of
light enters, and a strong, thick, bright beam bursts out of the other side,
dramatically amplified, with a soft bloom of light. Static camera. No text anywhere.
```

**Clip 9 · → F9**
```
The bright beam leaving the chip breaks up into a crisp stream of evenly spaced
square pulses of light marching along one wire, while small points of light tick
rhythmically along a second wire beside it, like a clock. Slow, steady motion.
No text anywhere.
```

**Clip 10 · → F10**
```
The camera tracks along the wires as the square pulses of light travel into an
ESP32-C6 board. As they arrive, the main chip lights up with a soft blue glow and
fine traces of light spread across the circuit. Smooth tracking motion.
No text anywhere.
```

**Clip 11 · → F11**
```
The camera pulls back slowly to reveal the complete scale: load cell with the
weight, the HX711 and the ESP32-C6 connected, and the round display facing the
camera. A luminous blue arc sweeps smoothly around the edge of the round display
and stops. Slow, satisfying reveal. No text, no digits anywhere.
```

---

## Datos técnicos que sostiene la narración

Para que puedas defender cada frase si alguien pregunta en los comentarios:

| Dato | Valor |
|---|---|
| Galgas en la celda de barra | 4, en puente de Wheatstone completo |
| Salida típica de la celda | ~1 mV por volt de excitación a carga máxima (unos pocos mV en total) |
| Ganancia del HX711 | 128 en el canal A (64 opcional; 32 en el canal B) |
| Resolución del HX711 | ADC de 24 bits |
| Interfaz | 2 hilos propios (DT y SCK) — **no es I2C** |
| Velocidad | 10 u 80 muestras por segundo |
| Cables de la celda | Rojo E+ · Negro E− · Verde A+ · Blanco A− (convención común; **verifica la hoja de datos de tu celda**, algunos fabricantes cambian verde y blanco) |

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado

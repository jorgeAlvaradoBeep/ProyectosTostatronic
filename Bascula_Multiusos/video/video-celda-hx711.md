# Video explicativo — Celda de carga + HX711

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado
Formato: **9:16** · Duración objetivo: **~55 s** · 11 clips de 5 s encadenados
Con **Tostabot** como presentador

---

## Tostabot — cara y reglas de animación

> **Esto cambió respecto a los primeros videos.** La boca ya no es una onda.

| | |
|---|---|
| **Ojos** | Dos puntos redondos brillantes en la matriz LED. **Siempre visibles**, nunca desaparecen ni se apagan. |
| **Boca** | La **sonrisa semicircular** de puntos LED que ya trae el personaje. |
| **Al hablar** | La sonrisa se **flexiona y cambia de forma** — se ensancha, se angosta, se curva más o menos — pero **sigue siendo una sonrisa**. |
| **Nunca** | Onda de audio. Línea recta. Labios reales. Ojos cerrados o apagados. |

Kling anima bien esa boquita: se ve mucho mejor que la onda y mantiene al
personaje reconocible.

**Etiqueta a Tostabot como Elemento en Kling** antes de generar. Así sale
idéntico en los once clips. En los prompts aparece como *"Tostabot, the tagged
character element"*; si algún día lo haces por API, ahí va `<<<id>>>`.

---

## Dónde aparece Tostabot, y por qué ahí

Aparece en **4 de los 11 clips**. No más: si sale en todos, estorba la
explicación técnica, que es lo que el video viene a contar.

| Clips | Tostabot | Encuadre |
|---|---|---|
| **1 – 3** | Presenta la celda y reacciona al peso | Plano abierto, él a un lado |
| **4 – 6** | Fuera de cuadro | Macro y rayos X |
| **7** | **Explica el puente de Wheatstone** | La cámara abre y él aparece junto al holograma |
| **8 – 10** | Fuera de cuadro | Macro del HX711 y del ESP32 |
| **11** | Cierra junto a la báscula terminada | Plano abierto |

### La regla que evita que se teletransporte

Tostabot **nunca aparece ni desaparece de golpe**. Entra y sale porque **la
cámara se acerca o se aleja**: cuando empujamos al macro, él queda fuera de
encuadre — no se esfuma. Cuando abrimos, vuelve a entrar.

Por eso el orden de los fotogramas está armado así: los clips donde está
presente son consecutivos, y las transiciones a macro son empujes de cámara.

---

## Cómo funciona el encadenado

```
F0 ──clip 1──▶ F1 ──clip 2──▶ F2 ──clip 3──▶ F3 … ──clip 11──▶ F11
```

- **Clip 1:** inicio = F0, final = F1.
- **Clip 2 en adelante:** inicio = **último cuadro del clip anterior**, final =
  el siguiente fotograma.

**Genera cada fotograma usando el anterior como referencia** (F1 desde F0, F2
desde F1…). Y en los que sale Tostabot, agrega su imagen como segunda
referencia.

**Referencias de producto:** sube fotos reales de tu celda de carga, tu HX711 y
tu ESP32-C6. Úsalas en F0, F8 y F10. Sin ellas el modelo inventa las piezas.

**Nada de texto** en ningún fotograma: ni letras, ni números, ni etiquetas. Si
quieres rotular "×128" o "24 bits", ponlo tú en la edición.

---

## Narración — ElevenLabs, voz Ballesteros, una sola toma

Es la voz de Tostabot, aunque no esté en pantalla todo el tiempo.

```
[curious] ¿Cómo sabe una báscula cuánto pesa algo?

Todo empieza en esta barra de aluminio: la celda de carga.

Cuando le pones peso, la barra se dobla... [short pause] tan poco que ni lo ves.

Pegadas en ella hay galgas extensométricas. Unas se estiran, otras se comprimen, y su resistencia cambia.

Las cuatro forman un puente de Wheatstone, que convierte ese cambio en voltaje. [short pause] Pero es diminuto: apenas unos milivoltios.

[excited] Ahí entra el HX711. Amplifica la señal ciento veintiocho veces y la convierte en un número de veinticuatro bits.

Por solo dos cables, el ESP32 recibe ese número, lo calibra... [cheerfully] y te muestra el peso en gramos.
```

| Clip | Frase |
|---|---|
| 1 | ¿Cómo sabe una báscula cuánto pesa algo? |
| 2 | Todo empieza en esta barra de aluminio: la celda de carga. |
| 3 | Cuando le pones peso, la barra se dobla… tan poco que ni lo ves. |
| 4 | Pegadas en ella hay galgas extensométricas. |
| 5 | Unas se estiran, otras se comprimen, y su resistencia cambia. |
| 6 | Las cuatro forman un puente de Wheatstone… |
| 7 | …que convierte ese cambio en voltaje. |
| 8 | Pero es diminuto: apenas unos milivoltios. |
| 9 | Ahí entra el HX711. Amplifica la señal ciento veintiocho veces y la convierte en un número de veinticuatro bits. |
| 10 | Por solo dos cables, el ESP32 recibe ese número, lo calibra… |
| 11 | …y te muestra el peso en gramos. |

**Monta primero el audio.** Los clips son de 5 s, las frases no. Ajusta cada uno
con recorte, cámara lenta o congelando el último cuadro.

---

## Bloque de cara — cópialo en cada prompt donde salga Tostabot

```
Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots.
While he speaks, the smile flexes and changes shape — widening, narrowing,
curving more or less — but it always remains a smile. Never a waveform, never a
straight line, never real lips. His eyes never close or switch off.
```

---

## Fotogramas — Nano Banana Pro · 9:16

### F0 · Tostabot presenta la celda *(inicio del clip 1)*

Referencias: Tostabot + foto de tu celda de carga.

```
Photorealistic cinematic scene, vertical 9:16. A friendly boxy blue metallic robot
— Tostabot, the tagged character element, kept exactly as in his reference — stands
on the left side of the frame at a dark charcoal workbench, turned slightly toward
the centre. On the bench in front of him, taking the centre of the frame, lies a
single aluminium bar-type load cell in three-quarter view: a solid rectangular
silver bar with two joined holes through its centre, a white protective coating
over the middle, threaded mounting holes at both ends, and four thin wires — red,
black, green and white — coming out of one end. Keep the load cell exactly as in
its reference photo.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots.

Cool blue rim light, warm key light, shallow depth of field, dark background with
subtle blue haze. No text, no letters, no numbers, no labels anywhere.
```

### F1 · Montada como báscula

Referencia: F0.

```
Same style, lighting, colour grade and framing as the reference image, vertical
9:16. Tostabot stays on the left of the frame in the same position, now with one
wrench hand raised in a small presenting gesture toward the centre. The load cell
is now mounted as a scale: its left end bolted onto a dark metal base block, its
right end holding a round brushed-steel weighing platform. A polished steel
calibration weight hovers a few centimetres above the platform.

Tostabot's two round glowing eyes stay visible, and his mouth is a small
semicircular smile of glowing LED dots — never a waveform, never a straight line.

No text, no letters, no numbers anywhere.
```

### F2 · Cae el peso y la barra se dobla

Referencia: F1.

```
Same scene, style and framing as the reference image, vertical 9:16. The steel
weight now rests on the platform. The free end of the load cell bar is bent
slightly downward — exaggerated just enough to be visible — and a soft blue glow
comes from the two holes in the middle of the bar. Tostabot, still on the left,
leans in slightly and tilts his head toward the bending bar with interest.

His two round glowing eyes stay visible, and his mouth is a small semicircular
smile of glowing LED dots — never a waveform.

No text, no letters, no numbers anywhere.
```

### F3 · Macro de la galga *(Tostabot queda fuera de cuadro)*

Referencia: F2.

```
Same visual style, lighting and colour grade as the reference image, vertical 9:16.
Extreme macro close-up of the middle section of the aluminium load cell, over the
holes — the camera has pushed all the way in, so nothing else is in frame. The
white protective coating has become semi-transparent, revealing a thin metal foil
strain gauge bonded to the surface: a fine zig-zag grid of copper-coloured traces
with two solder tabs, glowing faintly. Very shallow depth of field, dark
background, cool blue rim light. No text, no letters, no numbers anywhere.
```

### F4 · Unas se estiran, otras se comprimen

Referencia: F3.

```
Same style as the reference image, vertical 9:16. A holographic x-ray view of the
load cell bar from the side, with four strain gauge grids visible inside it. The
two gauges on the stretched areas glow warm orange with their zig-zag lines
slightly elongated; the two on the compressed areas glow cool blue with their
lines slightly squeezed. Lines of stress light flow through the bending metal
between them. Dark background, cinematic, high-tech. No text, no letters, no
numbers, no labels anywhere.
```

### F5 · El puente de Wheatstone

Referencia: F4.

```
Same style as the reference image, vertical 9:16. The four glowing strain gauges
lift out of the metal and rearrange in mid-air into a diamond-shaped circuit: four
resistor elements forming the four sides of a diamond, joined at the corners by
thin lines of light — two glowing orange, two glowing blue. A gentle current of
blue light particles flows around the diamond. The load cell bar stays softly out
of focus below. Dark background, holographic, cinematic. No text, no letters, no
numbers, no symbols anywhere.
```

### F6 · Tostabot explica el puente *(la cámara abre y él vuelve)*

Referencias: F5 + Tostabot.

```
Same style, lighting and colour grade as the first reference image, vertical 9:16.
The camera has pulled back to a wider shot: the glowing diamond-shaped Wheatstone
bridge circuit now floats as a large hologram in the middle of the frame above the
workbench, and Tostabot — the tagged character element, kept exactly as in his
reference — stands to the left of it, one wrench hand raised toward the hologram
as if explaining it. The blue light of the hologram reflects on his metallic blue
body.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
never a waveform, never a straight line.

Dark background, cinematic, holographic. No text, no letters, no numbers anywhere.
```

### F7 · Una señal diminuta *(la cámara vuelve a cerrar)*

Referencia: F6.

```
Same style and colour grade as the reference image, vertical 9:16. The camera has
pushed back in: only the lower corners of the glowing diamond circuit are in
frame. From them, a very faint, thin, flickering thread of light — barely visible,
weak and fragile — travels away along two thin wires, one green and one white,
toward the bottom of the frame. The contrast makes it obvious the signal is tiny.
Dark background, holographic. No text, no letters, no numbers anywhere.
```

### F8 · Llega al HX711

Referencias: F7 + foto de tu HX711.

```
Same style and lighting as the first reference image, vertical 9:16. A small HX711
load cell amplifier breakout board sits on the dark workbench at a slight angle,
kept exactly as in the product reference photo: a small PCB with the HX711 chip in
the middle and a row of pin headers on each side. Four thin wires — red, black,
green and white — plug into one side, and the faint thread of light arrives along
the green and white wires, reaching the chip. Cool blue rim light, shallow depth
of field. No text, no letters, no numbers, no labels anywhere.
```

### F9 · Amplificada y convertida a digital

Referencia: F8.

```
Same scene, style and lighting as the reference image, vertical 9:16. The HX711
chip glows brightly from within. On the incoming side the signal is still a faint
thin thread; on the outgoing side it bursts out as a stream of crisp, evenly
spaced, glowing square pulses — bright rectangular blocks of light marching in a
line along one wire, with small points of light ticking rhythmically along a
second wire beside it, like a clock. Soft light bloom around the chip. No text, no
letters, no numbers, no binary digits anywhere.
```

### F10 · Llega al ESP32

Referencias: F9 + foto de tu ESP32-C6.

```
Same style and lighting as the first reference image, vertical 9:16. The two wires
carrying the square light pulses arrive at an ESP32-C6 development board lying on
the dark workbench, kept exactly as in the product reference photo. The pulses flow
into the board and its main chip lights up with a soft blue glow, with fine light
traces spreading through the circuit. Cool blue rim light, shallow depth of field.
No text, no letters, no numbers, no labels anywhere.
```

### F11 · Cierre con Tostabot

Referencias: F10 + Tostabot.

```
Same style, lighting and colour grade as the first reference image, vertical 9:16.
The camera has pulled back to a wide shot of the finished scale on the dark
workbench: the load cell with the steel weight on its platform, the HX711 and the
ESP32-C6 connected by wires, and a small round glowing display facing the camera.
Tostabot — the tagged character element, kept exactly as in his reference — stands
beside the assembly on the left, one wrench hand open toward it in a presenting
gesture. The round display shows only a clean luminous circular arc gauge filling
around its edge in brand blue, with a glowing dot at the end of the arc — no
digits.

Tostabot's two round glowing eyes stay visible, and his mouth is a small
semicircular smile of glowing LED dots — never a waveform.

Cinematic, warm key light, cool blue rim light. No text, no letters, no numbers
anywhere in the frame.
```

> En F11 la pantalla **no lleva el número a propósito**: el modelo casi nunca
> dibuja dígitos limpios. Pon el peso real en la edición, o corta a una toma real
> de tu báscula.

---

## Movimiento — Kling 3.0 · 9:16 · 5 s · sin audio nativo

En los clips donde sale Tostabot, **etiquétalo como Elemento** y usa el fotograma
inicial y final. Movimiento lento y suave, sin cortes dentro del clip.

**Clip 1 · F0 → F1** *(con Tostabot)*
```
Tostabot turns slightly toward the camera and raises one wrench hand in a small
presenting gesture toward the load cell, speaking. His semicircular LED smile
flexes and changes shape as he talks, always staying a smile — never a waveform,
never a straight line — and his two round glowing eyes stay visible throughout.
Meanwhile the load cell glides smoothly into place, mounts onto its base, the
platform settles on its free end, and a steel weight descends gently and stops,
hovering above it. Slow cinematic motion, no camera cuts. No text anywhere.
```

**Clip 2 · → F2** *(con Tostabot)*
```
The steel weight drops softly onto the platform. The free end of the bar dips
under the load and a soft blue glow pulses from the holes in the middle. Tostabot
leans in and tilts his head toward the bending bar, still speaking: his
semicircular LED smile flexes and changes shape as he talks, always staying a
smile, and his round glowing eyes stay visible. Static camera. No text anywhere.
```

**Clip 3 · → F3** *(Tostabot sale por el encuadre)*
```
The camera pushes in slowly and continuously toward the middle of the load cell
until it becomes an extreme macro, so Tostabot naturally leaves the frame at the
edge — he does not vanish or fade, the camera simply moves past him. As the shot
closes in, the white coating turns semi-transparent, revealing the fine zig-zag
metal foil of the strain gauge, which begins to glow. No text anywhere.
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

**Clip 6 · → F6** *(vuelve Tostabot)*
```
The camera pulls back smoothly and continuously, revealing the diamond circuit as
a large floating hologram and bringing Tostabot into frame from the left as the
shot widens — he walks into view naturally, he does not appear out of nowhere. He
stops beside the hologram and raises one wrench hand toward it, speaking. His
semicircular LED smile flexes and changes shape as he talks, always staying a
smile — never a waveform — and his round glowing eyes stay visible. The hologram's
blue light reflects on his body. No text anywhere.
```

**Clip 7 · → F7** *(sale por el encuadre)*
```
Tostabot gestures down toward the wires and the camera pushes in past him,
following his gesture, so he leaves the frame at the edge as the shot closes in on
the lower corners of the diamond. A very faint, thin, flickering thread of light
emerges there and travels slowly away along a green and a white wire toward the
bottom of the frame. No text anywhere.
```

**Clip 8 · → F8**
```
The camera follows the faint thread of light along the wires as it travels down
into a small HX711 amplifier board, which comes into focus as the light reaches
its chip. Smooth tracking motion. No text anywhere.
```

**Clip 9 · → F9**
```
The HX711 chip glows brighter and brighter. The weak incoming thread of light
enters, and out of the other side bursts a stream of crisp, evenly spaced square
pulses of light marching along one wire, while small points of light tick
rhythmically along a second wire beside it, like a clock. Soft bloom around the
chip. Static camera. No text, no digits anywhere.
```

**Clip 10 · → F10**
```
The camera tracks along the wires as the square pulses of light travel into an
ESP32-C6 board. As they arrive, the main chip lights up with a soft blue glow and
fine traces of light spread across the circuit. Smooth tracking motion.
No text anywhere.
```

**Clip 11 · → F11** *(cierre con Tostabot)*
```
The camera pulls back slowly to reveal the complete scale — load cell with the
weight, HX711 and ESP32-C6 connected, and the round display facing the camera —
bringing Tostabot into frame from the left as the shot widens. He stops beside the
assembly and opens one wrench hand toward it in a presenting gesture, speaking his
last line. His semicircular LED smile flexes and changes shape as he talks, always
staying a smile, and his round glowing eyes stay visible. A luminous blue arc
sweeps smoothly around the edge of the round display and stops. Slow, satisfying
reveal. No text, no digits anywhere.
```

---

## Datos técnicos que sostiene la narración

| Dato | Valor |
|---|---|
| Galgas en la celda de barra | 4, en puente de Wheatstone completo |
| Salida típica de la celda | ~1 mV por volt de excitación a carga máxima |
| Ganancia del HX711 | 128 en el canal A (64 opcional; 32 en el canal B) |
| Resolución del HX711 | ADC de 24 bits |
| Interfaz | 2 hilos propios (DT y SCK) — **no es I2C** |
| Velocidad | 10 u 80 muestras por segundo |
| Cables de la celda | Rojo E+ · Negro E− · Verde A+ · Blanco A− (convención común; **verifica la hoja de datos de tu celda**) |

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado

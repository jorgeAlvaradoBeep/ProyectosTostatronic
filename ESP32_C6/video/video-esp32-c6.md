# Video — ESP32-C6 N4 y SuperMini

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado
Formato: **9:16** · Narración principal: **47.1 s** · **10 clips de 5 s = 50 s**
Después: blooper de 2 clips (10 s) — plan aparte, cuando llegue su SRT
Voz: Ballesteros (ElevenLabs, una toma) · SRT: `principal.srt`

---

## ⚠️ Revisa esto antes de seguir: 13.1 – 13.6 s

El SRT transcribió **"zigbee, z Wave y Matter"**. Escribí *"zred"* (Thread),
pero si al oírlo suena a **"Z-Wave"**, es un error de contenido: **el ESP32-C6
no tiene Z-Wave**. Es otro protocolo.

- **Si suena a "Thread":** no hay que hacer nada; fue la transcripción.
- **Si suena a "Z-Wave":** en la edición recorta de ≈13.1 a ≈13.5 s para que
  quede *"…para Zigbee y Matter"*. Sigue siendo cierto y no hay que regenerar.

---

## Tostabot — cara y reglas

| | |
|---|---|
| **Ojos** | Dos puntos redondos brillantes. **Siempre visibles.** |
| **Boca** | **Sonrisa semicircular** de puntos LED que se deforma al hablar, pero **sigue siendo sonrisa**. |
| **Nunca** | Onda, línea recta, labios, ojos cerrados. |

Etiquétalo como **Elemento** en Kling. Entra y sale solo por movimiento de
cámara, nunca aparece de golpe.

```
Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.
```

---

## Código de color de los radios

Se usa en los clips 2 a 4 para que se entienda que son **tres radios distintos**:

| Radio | Color |
|---|---|
| WiFi 6 | **Azul eléctrico** |
| Bluetooth 5 | **Blanco / cian** |
| 802.15.4 (Zigbee · Thread · Matter) | **Verde** |

---

## Plan de clips — calculado sobre el SRT

| Clip | Ventana | Qué se dice (SRT) | Qué se ve | Tostabot | Fotogramas |
|---|---|---|---|---|---|
| **1** | 0 – 5 s | 0.1 "¡Conoce la ESP32-C6!" · 3.4 "La primera ESP32 con WiFi 6…" | Tostabot levanta la N4; de la antena salen anillos **azules** | ✅ | F0 → F1 |
| **2** | 5 – 10 s | "…WiFi 6." (6.7) · 7.2 "Pero no se queda ahí:" · 8.5 "además de WiFi y Bluetooth…" | Deja la placa en la mesa; a los anillos azules se suman los **blancos** | ✅ | → F2 |
| **3** | 10 – 15 s | 10.5 "…5, trae un tercer radio para Zigbee, Thread y Matter." (14.6) | La cámara entra a la placa; nace un tercer anillo **verde** | sale | → F3 |
| **4** | 15 – 20 s | 14.9 "O sea, puede hablar directo con focos, sensores y enchufes inteligentes." (18.5) · 19.1 "Su cerebro:" | Los hilos verdes llegan a un foco, un sensor y un enchufe inteligentes, que se encienden | — | → F4 |
| **5** | 20 – 25 s | "…un procesador RISC-V a 160 megahertz, con 512 KB de memoria." (19.1 – 26.0) | Rayos X del módulo: el núcleo late con pulsos de reloj y la memoria brilla | — | → F5 |
| **6** | 25 – 30 s | 26.4 "Y viene en dos tamaños." · 28.0 "La N4: cuatro megas de flash, LED…" | La cámara se aleja: Tostabot con **las dos placas** lado a lado y señala la N4 | entra | → F6 |
| **7** | 30 – 35 s | 31.0 "…RGB integrado y lista para la protoboard." · 34.0 "Y la SuperMini…" | La N4 entra en la protoboard y su LED RGB cambia de color; Tostabot levanta la SuperMini | ✅ | → F7 |
| **8** | 35 – 40 s | 35.2 "…casi del tamaño de una estampilla, para proyectos donde cada milímetro cuenta." (39.4) | Macro: la SuperMini junto a una estampilla del mismo tamaño | sale | → F8 |
| **9** | 40 – 45 s | 39.9 "Las dos se programan con Arduino, MicroPython o ESPHome…" (44.3) | Laptop con las dos placas conectadas por USB-C; Tostabot teclea | entra | → F9 |
| **10** | 45 – 50 s | 44.5 "…¡y se llevan de maravilla con Home Assistant!" (47.1) | Casita inteligente en miniatura que se ilumina; Tostabot orgulloso | ✅ | → F10 |

**En la edición:** audio principal en 0 s y los 10 clips seguidos desde 0 s.
El blooper empieza en 50 s, sobre F10.

---

## Referencias — súbelas antes de empezar

- **Foto de tu ESP32-C6 N4** → F0, F1, F2, F3, F5, F6, F7, F9
- **Foto de tu ESP32-C6 SuperMini** → F6, F7, F8, F9
- **Tostabot** → en todos los que sale

**Nada de texto ni logotipos:** la pantalla de la laptop va desenfocada, la
estampilla sin letras y sin el logo de Home Assistant. Si quieres rotular
"WiFi 6", "Zigbee · Thread · Matter", "RISC-V 160 MHz" o "4 MB", hazlo en la
edición.

---

## Fotogramas — Nano Banana Pro · 9:16

Estilo común: mesa de trabajo gris carbón, luz cálida lateral, contraluz azul
eléctrico, profundidad de campo corta, fotorrealista.

### F0 · Inicio · Tostabot con la N4

Referencias: Tostabot + foto de la N4.

```
Photorealistic cinematic scene, vertical 9:16. Tostabot, the tagged character
element — a friendly boxy blue metallic robot, kept exactly as in his reference —
stands behind a dark charcoal electronics workbench, framed from the chest up in
the upper half of the frame. He holds an ESP32-C6 development board, exactly as in
its reference photo, between the jaws of one wrench hand at chest height, looking
at it with excitement.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light from the left, electric blue rim light, soft bokeh of shelves.
No text, no letters, no logos other than the markings on the board.
```

### F1 · 5 s · WiFi 6: anillos azules

Referencias: F0 + Tostabot + foto de la N4.

```
Same scene. Tostabot, the tagged character element, holds the ESP32-C6 board up
toward the camera. From the antenna end of the board, three concentric rings of
electric blue light radiate outward like ripples, glowing softly in the air and
lighting his metallic face blue. He looks at the camera, delighted.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic, vertical 9:16. No text, no logos other than the board's markings.
```

### F2 · 10 s · Se suma Bluetooth: anillos blancos

Referencias: F1 + Tostabot + foto de la N4.

```
Same scene, vertical 9:16. The ESP32-C6 board now lies on the workbench in front of
Tostabot, the tagged character element, who stands behind it gesturing toward it
with an open wrench hand. From the board's antenna two kinds of light rings radiate
and interleave: electric blue rings and white-cyan rings, clearly different colours.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic. No text, no logos other than the board's markings.
```

### F3 · 15 s · El tercer radio: anillos verdes

Referencias: F2 + foto de la N4.

```
Low macro shot, vertical 9:16. The ESP32-C6 board on the dark workbench fills the
lower half of the frame, the robot out of frame. From its antenna, three distinct
families of light rings radiate into the air: electric blue, white-cyan, and — the
newest and brightest — vivid green rings. The green rings stretch furthest, toward
the right edge of the frame, turning into thin green threads of light.
Photorealistic, shallow depth of field. No text, no logos other than the board's
markings.
```

### F4 · 20 s · La casa inteligente responde

Referencia: F3.

```
Photorealistic, vertical 9:16. Further along the dark workbench, a small group of
smart home devices: a smart LED light bulb in a small lamp base glowing warm white,
a small white wireless sensor with a tiny green status light, and a compact smart
plug with a green status light. Thin threads of vivid green light arrive from the
left edge of the frame and connect to each device. In the far left background, out
of focus, the development board glows. Warm, cosy light. No text, no letters, no
brand names, no logos.
```

### F5 · 25 s · El cerebro: RISC-V

Referencias: F4 (luz) + foto de la N4.

```
Extreme macro, vertical 9:16. The metal shield of the ESP32-C6 module on the board
has become translucent, x-ray style, revealing the silicon chip inside. At the
chip's centre, a compact processor core glows electric blue and emits rhythmic
rings of light like a heartbeat — a clock pulse. Around it, a neat grid of memory
blocks glows softly in white-blue. The rest of the board is sharp but darker.
Clean, technical, photorealistic cutaway. No text, no letters, no labels.
```

### F6 · 30 s · Dos tamaños

Referencias: Tostabot + foto de la N4 + foto de la SuperMini.

```
Photorealistic, vertical 9:16. The camera has pulled back to the workbench. Side by
side on the bench, lying flat: on the left, the ESP32-C6 N4 development board,
exactly as in its reference photo; on the right, the much smaller ESP32-C6
SuperMini board, exactly as in its reference photo, so the size difference is
obvious. Tostabot, the tagged character element, stands behind them, framed from
the chest up, pointing at the larger N4 board with one wrench hand.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light, electric blue rim light. No text, no logos other than the boards'
markings.
```

### F7 · 35 s · La N4 en la protoboard · la SuperMini en la mano

Referencias: F6 + Tostabot + foto de la N4 + foto de la SuperMini.

```
Photorealistic, vertical 9:16. In the lower half of the frame, the ESP32-C6 N4
board is pressed into a breadboard, and its small onboard RGB LED glows bright
magenta, casting coloured light on the breadboard. In the upper half, Tostabot, the
tagged character element, holds the tiny ESP32-C6 SuperMini board up between the
jaws of one wrench hand, showing it to the camera.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

No text, no logos other than the boards' markings.
```

### F8 · 40 s · Del tamaño de una estampilla

Referencias: F7 (luz) + foto de la SuperMini.

```
Extreme macro, top-down view, vertical 9:16. On the dark charcoal workbench, the
tiny ESP32-C6 SuperMini board, exactly as in its reference photo, lies right next to
an ordinary postage stamp of almost the same size — the stamp has a perforated edge
and a simple abstract illustration, with NO text, no numbers, no letters. The
similar size is obvious at a glance. Soft warm light, subtle electric blue rim.
No text other than the board's own markings.
```

### F9 · 45 s · Se programan con lo que ya usas

Referencias: Tostabot + foto de la N4 + foto de la SuperMini.

```
Photorealistic, vertical 9:16. The camera has pulled back. On the dark workbench, an
open laptop whose screen shows soft, out-of-focus coloured blocks of code — nothing
readable. Two USB-C cables run from the laptop to the ESP32-C6 N4 board on its
breadboard and to the small ESP32-C6 SuperMini beside it. Tostabot, the tagged
character element, sits at the laptop, framed from the chest up, typing with his
wrench hands and glancing at the camera.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm light, electric blue accents. No readable text, no logos.
```

### F10 · 50 s · Casa inteligente

Referencias: F9 + Tostabot + foto de la SuperMini.

```
Photorealistic, vertical 9:16. On the workbench, a small wooden model of a house
with open windows; inside, tiny lights glow warm white, and a thin green thread of
light connects the house to the ESP32-C6 SuperMini board lying in front of it.
Tostabot, the tagged character element, stands beside the little house, framed from
the chest up, holding up the SuperMini proudly in one wrench hand, the other hand on
his hip.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a big, proud upward curve. Never a waveform, never a straight line, never real lips.

Warm, cosy light, electric blue rim light. No text, no logos.
```

---

## Clips — Kling 3.0 · 5 s · 9:16

| | |
|---|---|
| Modelo | Kling 3.0 en Higgsfield |
| **Audio nativo** | **Apagado** |
| Elemento Tostabot | Clips 1, 2, 3, 6, 7, 8, 9, 10 |

**Prompt negativo (todos):**

```
text, letters, subtitles, captions, watermark, logos, extra robots, extra limbs,
duplicate boards, deformed hands, waveform mouth, straight line mouth, human lips,
closed eyes, flicker, sudden teleport, cut, scene change
```

### Clip 1 · 0–5 s · F0 → F1

```
Tostabot, the tagged character element, lifts the ESP32-C6 board toward the camera
with excitement, talking the whole time: his semicircular LED smile flexes as if
speaking, his round glowing eyes stay on. Around the third second, rings of electric
blue light begin to radiate from the antenna end of the board, one after another,
lighting his metal face blue. Slow push-in. No text.
```

### Clip 2 · 5–10 s · → F2

```
Tostabot lowers the board and places it on the workbench in front of him, still
talking, then gestures toward it with an open wrench hand. The electric blue rings
keep radiating from its antenna, and in the last two seconds a second family of
white-cyan rings starts to radiate between them. His LED smile flexes as he speaks,
his eyes stay on. Steady camera. No text.
```

### Clip 3 · 10–15 s · → F3

```
The camera pushes in and down toward the board on the workbench, moving past
Tostabot, who slides naturally out of frame at the top edge — he does not vanish.
The blue and white-cyan rings keep radiating, and a third family of vivid green
rings bursts out, brighter than the others, stretching toward the right edge of the
frame and turning into thin green threads of light. No text.
```

### Clip 4 · 15–20 s · → F4

```
The camera follows the thin green threads of light as they travel to the right along
the dark workbench and reach a small group of smart home devices. One by one, the
threads connect: the smart bulb lights up warm white, the small wireless sensor's
status light turns green, the smart plug's light turns green. Continuous tracking
shot, no cuts. No text, no logos.
```

### Clip 5 · 20–25 s · → F5

```
The camera glides back to the left and dives into an extreme macro of the board's
metal module. The shield turns translucent, x-ray style, revealing the silicon chip:
its central processor core lights up electric blue and starts pulsing rhythmically
like a heartbeat, and around it a grid of memory blocks lights up block by block.
Smooth, continuous, no cuts. No text, no labels.
```

### Clip 6 · 25–30 s · → F6

```
The x-ray glow fades and the camera pulls back smoothly, revealing the N4 board lying
on the workbench next to the much smaller SuperMini board. As the camera widens,
Tostabot, the tagged character element, is revealed standing behind them — he does
not pop in. He points at the larger N4 board while talking: his semicircular LED
smile flexes, his eyes stay on. No text.
```

### Clip 7 · 30–35 s · → F7

```
Tostabot presses the N4 board into a breadboard in the lower part of the frame, and
its small onboard RGB LED lights up and smoothly cycles through red, green, blue and
magenta. Then he picks up the tiny SuperMini board with one wrench hand and holds it
up to the camera, talking with excitement: his LED smile flexes, his eyes stay on.
Steady camera. No text other than the boards' markings.
```

### Clip 8 · 35–40 s · → F8

```
The camera follows Tostabot's wrench hand as it lowers the tiny SuperMini board onto
the workbench and pushes in to an extreme top-down macro; Tostabot leaves the frame
naturally as the camera moves in. A postage stamp slides into frame right next to
the board, revealing that they are almost exactly the same size. Hold on the pair.
No text on the stamp.
```

### Clip 9 · 40–45 s · → F9

```
The camera rises and pulls back from the macro, revealing an open laptop on the
workbench with USB-C cables running to the SuperMini and to the N4 board on its
breadboard. Tostabot, the tagged character element, is revealed sitting at the
laptop — he does not pop in — typing quickly with his wrench hands and glancing at
the camera while talking: his LED smile flexes, his eyes stay on. The screen stays
blurred and unreadable. No text, no logos.
```

### Clip 10 · 45–50 s · → F10

```
Tostabot stands up from the laptop, picks up the SuperMini and turns to a small
wooden model of a house on the workbench. A thin green thread of light jumps from
the board to the house and its tiny windows light up warm white, one after another.
He holds the board up proudly with a big smile — his semicircular LED smile widens,
his eyes stay on — and puts his other hand on his hip. Slow push-in. No text, no
logos.
```

---

## Si algún clip sale mal

| Problema | Qué hacer |
|---|---|
| Tostabot aparece o desaparece de golpe | Regenera; si insiste, agrega *"One continuous shot, no cuts."* |
| La placa cambia de forma o gana componentes | Regenera con la foto de la placa como referencia extra. |
| Los tres colores de anillos se mezclan | En la edición rotula "WiFi 6", "Bluetooth 5" y "Zigbee · Thread · Matter" con el mismo color. |
| La estampilla sale con letras | Regenera F8 o difumina la estampilla en la edición. |

---

## Datos que sostiene la narración (fichas de tostatronic.com)

| Frase | Dato |
|---|---|
| "La primera ESP32 con WiFi 6" | El ESP32-C6 fue el primer chip de Espressif con WiFi 6 (802.11ax), 2.4 GHz. |
| "Bluetooth 5" | Bluetooth 5 (LE). |
| "Tercer radio: Zigbee, Thread y Matter" | IEEE 802.15.4. **No incluye Z-Wave.** |
| "RISC-V a 160 MHz" | Procesador RISC-V de 32 bits, 160 MHz. |
| "512 KB de memoria" | 512 KB de SRAM. |
| "N4: 4 megas de flash, LED RGB" | Módulo ESP32-C6-WROOM-1, 4 MB de flash, LED RGB direccionable, 4.6 × 2.8 cm. |
| "SuperMini… del tamaño de una estampilla" | Formato ultracompacto. |
| "Arduino, MicroPython o ESPHome… Home Assistant" | Entornos listados en ambas fichas. |

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com)

# Video explicativo — Display de 7 segmentos: ¿cátodo o ánodo común?

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado
Formato: **9:16** · Duración objetivo: **~70 s** · 14 clips de 5 s encadenados
Narrador: **Tostabot**

---

## La idea del video

El tema que confunde es **cátodo común vs ánodo común**, así que el video se
construye alrededor de un solo código de color que se mantiene de principio a fin:

| | Color | Idea visual |
|---|---|---|
| **Cátodo común** | **Azul eléctrico** | El común va **abajo**, a tierra. La señal que enciende es un pulso **brillante** (un 1). |
| **Ánodo común** | **Rojo / naranja** | El común va **arriba**, al voltaje. La señal que enciende es un **hueco oscuro** (un 0). |

Si el estudiante solo recuerda "azul abajo, rojo arriba", ya entendió la diferencia.

---

## Tostabot — cara y reglas

| | |
|---|---|
| **Ojos** | Dos puntos redondos brillantes en la matriz LED. **Siempre visibles.** |
| **Boca** | La **sonrisa semicircular** de puntos LED. Al hablar se deforma, pero **sigue siendo sonrisa**. |
| **Nunca** | Onda de audio. Línea recta. Labios. Ojos cerrados o apagados. |

**Etiquétalo como Elemento en Kling.** En los prompts aparece como
*"Tostabot, the tagged character element"*.

### Bloque de cara — va en cada fotograma donde sale Tostabot

```
Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.
```

---

## Dónde sale Tostabot

| Clips | Tostabot | Qué se ve |
|---|---|---|
| **1** | Presenta con un display en cada mano | Plano medio |
| **2 – 5** | Fuera (la cámara entra al display) | Macro → rayos X → holograma cátodo común |
| **6** | Vuelve: la cámara se aleja y él señala el holograma | Explica cátodo común |
| **7 – 8** | Fuera | Holograma ánodo común · la lógica invertida |
| **9** | Con las puntas del multímetro | La prueba para saber cuál tienes |
| **10 – 13** | Fuera | Macro de la prueba · ESP32 · driver · aplicaciones |
| **14** | Cierra con una resistencia en la mano | Plano medio |

**Regla de continuidad:** Tostabot nunca aparece ni desaparece de golpe. Entra
y sale porque la cámara se acerca, se aleja o se desplaza.

---

## Encadenado y referencias

```
F0 ──clip 1──▶ F1 ──clip 2──▶ F2 … ──clip 14──▶ F14
```

- **Clip 1:** inicio = F0, final = F1.
- **Clip 2 en adelante:** inicio = último cuadro del clip anterior, final = F(n).
- **Genera cada fotograma usando el anterior como referencia.** Si sale
  Tostabot, agrega su imagen como segunda referencia.

**Fotos de producto — súbelas antes de empezar:**
- **Tu display de 1 dígito** (el que vendes). F0 – F4, F9 – F12, F14.
- **Tu ESP32** para F11.
- **Tu multímetro** para F9 (opcional; sin foto sale uno genérico).

**Nada de texto** en ningún fotograma. Los únicos "caracteres" permitidos son
los segmentos encendidos del display. Rótulos como "a–g", "GND", "VCC",
"cátodo común" los pones en la edición.

---

## Narración — ElevenLabs · Ballesteros · una sola toma

*(Aún no generada. Me dices y la saco.)*

```
[curious] ¿Cátodo común o ánodo común? La duda número uno con estos displays.

Un display de siete segmentos son siete leds en forma de ocho. Muchos traen un octavo: el punto decimal.

Para ahorrar pines, todos comparten una patita: el común.

Si comparten el negativo, es cátodo común: el común va a tierra, y cada segmento prende con un uno.

Si comparten el positivo, es ánodo común: el común va al voltaje, y prende con un cero. [short pause] La lógica se invierte.

¿Cuál tienes? Multímetro en modo diodo, punta roja al común. [short pause] Si prende un segmento, es ánodo común.

Cátodo común se lleva perfecto con un microcontrolador o un max siete dos uno nueve. Ánodo común, con drivers que absorben corriente.

Los ves en relojes, básculas, contadores y marcadores.

[cheerfully] Eso sí: si lo conectas directo, una resistencia por segmento.
```

| Clip | Frase |
|---|---|
| 1 | ¿Cátodo común o ánodo común? La duda número uno con estos displays. |
| 2 | Un display de siete segmentos son siete LEDs en forma de ocho. |
| 3 | Muchos traen un octavo: el punto decimal. |
| 4 | Para ahorrar pines, todos comparten una patita: el común. |
| 5 | Si comparten el negativo, es cátodo común: el común va a tierra… |
| 6 | …y cada segmento prende con un uno. |
| 7 | Si comparten el positivo, es ánodo común: el común va al voltaje, y prende con un cero. |
| 8 | La lógica se invierte. |
| 9 | ¿Cuál tienes? Multímetro en modo diodo, punta roja al común. |
| 10 | Si prende un segmento, es ánodo común. |
| 11 | Cátodo común se lleva perfecto con un microcontrolador o un MAX7219. |
| 12 | Ánodo común, con drivers que absorben corriente. |
| 13 | Los ves en relojes, básculas, contadores y marcadores. |
| 14 | Eso sí: si lo conectas directo, una resistencia por segmento. |

---

## Fotogramas — Nano Banana Pro · 9:16

Estilo común: mesa de trabajo gris carbón, luz cálida lateral, contraluz azul
eléctrico, profundidad de campo corta, fotorrealista.

### F0 · Tostabot con un display en cada mano *(inicio del clip 1)*

Referencias: Tostabot + foto de tu display.

```
Photorealistic cinematic scene, vertical 9:16. Tostabot, the tagged character
element — a friendly boxy blue metallic robot, kept exactly as in his reference —
stands behind a dark charcoal electronics workbench, framed from the chest up in
the upper half of the frame. In each wrench hand he holds up one identical
single-digit seven-segment LED display, exactly as in its reference photo, both
unlit. He looks from one to the other with a curious tilt of his head, as if
comparing them.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light from the left, electric blue rim light on the left display, warm
red-orange rim light on the right display. Soft bokeh of shelves behind him.
No text, no letters, no numbers.
```

### F1 · Los dos displays encendidos *(final del clip 1)*

Referencias: F0 + Tostabot + foto del display.

```
Same scene and framing. Tostabot, the tagged character element, now holds both
displays toward the camera. Both are lit, each showing the full figure eight with
its decimal point, in bright red segments: all seven segments and the dot glowing.
The left display has a faint electric blue halo around it, the right one a faint
warm red-orange halo. He looks at the camera.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic, vertical 9:16. No text other than the lit segments.
```

### F2 · Macro: los 7 segmentos *(final del clip 2)*

Referencias: F1 + foto del display.

```
Extreme macro, vertical 9:16. A single seven-segment LED display fills the frame,
exactly as in its reference photo, held at its edges by the blurred blue metallic
jaws of a robot wrench hand. Its seven bar-shaped segments glow bright red forming
the figure eight; the small decimal point in the lower right corner is still OFF
and dark. The robot's body is completely out of frame. Dark background, razor-sharp
focus on the segments, subtle red light spill on the jaws.
No text other than the lit segments.
```

### F3 · El punto decimal *(final del clip 3)*

Referencia: F2.

```
Same extreme macro, same framing. Now the small round decimal point in the lower
right corner also glows bright red, slightly brighter than the rest, with a soft
bloom around it. All seven segments remain lit. Vertical 9:16.
No text other than the lit segments.
```

### F4 · Rayos X: el pin común *(final del clip 4)*

Referencia: F3.

```
Same framing, vertical 9:16. The display's body has become translucent like smoked
glass, x-ray style. Under each of the eight glowing segments sits a tiny square
LED chip. Each LED chip has two thin gold wires: one wire from every chip runs to
its own separate metal leg, and the other wire from every chip joins a single
shared metal trace that runs along the inside and ends in one common leg, which
glows much brighter than the others, in white-blue light. Eight separate legs, one
shared glowing leg. Clean, technical, photorealistic cutaway.
No text, no letters, no labels.
```

### F5 · Holograma: cátodo común *(final del clip 5)*

Referencia: F4.

```
Vertical 9:16. The x-ray display has dissolved into a floating electric blue
holographic circuit diagram above the dark workbench. It shows eight identical
LED diode symbols standing side by side in a row, each drawn as a triangle pointing
DOWN onto a short bar. The top of each diode has its own separate input line going
up. The bottoms of all eight diodes are joined by one thick horizontal glowing line
that runs down into a ground symbol at the bottom centre. On three of the diodes, a
bright pulse of light is travelling down from the input, through the diode, into
the shared bottom line, and those three diodes glow brightly. All blue.
Elegant, clean, technical. No text, no letters, no numbers.
```

### F6 · Tostabot explica el cátodo común *(final del clip 6)*

Referencias: F5 + Tostabot.

```
Vertical 9:16. The camera has pulled back. The same floating electric blue
hologram of eight diodes with their bottoms joined to a ground symbol hovers above
the workbench on the right half of the frame. Tostabot, the tagged character
element, stands on the left, framed from the waist up, pointing at the shared
bottom line with his wrench hand like a teacher. The hologram casts blue light
on his metallic body.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic. No text, no letters, no numbers.
```

### F7 · Holograma: ánodo común *(final del clip 7)*

Referencia: F6.

```
Vertical 9:16. The camera has pushed in past Tostabot — he is out of frame. The
hologram has flipped and changed colour to warm red-orange. Now the eight LED diode
symbols stand in a row with their triangles pointing DOWN, but their TOPS are all
joined by one thick horizontal glowing line that connects up to a positive power
rail across the top of the frame, glowing red. The bottom of each diode has its own
separate line going down. On three of the diodes, light flows from the shared top
line, down through the diode and out the bottom, and those three glow brightly.
All in warm red-orange. Clean, technical. No text, no letters, no numbers.
```

### F8 · La lógica invertida *(final del clip 8)*

Referencia: F7.

```
Vertical 9:16. Two floating holographic seven-segment digits side by side above
the dark workbench, identical in shape. The LEFT digit is drawn in electric blue
light; above it, a single input line carries a bright glowing pulse — a solid bar
of light. The RIGHT digit is drawn in warm red-orange light; above it, the same
input line carries a dark hollow gap — an empty outline where the light is missing.
Despite the opposite signals, both digits light exactly the same top segment.
Symmetric composition, clean and graphic. No text, no letters, no numbers.
```

### F9 · Tostabot con el multímetro *(final del clip 9)*

Referencias: Tostabot + foto del display + foto del multímetro (opcional).

```
Photorealistic, vertical 9:16. On the dark charcoal workbench, a single-digit
seven-segment display, exactly as in its reference photo, is pressed into a small
breadboard. Tostabot, the tagged character element, leans over it, framed from the
chest up, holding a red multimeter probe in one wrench hand and a black probe in
the other. The red probe tip touches one of the two middle pins of the display,
the black probe touches another pin. A handheld digital multimeter lies on the
bench beside the breadboard, its screen turned away from the camera. One segment of
the display glows faintly red.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light, electric blue rim light. No text, no letters, no numbers.
```

### F10 · Macro: el segmento prende *(final del clip 10)*

Referencia: F9 + foto del display.

```
Extreme macro, vertical 9:16. The seven-segment display on the breadboard fills the
frame. The red probe tip touches one middle pin, the black probe tip touches another
pin; both probes enter from the edges of the frame, the robot out of frame. Exactly
one segment — the top one — glows softly red, clearly lit against the dark unlit
segments. Shallow depth of field, focus on the glowing segment.
No text other than the lit segment.
```

### F11 · Cátodo común con ESP32 *(final del clip 11)*

Referencias: F10 (luz y color) + foto del display + foto de tu ESP32.

```
Photorealistic, vertical 9:16. The camera has pulled back: on the dark workbench,
the same breadboard now holds the seven-segment display wired to an ESP32
development board, exactly as in its reference photo, with neat short jumper wires
and a row of eight small resistors between the board and the display. The display
shows a bright red digit 3. A subtle electric blue glow outlines the wires. Warm
key light from the left. No text other than the lit segments and the markings
already printed on the board.
```

### F12 · Ánodo común con driver *(final del clip 12)*

Referencia: F11 (luz y color) + foto del display.

```
Photorealistic, vertical 9:16. A second breadboard further along the same dark
workbench. A seven-segment display is wired through a row of small resistors into a
black 16-pin DIP integrated circuit. Tiny glowing warm red-orange particles of
current flow from the top power rail into the display, through the resistors, and
INTO the chip, making the direction of flow obvious. The display shows a bright red
digit 7. Warm red-orange rim light. No text, no letters, no logos other than the
lit segments.
```

### F13 · Aplicaciones *(final del clip 13)*

Referencia: F12 (luz y color).

```
Photorealistic, vertical 9:16. A long dark workbench seen at a low angle, receding
into depth, with a row of everyday devices that all use red seven-segment LED
digits: in front, a digital alarm clock; behind it, a small kitchen scale; behind
that, a handheld tally counter with a digit display; at the back, a small tabletop
scoreboard. Each shows bright red digits. Shallow depth of field, the front device
sharp. Warm light, electric blue accents.
No letters, no brand names, no logos — only the lit digits.
```

### F14 · Cierre *(final del clip 14)*

Referencias: F13 + Tostabot + foto del display.

```
Photorealistic, vertical 9:16. Tostabot, the tagged character element, stands
centred behind the dark workbench, facing the camera, framed from the chest up. He
holds up a single small resistor between the jaws of one wrench hand, like showing
an important tool. In front of him, the seven-segment display on the breadboard
glows with a red figure eight and decimal point. Warm key light, electric blue rim
light. Leave the upper fifth and lower fifth of the frame calm for the logo and
the closing title.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

No text other than the lit segments.
```

---

## Clips — Kling 3.0 · 5 s · 9:16

### Configuración para los 14

| | |
|---|---|
| Modelo | Kling 3.0 en la plataforma de Higgsfield |
| Duración | 5 s |
| Formato | 9:16 |
| **Audio nativo** | **Apagado.** La voz sale de ElevenLabs. |
| Elemento | Tostabot etiquetado en los clips 1, 2, 6, 7, 9, 10 y 14 |
| Fotogramas | Clip 1: F0 → F1. Del 2 en adelante: último cuadro del anterior → F(n) |

**Prompt negativo (el mismo para todos):**

```
text, letters, subtitles, captions, watermark, extra robots, extra limbs,
duplicate displays, deformed hands, waveform mouth, straight line mouth, human
lips, closed eyes, flicker, sudden teleport, cut, scene change
```

---

### Clip 1 · F0 → F1

```
Tostabot, the tagged character element, looks from the display in his left hand to
the display in his right hand, curious, then raises both toward the camera. As he
lifts them, both displays light up, segment by segment, into a bright red figure
eight with its decimal point. The left display gains a faint electric blue halo,
the right one a warm red-orange halo. He talks to the camera: his semicircular LED
smile flexes as if speaking, his round glowing eyes stay on. Slow push-in.
No text.
```

### Clip 2 · → F2

```
The camera pushes in smoothly toward the display in Tostabot's hand, moving past
his face — he is still talking — until he slides naturally out of the frame at the
edge. He does not vanish; the camera simply moves past him. The display fills the
frame and its seven segments light up one by one in order, forming a bright red
figure eight. The decimal point stays dark. No text.
```

### Clip 3 · → F3

```
Static extreme macro of the lit figure eight. After a beat, the small round decimal
point in the lower right corner switches on with a soft bloom of red light. A very
slow push-in toward it. Calm and precise. No text.
```

### Clip 4 · → F4

```
The display's body gradually turns translucent like smoked glass, x-ray style,
revealing a tiny LED chip under each segment with thin gold wires. One wire from
every chip lights up in turn and all of them flow together into one single shared
trace, which runs to one common leg that ends up glowing brightly in white-blue.
Smooth transformation, no cuts. No text.
```

### Clip 5 · → F5

```
The x-ray display dissolves upward into floating electric blue light that
reassembles into a holographic diagram of eight diode symbols in a row, their
bottoms joined by one thick line that runs down into a ground symbol. Pulses of
bright light travel down through three of the diodes into the shared bottom line,
and those diodes glow. Everything in electric blue. Elegant and clean. No text.
```

### Clip 6 · → F6

```
The camera pulls back smoothly from the blue hologram, revealing the dark workbench
and Tostabot, the tagged character element, who enters the frame from the left edge
as the camera widens — he does not appear suddenly, the camera reveals him. He
points at the shared bottom line of the hologram like a teacher. Bright pulses keep
flowing down through the diodes. His LED smile flexes as he talks, his round glowing
eyes stay on. Blue light on his metallic body. No text.
```

### Clip 7 · → F7

```
Tostabot keeps talking as the camera pushes in toward the hologram; he slides
naturally out of frame at the left edge. The hologram rotates and flips: the shared
line moves from the bottom to the top of the diodes and connects to a positive power
rail across the top, and the whole diagram changes colour from electric blue to
warm red-orange. Light now flows from the shared top line down through three diodes.
Smooth, continuous, no cuts. No text.
```

### Clip 8 · → F8

```
The red-orange diagram folds into two floating holographic seven-segment digits side
by side: the left one turns electric blue, the right one stays red-orange. Above the
blue digit, a bright solid pulse of light arrives on its input line; above the red
digit, a dark empty gap arrives instead. At the same instant, both digits light the
same top segment. The camera holds steady on the symmetric pair. No text.
```

### Clip 9 · → F9

```
The two holographic digits shrink and drift down onto the workbench, where the light
settles into a real seven-segment display pressed into a breadboard. The camera
pulls back and Tostabot, the tagged character element, is revealed leaning over it —
he does not pop in. He brings a red multimeter probe and a black probe to the pins.
He talks while doing it: his semicircular LED smile flexes, his eyes stay on.
Photorealistic. No text.
```

### Clip 10 · → F10

```
The camera pushes in to an extreme macro of the display, moving past Tostabot, who
slides out of frame naturally. The red probe tip touches one of the middle pins, the
black probe touches another pin, and at the moment of contact the top segment softly
lights up red. Shallow depth of field, focus on the glowing segment. No text.
```

### Clip 11 · → F11

```
The probes lift away and the camera pulls back smoothly, revealing jumper wires and
a row of small resistors connecting the display to an ESP32 development board on the
same breadboard. The display switches from the single glowing segment to a bright
red digit 3. A faint electric blue glow travels along the wires from the board to
the display. Photorealistic. No text other than the lit digit.
```

### Clip 12 · → F12

```
The camera tracks smoothly to the right along the workbench, leaving the ESP32 board
behind, and arrives at a second breadboard where another display is wired through
resistors into a black 16-pin chip. Tiny warm red-orange particles of current flow
from the top power rail through the display and into the chip. The display shows a
bright red 7. Continuous tracking shot, no cuts. No text.
```

### Clip 13 · → F13

```
The camera keeps tracking and lowers to a low angle, gliding along the workbench past
a row of everyday devices with red seven-segment digits: a digital alarm clock, a
kitchen scale, a tally counter, a small scoreboard. Their digits change as the camera
passes. Focus racks from one device to the next. No letters, no logos.
```

### Clip 14 · → F14

```
The camera rises and pulls back, arriving at the front of the workbench where
Tostabot, the tagged character element, is revealed standing behind the breadboard
with the glowing figure eight — he does not pop in. He holds up a small resistor
between the jaws of his wrench hand and talks to the camera with a big happy smile:
his semicircular LED smile widens and flexes, his round glowing eyes stay on. Calm
ending. No text other than the lit segments.
```

---

### Si algún clip sale mal

| Problema | Qué hacer |
|---|---|
| Tostabot aparece o desaparece de golpe | Regenera. Si insiste, agrega al inicio: *"One continuous shot, no cuts."* |
| El display sale con más o menos segmentos | Regenera con la foto de tu display como referencia extra. |
| En el clip 9 el paso holograma → protoboard sale feo | Genera el 9 solo con F9 como inicio y pon una disolución de 10 cuadros desde el clip 8. |
| En el clip 8 no se entiende el pulso contra el hueco | Rotúlalo en la edición: **"1"** sobre el azul, **"0"** sobre el rojo. |

---

## Datos técnicos que sostiene la narración

Regla de Tostabot: nunca dice algo que no se pueda comprobar.

| Afirmación | Dato |
|---|---|
| "Siete LEDs en forma de ocho" | Segmentos **a, b, c, d, e, f, g**, en sentido horario desde arriba, y **g** al centro. |
| "Un octavo: el punto decimal" | **DP**. Por eso algunos lo llaman de 8 segmentos. |
| "Comparten una patita: el común" | En el formato más común de 1 dígito (10 pines), el común está en **los dos pines centrales (3 y 8)**, unidos por dentro. Revisa la hoja de datos de tu modelo. |
| "Cátodo común: a tierra, prende con un 1" | Común a GND. El segmento enciende con ALTO en su pin (a través de su resistencia). |
| "Ánodo común: al voltaje, prende con un 0" | Común a VCC. El segmento enciende con BAJO en su pin. |
| "Multímetro en diodo, punta roja al común" | La punta roja es la positiva. Si con la roja en el común y la negra en un segmento el segmento prende, el común es ánodo. Si no prende, invierte las puntas: si ahora prende, es cátodo común. |
| "MAX7219" | Driver diseñado para displays de **cátodo común**. Controla hasta 8 dígitos con 3 pines. |
| "Drivers que absorben corriente" | Ej. **74LS47** (decodificador BCD con salidas activas en bajo, para ánodo común) o un **ULN2803**. Para cátodo común, el equivalente clásico es el **CD4511**. |
| "Una resistencia por segmento" | Si pones una sola resistencia en el común, el brillo cambia según cuántos segmentos enciendas: un 8 se ve más tenue que un 1. **Excepción:** el MAX7219 regula la corriente con una sola resistencia (RSET), por eso la frase dice "si lo conectas directo". |

### Notas para la descripción o un segundo video

- **Valores de resistencia** (LED rojo, ~10 mA por segmento): a 5 V, 220–330 Ω; a 3.3 V, 100–150 Ω.
- **Ojo con ánodo común + ESP32.** Si pones el común a 5 V y manejas los segmentos
  desde un pin de 3.3 V, un ALTO no alcanza a apagar el segmento: quedan
  ~1.7 V y el segmento se ve encendido a medias ("fantasma"). Conecta el común
  a 3.3 V o usa transistores.
- **5161AS / 5161BS.** En muchos displays de este modelo la terminación **AS** es
  cátodo común y **BS** ánodo común. No es un estándar: confírmalo siempre con
  la prueba del multímetro.

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com)

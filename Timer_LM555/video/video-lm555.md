# Video explicativo — El temporizador LM555

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado
Formato: **9:16** · Duración objetivo: **~65 s** · 13 clips de 5 s encadenados
Narrador: **Tostabot**

---

## Tostabot — cara y reglas

| | |
|---|---|
| **Ojos** | Dos puntos redondos brillantes en la matriz LED. **Siempre visibles.** |
| **Boca** | La **sonrisa semicircular** de puntos LED. Al hablar se deforma, pero **sigue siendo sonrisa**. |
| **Nunca** | Onda de audio. Línea recta. Labios. Ojos cerrados o apagados. |

**Etiquétalo como Elemento en Kling** antes de empezar. En los prompts aparece
como *"Tostabot, the tagged character element"*.

### Bloque de cara — va en cada fotograma donde sale Tostabot

```
Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.
```

---

## Dónde sale Tostabot

En este video **él es el narrador**, así que sale más que en el de la celda:
**7 de los 13 clips**. Los otros 6 son el chip por dentro, donde él estorbaría.

| Clips | Tostabot | Qué se ve |
|---|---|---|
| **1 – 2** | Presenta el chip | Plano medio, lo sostiene en la mano |
| **3 – 4** | Fuera de cuadro (la cámara entra al chip) | Macro → rayos X → holograma interno |
| **5 – 6** | Vuelve: la cámara se aleja y él está junto al holograma | Explica el capacitor |
| **7 – 8** | Fuera de cuadro | El capacitor se carga, se descarga, sale el pulso |
| **9 – 10** | Entra por el costado y cambia el capacitor | Circuito real en protoboard |
| **11** | Fuera de cuadro | Paneo por aplicaciones |
| **12 – 13** | Cierra | Junto a un ESP32 y el circuito parpadeando |

**Regla de continuidad:** Tostabot nunca aparece ni desaparece de golpe. Entra
y sale porque la cámara se acerca, se aleja o se desplaza.

---

## Encadenado

```
F0 ──clip 1──▶ F1 ──clip 2──▶ F2 … ──clip 13──▶ F13
```

- **Clip 1:** inicio = F0, final = F1.
- **Clip 2 en adelante:** inicio = último cuadro del clip anterior, final = el
  siguiente fotograma.
- **Genera cada fotograma usando el anterior como referencia** (F1 desde F0, F2
  desde F1…). Si sale Tostabot, agrega su imagen como segunda referencia.

**Referencias de producto — súbelas antes de empezar:**
- Foto real de **tu LM555** (el que vendes). La uso en F0, F1, F2, F9, F12.
- Foto de **tu ESP32** para F12.

Así el chip sale con la **misma serigrafía que el tuyo**. Sin foto, el modelo
inventa marcas y logotipos.

**Nada de texto** en ningún fotograma, salvo la serigrafía del chip tal como
viene en tu foto. Los rótulos ("⅓", "⅔", "Comparador", "Flip-flop") los pones
en la edición: la IA los escribe mal.

---

## Narración — ElevenLabs · Ballesteros · una sola toma

*(Aún no generada. Me dices y la saco.)*

```
[curious] ¿Qué chip lleva más de cincuenta años dentro de miles de circuitos?

El ele eme cinco cinco cinco: un temporizador de ocho pines.

Por dentro tiene tres resistencias iguales, dos comparadores, un flip flop y un transistor de descarga.

Su truco es medir tiempo con un capacitor. [short pause] Se carga hasta dos tercios del voltaje... se descarga hasta un tercio... y vuelve a empezar.

[excited] El resultado: pulsos perfectamente medidos. Un reloj.

Puede oscilar solo, o dar un único pulso de duración exacta. Y el tiempo lo eliges con una resistencia y un capacitor. Sin programar nada.

Lo encuentras en luces intermitentes, alarmas, generadores de tono y control de motores por pe doble u eme.

¿Y hoy, con tantos microcontroladores? Sigue vigente: es barato, resistente y aguanta hasta dieciséis volts.

[cheerfully] Y no hay mejor forma de entender cómo se mide el tiempo en electrónica.
```

| Clip | Frase |
|---|---|
| 1 | ¿Qué chip lleva más de cincuenta años dentro de miles de circuitos? |
| 2 | El LM555: un temporizador de ocho pines. |
| 3 | Por dentro tiene… |
| 4 | …tres resistencias iguales, dos comparadores, un flip-flop y un transistor de descarga. |
| 5 | Su truco es medir tiempo con un capacitor. |
| 6 | Se carga hasta dos tercios del voltaje… |
| 7 | …se descarga hasta un tercio… y vuelve a empezar. |
| 8 | El resultado: pulsos perfectamente medidos. Un reloj. |
| 9 | Puede oscilar solo, o dar un único pulso de duración exacta. |
| 10 | Y el tiempo lo eliges con una resistencia y un capacitor. Sin programar nada. |
| 11 | Luces intermitentes, alarmas, generadores de tono, motores por PWM. |
| 12 | ¿Y hoy, con tantos microcontroladores? Sigue vigente: barato, resistente, hasta 16 V. |
| 13 | Y no hay mejor forma de entender cómo se mide el tiempo en electrónica. |

---

## Fotogramas — Nano Banana Pro · 9:16

Estilo común a todos: mesa de trabajo gris carbón oscura, luz cálida lateral,
contraluz azul eléctrico, profundidad de campo corta, fotorrealista.

### F0 · Tostabot presenta el chip *(inicio del clip 1)*

Referencias: Tostabot + foto de tu LM555.

```
Photorealistic cinematic scene, vertical 9:16. Tostabot, the tagged character
element — a friendly boxy blue metallic robot, kept exactly as in his reference —
stands behind a dark charcoal electronics workbench, framed from the chest up in
the upper half of the frame, facing the camera. On the bench in front of him, in
the lower third, sits a small solderless breadboard with a single black 8-pin DIP
integrated circuit pressed into it, four metal legs on each side, a small notch
at one end. Keep the chip and its marking exactly as in its reference photo.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light from the left, electric blue rim light from the right tracing his
metallic edges. Soft bokeh of shelves and parts drawers behind him. No text, no
letters, no captions anywhere except the marking already printed on the chip.
```

### F1 · Tostabot sostiene el chip *(final del clip 1)*

Referencias: F0 + Tostabot + foto del LM555.

```
Same scene, same lighting, same workbench. The camera has moved slightly closer.
Tostabot, the tagged character element, now holds the black 8-pin DIP chip up
between the jaws of his right wrench hand, at chest height, turned toward the
camera so its top face and legs are clearly visible. The empty breadboard stays
on the bench below. Keep the chip and its marking exactly as in its reference.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic, vertical 9:16. No text other than the chip's own marking.
```

### F2 · Macro del chip *(final del clip 2)*

Referencias: F1 + foto del LM555.

```
Extreme macro close-up, vertical 9:16. The black 8-pin DIP chip fills the centre
of the frame, still gripped at its edges by the blurred blue metallic jaws of a
robot wrench hand. Four silver legs on each side catch the light; the small notch
and pin-one dot are sharp. Keep the marking exactly as in the reference photo.
The robot's body is completely out of frame. Dark background with a soft electric
blue glow behind the chip. Photorealistic, razor-sharp focus on the chip surface.
No text other than the chip's own marking.
```

### F3 · Rayos X: el chip se vuelve transparente *(final del clip 3)*

Referencia: F2.

```
Same macro framing, vertical 9:16. The black epoxy body of the 8-pin chip has
become translucent like smoked glass, revealing its interior: a small square
silicon die glowing faintly in the centre, sitting on a metal pad, with eight
extremely thin gold bond wires arcing from the die to the inner ends of the eight
metal legs. The legs and the wrench jaws holding the chip are unchanged.
Electric blue light seeps out of the die. Photorealistic x-ray cutaway style.
No text, no letters, no labels.
```

### F4 · El circuito interno como holograma *(final del clip 4)*

Referencia: F3.

```
Vertical 9:16. The glowing silicon die has risen out of the chip and unfolded into
a floating electric blue holographic circuit diagram, centred in the frame, above
the now-small translucent chip. The hologram shows, clearly separated:
— on the left, a vertical chain of three identical resistors in series, glowing
  equally, connected from a top rail to a bottom rail;
— in the middle, two triangular comparator symbols stacked one above the other,
  each fed by one of the junctions between the resistors;
— to their right, a single rectangular block representing a flip-flop, receiving
  both comparator outputs;
— below it, one transistor symbol; to the far right, a triangular output buffer.
Thin luminous lines connect all blocks. Dark workshop background, soft bokeh.
Clean, elegant, technical. No text, no letters, no numbers, no labels.
```

### F5 · Tostabot junto al holograma *(final del clip 5)*

Referencias: F4 + Tostabot.

```
Vertical 9:16. The camera has pulled back. The same floating electric blue
holographic circuit diagram hovers above the dark charcoal workbench, now smaller,
on the right half of the frame. Tostabot, the tagged character element, stands on
the left, framed from the waist up, turned toward the hologram, pointing at it
with his wrench hand like a teacher at a whiteboard. Next to the hologram, a single
glowing holographic capacitor symbol has appeared, larger than the rest.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light, electric blue glow from the hologram on his metallic body.
Photorealistic. No text, no letters, no numbers.
```

### F6 · El capacitor se carga *(final del clip 6)*

Referencia: F5.

```
Vertical 9:16. The camera has pushed in past Tostabot — he is now out of frame —
toward the holographic capacitor, which fills the centre of the frame. It is drawn
as a tall transparent glass cylinder between two metal plates. Two thin horizontal
glowing lines cross the cylinder: a lower one at one third of its height and an
upper one at two thirds. Bright electric blue light fills the cylinder from the
bottom up and has just reached the upper line, glowing more intensely there.
Dark background with soft bokeh. No text, no letters, no numbers.
```

### F7 · El capacitor se descarga *(final del clip 7)*

Referencia: F6.

```
Same framing, same capacitor cylinder and the same two glowing level lines, at
one third and two thirds of its height. The electric blue light has drained down
and now sits exactly at the lower line. A brief spark of light flows out of the
bottom of the cylinder through a thin glowing wire toward the right edge of the
frame. Vertical 9:16. No text, no letters, no numbers.
```

### F8 · Nace el pulso *(final del clip 8)*

Referencia: F7.

```
Vertical 9:16. The capacitor cylinder has moved to the upper left of the frame,
still pulsing between its two level lines. From it, a bright electric blue
square-wave signal stretches across the frame as a luminous 3D ribbon: sharp flat
tops, sharp flat bottoms, perfectly vertical edges, evenly spaced, receding into
the distance like a road of light. Dark background. Clean and precise.
No text, no letters, no numbers, no grid.
```

### F9 · El circuito real en protoboard *(final del clip 9)*

Referencia: F8 (solo para luz y color) + foto del LM555.

```
Photorealistic macro, vertical 9:16. On the dark charcoal workbench, a small
solderless breadboard holds a classic astable timer circuit: the black 8-pin DIP
chip from the reference photo in the centre, two small resistors with colour
bands, one blue electrolytic capacitor, one ceramic capacitor, and a single red
5 mm LED glowing brightly. Neat short jumper wires in red, black and yellow. A
thin wire leaves the frame to the right toward a battery. Faint electric blue rim
light on the edges of the components, warm light from the left.
No text other than the chip's own marking.
```

### F10 · Tostabot cambia el capacitor *(final del clip 10)*

Referencias: F9 + Tostabot.

```
Same breadboard, same circuit, same lighting, vertical 9:16. The camera has
pulled back slightly and drifted left. Tostabot, the tagged character element,
has leaned in from the left edge, framed from the chest up, and is pressing a
new, smaller blue electrolytic capacitor into the breadboard with the jaws of his
wrench hand. The red LED is glowing. He looks at the circuit, pleased.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic. No text other than the chip's own marking.
```

### F11 · Aplicaciones *(final del clip 11)*

Referencia: F10 (solo para luz y color).

```
Photorealistic, vertical 9:16. A long dark charcoal workbench seen at a low angle,
receding into depth, with a row of small finished electronics projects, each built
on a small green PCB with one black 8-pin chip: in front, an amber warning beacon
flashing; behind it, a small round piezo buzzer; behind that, a tiny DC motor
spinning a small fan blade; at the back, a strip of blinking LEDs. Shallow depth
of field, the front project sharp and the others progressively blurred. Warm light,
electric blue accents. No text, no letters, no logos.
```

### F12 · El 555 junto al ESP32 *(final del clip 12)*

Referencias: F11 + Tostabot + foto del LM555 + foto de tu ESP32.

```
Photorealistic, vertical 9:16. On the dark charcoal workbench, side by side in the
lower third: on the left, a modern ESP32 development board exactly as in its
reference photo; on the right, the small breadboard with the black 8-pin DIP chip
and its glowing red LED. Tostabot, the tagged character element, stands behind
them, framed from the chest up, looking at the camera with a confident expression,
one wrench hand resting next to the breadboard, as if saying "this one still
matters".

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Warm key light from the left, electric blue rim light. No text other than the
markings already printed on the boards.
```

### F13 · Cierre *(final del clip 13)*

Referencias: F12 + Tostabot.

```
Same scene, vertical 9:16. The camera has pulled back to a medium-wide shot.
Tostabot, the tagged character element, stands centred behind the workbench,
facing the camera, one wrench hand raised in a friendly wave. In front of him the
ESP32 board and the breadboard with the glowing red LED. Warm workshop lighting,
electric blue rim light, soft bokeh. Leave the upper fifth and the lower fifth of
the frame calm and uncluttered for the logo and the closing title.

Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.

Photorealistic. No text, no letters.
```

---

## Clips — Kling 3.0 · 5 s · 9:16

### Configuración para los 13

| | |
|---|---|
| Modelo | Kling 3.0, en la plataforma de Higgsfield (ahí es ilimitado) |
| Duración | 5 s |
| Formato | 9:16 |
| **Audio nativo** | **Apagado.** La voz sale de ElevenLabs. Si el audio nativo está encendido, Kling le inventa otra voz a Tostabot. |
| Elemento | **Tostabot etiquetado** en los clips 1, 2, 5, 6, 10, 12 y 13 |
| Fotogramas | Clip 1: F0 → F1. Del clip 2 en adelante: **último cuadro del clip anterior** → F(n) |

**Prompt negativo (el mismo para todos):**

```
text, letters, subtitles, captions, watermark, extra robots, extra limbs,
duplicate chip, deformed hands, waveform mouth, straight line mouth, human lips,
closed eyes, flicker, sudden teleport, cut, scene change
```

Donde sale Tostabot, el prompt pide que **mueva la sonrisa como si hablara**,
aunque el clip no tenga voz. Así la boca coincide con la narración cuando
montes el audio encima.

---

### Clip 1 · F0 → F1 · "¿Qué chip lleva más de cincuenta años…?"

```
Tostabot, the tagged character element, reaches down to the breadboard with his
wrench hand, carefully pulls out the small black 8-pin chip and lifts it up to
chest height, turning it toward the camera to show it off. He looks at the camera
while talking enthusiastically: his semicircular LED smile flexes and changes shape
as if speaking, his round glowing eyes stay on and blink softly once.
The camera slowly pushes in. Warm workshop light, electric blue rim light.
Smooth, steady, cinematic. No text.
```

### Clip 2 · → F2 · "El LM555: un temporizador de ocho pines."

```
The camera keeps pushing in smoothly toward the black 8-pin chip held in
Tostabot's wrench hand, moving past his face — Tostabot is still talking, his LED
smile moving, until he naturally slides out of the frame at the edge. He does not
vanish or fade: the camera simply moves past him. The shot ends in an extreme macro
of the chip, its eight silver legs catching the light, the robot jaws blurred at
the edges. Slow, precise, cinematic. No text.
```

### Clip 3 · → F3 · "Por dentro tiene…"

```
Extreme macro of the black 8-pin chip. The camera orbits very slightly around it.
The black epoxy body gradually turns translucent like smoked glass, from the edges
toward the centre, revealing the small square silicon die inside and eight thin
gold bond wires connecting it to the legs. Electric blue light begins to glow from
the die. Smooth transformation, no cuts. No text.
```

### Clip 4 · → F4 · "…tres resistencias iguales, dos comparadores, un flip-flop y un transistor de descarga."

```
The glowing silicon die rises slowly out of the translucent chip and unfolds in the
air like a blueprint opening, becoming a floating electric blue holographic circuit
diagram. Its parts light up one after another: first three identical resistors in a
vertical chain, then two triangular comparators, then a rectangular flip-flop block,
then a transistor. The camera tilts up gently to follow the hologram as it grows.
Elegant, precise, technical. No text, no numbers.
```

### Clip 5 · → F5 · "Su truco es medir tiempo con un capacitor."

```
The camera pulls back smoothly from the floating hologram, revealing the dark
workbench and Tostabot, the tagged character element, who enters the frame from
the left edge as the camera widens — he does not appear suddenly, the camera
reveals him. He turns toward the hologram and points at it like a teacher. A single
larger glowing capacitor symbol lights up next to the circuit. His LED smile flexes
as he talks, his round glowing eyes stay on. The hologram casts blue light on his
metallic body. No text.
```

### Clip 6 · → F6 · "Se carga hasta dos tercios del voltaje…"

```
Tostabot keeps pointing and talking as the camera pushes in toward the holographic
capacitor; he slides naturally out of the frame at the left edge, the camera moving
past him. The capacitor grows into a tall transparent glass cylinder between two
metal plates, with two thin glowing lines across it at one third and two thirds of
its height. Bright electric blue light rises steadily from the bottom and reaches
the upper line, which flares brighter. Smooth, continuous. No text, no numbers.
```

### Clip 7 · → F7 · "…se descarga hasta un tercio… y vuelve a empezar."

```
Static framing on the glass capacitor cylinder with its two glowing level lines.
The electric blue light drains quickly down from the upper line to the lower line;
as it drains, a bright spark of light shoots out of the bottom through a thin
glowing wire toward the right edge of the frame. Then the light starts rising again.
Rhythmic, clean, satisfying. No text, no numbers.
```

### Clip 8 · → F8 · "El resultado: pulsos perfectamente medidos. Un reloj."

```
The capacitor cylinder drifts to the upper left corner, still filling and emptying
in a steady rhythm. With every cycle, a new rectangular pulse of electric blue light
is emitted and extends a luminous 3D ribbon of square pulses across the frame, sharp
flat tops and bottoms, perfectly vertical edges, evenly spaced. The camera glides
forward along the ribbon as it recedes into the distance. No text, no grid.
```

### Clip 9 · → F9 · "Puede oscilar solo, o dar un único pulso de duración exacta."

```
The camera follows the glowing ribbon of square pulses as it dives downward and
flows into a real breadboard on a dark workbench; the light fades into the circuit
and the scene becomes a photorealistic macro of the breadboard with the black 8-pin
chip, two resistors, a blue electrolytic capacitor and a red LED. As the light
arrives, the red LED starts blinking steadily, about once per second.
Smooth continuous camera move, no cuts. No text other than the chip's marking.
```

### Clip 10 · → F10 · "Y el tiempo lo eliges con una resistencia y un capacitor."

```
The camera pulls back slightly and drifts left. Tostabot, the tagged character
element, leans in from the left edge — he enters because the camera reveals him —
pulls the blue electrolytic capacitor out of the breadboard with his wrench hand
and presses a smaller one in its place. The red LED immediately starts blinking
much faster. He looks at it, pleased, and talks: his semicircular LED smile flexes
as if speaking, his round glowing eyes stay on. Photorealistic. No text other than
the chip's marking.
```

### Clip 11 · → F11 · "Luces intermitentes, alarmas, generadores de tono, motores por PWM."

```
The camera tracks smoothly to the right and lowers to a low angle, leaving Tostabot
behind at the left edge — he slides out of frame naturally as the camera moves
away. It glides along a long dark workbench revealing a row of small finished
electronics projects: an amber warning beacon flashing, a small piezo buzzer, a
tiny DC motor spinning a small fan blade, a strip of blinking LEDs. Shallow depth
of field, focus racking from one project to the next. No text, no logos.
```

### Clip 12 · → F12 · "¿Y hoy, con tantos microcontroladores? Sigue vigente…"

```
The camera rises and pulls back from the row of projects, arriving at the front of
the workbench where an ESP32 development board and the breadboard with the black
8-pin chip and blinking red LED sit side by side. Tostabot, the tagged character
element, is revealed behind them as the camera settles — he does not pop in. He
rests one wrench hand next to the breadboard and talks confidently to the camera:
his semicircular LED smile flexes as if speaking, his eyes stay on.
Warm key light, electric blue rim light. No text other than the boards' markings.
```

### Clip 13 · → F13 · "Y no hay mejor forma de entender cómo se mide el tiempo en electrónica."

```
Tostabot, the tagged character element, finishes talking to the camera with a big
happy smile — his semicircular LED smile widens, his round glowing eyes stay on —
and raises one wrench hand in a friendly wave. The camera slowly pulls back to a
medium-wide shot. In front of him, the red LED keeps blinking steadily. Calm
ending, the upper and lower parts of the frame stay uncluttered. No text.
```

---

### Si algún clip sale mal

| Problema | Qué hacer |
|---|---|
| Tostabot aparece o desaparece de golpe | Regenera. Si insiste, agrega al inicio del prompt: *"One continuous shot, no cuts."* |
| La boca vuelve a ser onda o línea | Confirma que el Elemento esté etiquetado y regenera. |
| En el clip 9 la transición holograma → protoboard sale fea | Genera el clip 9 **solo con F9 como inicio** (sin final) y en la edición pon una disolución de 10 cuadros desde el clip 8. |
| El chip cambia de forma o gana patas | Regenera con la foto de tu LM555 como referencia adicional. |

---

## Datos técnicos que sostiene la narración

Regla de Tostabot: nunca dice algo que no se pueda comprobar.

| Afirmación | Dato |
|---|---|
| "Más de cincuenta años" | Diseñado en 1971 por Hans Camenzind para Signetics; a la venta en 1972 como NE555. |
| "Ocho pines" | Encapsulado DIP-8: 1 GND · 2 TRIG · 3 OUT · 4 RESET · 5 CTRL · 6 THR · 7 DIS · 8 VCC. |
| "Tres resistencias iguales" | Divisor interno que fija los umbrales de ⅓ y ⅔ de VCC. |
| "Dos comparadores, un flip-flop, un transistor de descarga" | Diagrama de bloques del datasheet del LM555 (Texas Instruments). |
| "Se carga hasta ⅔, se descarga hasta ⅓" | Funcionamiento en modo astable. |
| "Oscila solo / un único pulso" | Modos astable y monoestable. También existe el biestable; no se menciona por tiempo. |
| "Lo eliges con una resistencia y un capacitor" | Astable: f ≈ 1.44 / ((R1 + 2·R2)·C). Monoestable: t ≈ 1.1·R·C. |
| "Hasta dieciséis volts" | LM555: alimentación de 4.5 V a 16 V. Salida de hasta 200 mA. |
| "Pulsos" | En el astable clásico la señal es rectangular: el tiempo en alto siempre es un poco mayor que en bajo. Por eso decimos "pulsos" y no "onda cuadrada perfecta". |

> **Sobre el nombre "555":** es muy común leer que viene de las tres
> resistencias de 5 kΩ. Camenzind contó que el número lo eligió Signetics y no
> tiene que ver con eso. Por eso la narración no lo menciona.

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com)

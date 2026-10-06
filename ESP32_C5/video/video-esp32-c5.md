# Video — ESP32-C5 DevKit y Mini

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado
Formato: **9:16** · Narración: **41.4 s** · **9 clips de 5 s = 45 s** (F0 a F9)
Voz: Ballesteros (ElevenLabs, una toma) · SRT: `principal.srt`
Clips: generados en Higgsfield, sin audio nativo, con fotograma inicial y final.
El intro real del producto va antes del clip 1 (lo agrega Jorge en la edición).

## Código de color de los radios

| Radio | Color |
|---|---|
| WiFi 2.4 GHz (saturado) | **Ámbar / naranja**, ondas enredadas |
| WiFi 5 GHz | **Azul eléctrico**, ondas limpias |
| Bluetooth 5 | **Blanco / cian** |
| 802.15.4 (Zigbee · Thread · Matter) | **Verde** |

## Plan de clips — calculado sobre el SRT

| Clip | Ventana | Qué se dice | Qué se ve | Tostabot | Fotogramas |
|---|---|---|---|---|---|
| 1 | 0 – 5 s | 0.1 "¿Tu WiFi de 2.4 está saturado?" · 3.1 "Conoce la ESP32-C5:" | Aire lleno de ondas ámbar enredadas; Tostabot levanta la DevKit | sí | F0 → F1 |
| 2 | 5 – 10 s | 6.5 "la primera ESP32 con WiFi 6 de doble banda." | Acercamiento a la placa; de la antena salen anillos ámbar y azules | sí | → F2 |
| 3 | 10 – 15 s | "Sí: 2.4 y 5 gigahertz. Menos interferencia," | Dos carriles: el ámbar congestionado, el azul despejado | sale | → F3 |
| 4 | 15 – 20 s | "conexión más estable…" · 16.9 "y además Bluetooth 5, Zigbee, Thread y Matter." | Se suman anillos blancos y verdes; un foco y un sensor se encienden | no | → F4 |
| 5 | 20 – 25 s | 21.1 "Su procesador RISC-V llega a 240 megahertz." | Rayos X del módulo: el núcleo late | no | → F5 |
| 6 | 25 – 30 s | 25.3 "La DevKit trae dos puertos USB-C y LED RGB." · "La Mini:" | La cámara se aleja: DevKit con dos cables USB-C y LED RGB; Tostabot la señala | entra | → F6 |
| 7 | 30 – 35 s | 30.7 "la misma potencia en un tamaño diminuto." · 33.5 "Se programan con Arduino," | Tostabot muestra la Mini en su palma junto a la DevKit | sí | → F7 |
| 8 | 35 – 40 s | 35.2 "ESP-IDF o MicroPython." · 37.9 "¡Pídela en tostatronic" | Laptop con las dos placas conectadas; Tostabot teclea | sí | → F8 |
| 9 | 40 – 45 s | "punto com, con envíos a todo México!" (termina 41.4) | Tostabot presenta las dos placas junto a una caja de envío; 3.6 s para logo y créditos | sí | → F9 |

Nota: el SRT escribió "mater" y "riscfave"; es la transcripción, el audio dice Matter y RISC-V.

## Referencias para cada fotograma

- Foto real de la **ESP32-C5 DevKit** → F0, F1, F2, F4, F5, F6, F7, F8, F9
- Foto real de la **ESP32-C5 Mini** → F7, F8, F9
- **Tostabot** → F0, F1, F2, F6, F7, F8, F9
- Para F1 en adelante, sube también el fotograma anterior como referencia de escena.

## Bloque fijo (pégalo al final de cada prompt)

```
Vertical 9:16, photorealistic 3D render, warm electronics workbench, soft studio
light, shallow depth of field. No text, no letters, no numbers, no logos, no
watermarks anywhere. The circuit boards must match the reference photos exactly.
```

Cara de Tostabot (pégalo en los fotogramas donde sale):

```
Tostabot's LED dot-matrix face shows two round glowing eyes that stay visible at
all times, and below them a small semicircular smile made of glowing LED dots —
a gentle upward curve. Never a waveform, never a straight line, never real lips.
```

## Prompts de fotogramas

**F0** — Tostabot stands at the right of a wooden workbench, medium shot, looking up at the air around him, which is crowded with tangled, overlapping amber-orange radio wave rings coming from several small blurred gadgets in the background. He holds the ESP32-C5 DevKit board low at his chest, not yet featured.

**F1** — Same scene and camera. Tostabot now raises the ESP32-C5 DevKit board up toward the camera with one hand, the board centered and sharp in the frame. The tangled amber waves behind him are dimmer and out of focus.

**F2** — Same scene, camera pushed in closer: the ESP32-C5 DevKit in Tostabot's hand fills the middle of the frame, his face still visible at the upper edge. From the board's antenna rise two sets of concentric rings side by side: amber-orange on the left, clean electric blue on the right.

**F3** — Camera has moved past the board; Tostabot is out of frame. The ESP32-C5 DevKit antenna is at the bottom edge. Above it the air splits into two glowing lanes: a narrow amber lane jammed with tangled waves on the left, and a wide, clean electric-blue lane on the right with a smooth stream of light particles flowing freely upward.

**F4** — The ESP32-C5 DevKit now rests flat on the workbench, three-quarter view from above, no robot in frame. Four sets of rings emanate from it: amber, electric blue, white-cyan and green. Thin green light threads reach a small smart light bulb and a small white smart sensor on the bench, both glowing softly.

**F5** — Extreme macro of the metal-shielded module on the ESP32-C5 DevKit, seen as a translucent x-ray cutaway: inside, a square processor core glows bright blue-white with pulse lines radiating along the copper traces. The colored rings are gone. No robot in frame.

**F6** — Camera pulled back to a medium shot: the ESP32-C5 DevKit on the workbench with two USB-C cables plugged into its two ports and its onboard RGB LED glowing magenta. Tostabot has entered at the right side of the frame, leaning in and pointing at the board with one finger.

**F7** — Same scene and camera. Tostabot holds his open palm forward at center frame with the tiny ESP32-C5 Mini board resting on it, clearly much smaller than the DevKit, which stays on the bench beside his hand with its RGB LED now glowing green.

**F8** — Same workbench, slightly wider. An open laptop with a blurred, unreadable screen sits at the left; the ESP32-C5 DevKit and the ESP32-C5 Mini are both connected to it with USB-C cables, their LEDs lit. Tostabot stands at the right with both hands on the keyboard, looking at the screen.

**F9** — Final hero shot, same workbench. Tostabot faces the camera at center, holding the ESP32-C5 DevKit in one hand and the ESP32-C5 Mini in the other, next to a small plain brown cardboard shipping box with no printing. Clean dark background with empty space in the upper third of the frame.

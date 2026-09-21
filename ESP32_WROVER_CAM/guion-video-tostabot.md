# Tostabot — producción del video ESP32-WROVER-CAM

Desarrollado por **Tostatronic** — Ing. Jorge Alvarado

---

## Lo que cambia con el audio nativo

Kling 3.0 genera **la voz y el movimiento de la boca al mismo tiempo**. Eso
elimina por completo el problema de sincronía: ya no hay que cortar entre un
clip callado y uno hablando, ni cuadrar una pista de ElevenLabs. El modelo
entrega el clip con la onda de la boca ya sincronizada con lo que dice.

**Y como el día es ilimitado, la estrategia es otra: fuerza bruta.**
Genera 4-6 tomas de cada clip y quédate con la mejor. No hay penalización por
intentar. Eso importa mucho aquí, porque el riesgo real de este video no es la
imagen — es la **pronunciación de los términos técnicos**.

### La pronunciación ya viene resuelta

Los prompts llevan los términos técnicos **escritos fonéticamente** más la
instrucción explícita de leerlos tal cual. Aun así, escucha cada toma: si alguna
sale rara, regenera — hoy no cuesta.

---

## Estructura del video — 75 segundos

| # | Bloque | Quién | Duración | Formato |
|---|---|---|---|---|
| **A** | Presentación del producto | Jorge en cámara | 22 s | Grabado |
| **B** | Entrada a Tostabot | Jorge | 4 s | Grabado |
| **C** | Especificaciones | **Tostabot** | **25 s** | Kling 3.0 |
| **D** | Demostración funcionando | Producto real | 20 s | Grabado |
| **E** | Cierre | Jorge | 8 s | Grabado |

El bloque C son **dos clips de Kling de 12-13 segundos**.

---

## Bloque A — Tu guion en cámara (22 s)

> Esta es la **ESP32-WROVER-CAM**.
>
> Es una placa de desarrollo con un ESP32 adentro… y una cámara de verdad,
> una OV2640 de dos megapíxeles.
>
> Con esto armas una cámara WiFi, un timbre con video, un robot que ve —
> y se programa como cualquier placa, con un cable USB normal.
>
> Nada de adaptadores raros.

*(Tomas sugeridas: placa en la mano girándola lento, close-up de la cámara,
close-up del conector Micro-USB.)*

---

## Bloque B — La entrada (4 s)

> Pero mejor que te lo explique bien…
> **nuestro amigo Tostabot** te va a dar las características específicas.

*(Puedes voltear a ver fuera de cuadro, o señalar hacia donde va a entrar.)*

---

## Bloque C — Tostabot (25 s, 2 clips)

Los términos técnicos van **escritos fonéticamente** en el diálogo. Así es como
el modelo los pronuncia bien:

| Se escribe | Se lee |
|---|---|
| `e-ese-pe treinta y dos uróver cam` | ESP32-WROVER-CAM |
| `pe-ese-ram` | PSRAM |
| `u-ese-be` | USB |
| `pinaut` | pinout |
| `wifi` | WiFi |

Los prompts incluyen además la instrucción explícita de pronunciar el diálogo
tal cual está escrito, letra por letra donde va con guiones.

---

### Clip 1 · ~12 s · horizontal 16:9

Audio nativo **activado**. Imagen a video con el fotograma base.

```
A friendly boxy blue robot with a metallic finish stands at a tidy electronics
workbench. Its face is a blue LED dot-matrix display with two round glowing eyes
and, below them, a glowing horizontal audio waveform as a mouth. The waveform
mouth ripples and oscillates in sync with its speech, like an audio level meter on
an LED matrix, and flattens into a straight line when it pauses.

It is framed in a medium shot from the waist up, centered, facing the camera.

The robot speaks cheerfully in neutral Mexican Spanish, with a warm young male voice
and a subtle robotic timbre. Pronounce the dialogue exactly as written, letter by
letter where it is hyphenated, saying:
"¡Hola! Soy Tostabot. Esta es la e-ese-pe treinta y dos uróver cam: cámara de dos megapíxeles, wifi, y cuatro megas de pe-ese-ram. Esa memoria es la que guarda la imagen completa."

It gestures lightly with its wrench hands while explaining.

Static camera, warm workshop lighting, shallow depth of field, blurred workbench
background. No text, no letters, no captions, no subtitles anywhere in the frame.
```

### Clip 2 · ~13 s · horizontal 16:9

```
A friendly boxy blue robot with a metallic finish stands at a tidy electronics
workbench. Its face is a blue LED dot-matrix display with two round glowing eyes
and, below them, a glowing horizontal audio waveform as a mouth. The waveform
mouth ripples and oscillates in sync with its speech, like an audio level meter on
an LED matrix, and flattens into a straight line when it pauses.

It is framed in a medium shot from the waist up, centered, facing the camera.

The robot speaks in neutral Mexican Spanish with a warm young male voice and a subtle
robotic timbre — excited at first, then switching to a friendly warning tone.
Pronounce the dialogue exactly as written, letter by letter where it is hyphenated,
saying:
"¿Y lo mejor? Trae u-ese-be integrado. Un cable y a programar, sin adaptadores. Ojo: la cámara ocupa catorce pines. Revisa el pinaut antes de conectar nada."

It raises one wrench hand in a small cautionary gesture on the last sentence.

Static camera, warm workshop lighting, shallow depth of field, blurred workbench
background. No text, no letters, no captions, no subtitles anywhere in the frame.
```

---

### Clip 1 · ~12 s · vertical 9:16

```
A friendly boxy blue robot with a metallic finish stands at a tidy electronics
workbench. Its face is a blue LED dot-matrix display with two round glowing eyes
and, below them, a glowing horizontal audio waveform as a mouth. The waveform
mouth ripples and oscillates in sync with its speech, like an audio level meter on
an LED matrix, and flattens into a straight line when it pauses.

It is framed in a vertical medium shot from the chest up, centered in the upper
two thirds of the frame, facing the camera, with the blurred workbench filling the
lower third.

The robot speaks cheerfully in neutral Mexican Spanish, with a warm young male voice
and a subtle robotic timbre. Pronounce the dialogue exactly as written, letter by
letter where it is hyphenated, saying:
"¡Hola! Soy Tostabot. Esta es la e-ese-pe treinta y dos uróver cam: cámara de dos megapíxeles, wifi, y cuatro megas de pe-ese-ram. Esa memoria es la que guarda la imagen completa."

It gestures lightly with its wrench hands while explaining.

Static camera, warm workshop lighting, shallow depth of field, blurred workbench
background. No text, no letters, no captions, no subtitles anywhere in the frame.
```

### Clip 2 · ~13 s · vertical 9:16

```
A friendly boxy blue robot with a metallic finish stands at a tidy electronics
workbench. Its face is a blue LED dot-matrix display with two round glowing eyes
and, below them, a glowing horizontal audio waveform as a mouth. The waveform
mouth ripples and oscillates in sync with its speech, like an audio level meter on
an LED matrix, and flattens into a straight line when it pauses.

It is framed in a vertical medium shot from the chest up, centered in the upper
two thirds of the frame, facing the camera, with the blurred workbench filling the
lower third.

The robot speaks in neutral Mexican Spanish with a warm young male voice and a subtle
robotic timbre — excited at first, then switching to a friendly warning tone.
Pronounce the dialogue exactly as written, letter by letter where it is hyphenated,
saying:
"¿Y lo mejor? Trae u-ese-be integrado. Un cable y a programar, sin adaptadores. Ojo: la cámara ocupa catorce pines. Revisa el pinaut antes de conectar nada."

It raises one wrench hand in a small cautionary gesture on the last sentence.

Static camera, warm workshop lighting, shallow depth of field, blurred workbench
background. No text, no letters, no captions, no subtitles anywhere in the frame.
```

> **Los dos clips de un mismo formato tienen que salir del mismo fotograma
> base**, o Tostabot cambia entre uno y otro. Y el fotograma vertical se genera
> aparte: no recortes el horizontal, porque pierdes la cara.

---

## Los fotogramas base — Nano Banana Pro

Antes de generar video, crea el fotograma que vas a alimentarle a Kling. Nano
Banana Pro también es ilimitado hoy, así que genera varias opciones.

**Imagen de entrada:** `tostabot-fondo-oscuro.png` (la versión limpia, sin halo).

### Fotograma principal — plano medio

Este es el importante. La cara tiene que salir **grande**: la boca de onda es
todo el gancho del personaje y necesita leerse.

```
Keep this exact robot character unchanged — same blue metallic boxy body, same
LED dot-matrix face, same beige tool pouch with the circular logo, same flexible
hose arms with wrench hands.

Reframe it as a medium shot from the waist up, centered, facing the camera. Place
it in a warm, tidy electronics workbench: soldering station, small parts drawers
and a green PCB softly blurred in the background with shallow depth of field.
Cinematic warm key light from the left, cool blue rim light from the right that
picks out the metallic edges. The LED face is the brightest thing in the frame.

On the LED display: two round glowing eyes and, below them, a single perfectly
straight horizontal glowing line as a mouth. Photorealistic product-render
quality. No text, no letters, no logos other than the one already on the pouch.
```

Formato: **16:9**, resolución **2k o 4k**.

### Fotograma alterno — plano entero

Por si quieres una toma de entrada o salida:

```
Keep this exact robot character unchanged. Show it full body, standing on a
wooden workbench surface, facing the camera, in a warm electronics workshop with
softly blurred shelves and components behind it. Cinematic warm lighting with a
cool blue rim light. On the LED display: two round glowing eyes and a straight
horizontal glowing line as a mouth. Photorealistic. No text, no letters.
```

### Fotograma vertical — para los clips 9:16

```
Keep this exact robot character unchanged — same blue metallic boxy body, same
LED dot-matrix face, same beige tool pouch with the circular logo, same flexible
hose arms with wrench hands.

Reframe it as a vertical medium shot from the chest up, centered in the upper two
thirds of the frame, facing the camera. Place it in a warm, tidy electronics
workbench: soldering station, small parts drawers and a green PCB softly blurred,
filling the lower third of the frame with shallow depth of field. Cinematic warm
key light from the left, cool blue rim light from the right that picks out the
metallic edges. The LED face is the brightest thing in the frame.

On the LED display: two round glowing eyes and, below them, a single perfectly
straight horizontal glowing line as a mouth. Photorealistic product-render
quality. No text, no letters, no logos other than the one already on the pouch.
```

Formato: **9:16**, resolución **2k o 4k**.

> No recortes el fotograma horizontal para hacer el vertical: pierdes la cara,
> que es lo único que importa en este personaje.

---

## Bloque D — La demostración (20 s)

Esta es la parte que **vende la placa**, y la tienes que grabar tú. Sin esto el
video es una presentación; con esto es una prueba.

### Lista de tomas

| # | Toma | Por qué |
|---|---|---|
| 1 | Close-up: conectar el cable Micro-USB a la placa | Muestra que es un cable normal |
| 2 | Pantalla: Arduino IDE terminando de cargar | Prueba que no hubo adaptador |
| 3 | Monitor serie mostrando la dirección IP | El momento "ya está viva" |
| 4 | Abrir esa IP en el navegador → aparece el video | El resultado |
| 5 | **Mover la mano frente a la cámara y verla moverse en la pantalla** | **La toma clave** |
| 6 | Plano abierto: placa en el escritorio y el monitor mostrando lo que ve | Cierra la idea |

> **La toma 5 es la más importante del video entero.** Es la única que demuestra
> que el video es en vivo y no una foto. Grábala en un solo plano, sin corte,
> con la placa y la pantalla en el mismo encuadre si se puede. Esa toma es la que
> la gente comparte.

---

## Bloque E — Cierre (8 s)

> Cámara, WiFi y memoria suficiente para trabajar con imagen, en una placa que
> se programa con un cable.
>
> La tienes en **tostatronic.com**.

---

## Insertos de datos (opcional, 3-4 s cada uno)

Dos gráficas del deck valen la pena sobre la voz de Tostabot:

- **`docs/w-psram.svg`** — la comparación 3.8 MB contra 520 KB. Entra durante la
  frase "cuatro megas de PSRAM" del Clip 1.
- **`docs/w-gpio.svg`** — el mapa de pines. Entra durante "la cámara ocupa
  catorce pines" del Clip 2.

Fuera de esos dos momentos, **nada de texto en pantalla**. El video es visual:
robot, placa real, y el navegador mostrando el video en vivo.

---

## Tostabot — ficha del personaje

Para que salga idéntico en todos los videos futuros.

| | |
|---|---|
| **Nombre** | Tostabot |
| **Rol** | Ayudante técnico de Tostatronic. Da las especificaciones. |
| **Cuerpo** | Robot cúbico azul eléctrico, acabado metálico, tornillos en las esquinas |
| **Cabeza** | Caja con visor: matriz LED azul, ojos redondos brillantes |
| **Boca** | Línea horizontal en el display. **Recta = callado. Onda = hablando.** |
| **Brazos** | Mangueras metálicas flexibles, manos tipo llave española |
| **Piernas** | Cortas, telescópicas, botas azules |
| **Detalle** | Bolsillo de lona beige con herramientas y el logo circular de Tostatronic |
| **Personalidad** | Entusiasta, directo, nerd. Nunca condescendiente. |
| **Voz** | Masculina joven, cálida, timbre robótico sutil. Español mexicano neutro. |

**Regla de oro:** Tostabot nunca dice algo que no sea verificable.
Su credibilidad es la de la marca.

---

## Paso a paso para hoy

1. **Genera el fotograma principal** en Nano Banana Pro, partiendo de
   `tostabot-fondo-oscuro.png`. Saca 4-6 y escoge el mejor.
2. **Genera el Clip 1** en Kling 3.0 con audio nativo, usando ese fotograma.
   Saca varias tomas. **Escucha la pronunciación**, no solo mires la imagen.
3. **Genera el Clip 2** con el mismo fotograma base.
4. **Graba tus bloques A, B y E.**
5. **Graba la demostración** siguiendo la lista de tomas — especialmente la 5.
6. **Monta** en el orden A → B → Clip 1 → Clip 2 → D → E.
7. **Mete los dos insertos** de gráfica sobre la voz de Tostabot.

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado
[tostatronic.com](https://www.tostatronic.com)

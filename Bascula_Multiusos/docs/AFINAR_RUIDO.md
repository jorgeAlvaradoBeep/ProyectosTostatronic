# Afinar ruido — ajustar los filtros a tu celda

Guía del asistente **MENÚ → Afinar ruido** (firmware ≥ 0.6). Escrita el 8 de
octubre de 2026.

## Para qué sirve

Cada celda de carga, cada mesa y cada fuente de alimentación hacen un ruido
distinto. Con los filtros de fábrica (pensados para un ruido "típico") puede
pasar que el último dígito baile en reposo, o que la báscula tarde en decir
**ESTABLE** porque le pide a la celda una quietud que no puede dar.

El asistente **escucha** la celda en reposo, mide cuánto ruido tiene y cuánto
tarda en asentarse al poner y quitar algo, y con eso fija tres cosas:

| Qué ajusta | De fábrica | Afinado |
|---|---|---|
| Ventana del filtro (muestras que se promedian) | 5 | de 5 a 15, según el ruido |
| Banda de estabilidad | ±1 división | ±1 división, o 3 sigmas del filtrado si la celda es más ruidosa |
| Promedio largo (ya estable) | α = 0.2 | de 0.03 a 0.2, según el ruido |

El resultado se guarda en la memoria de la placa (NVS): sobrevive a apagarla
y a actualizar el firmware. Independiente de esto, desde la v0.6 el último
dígito se **retiene**: ya estable, el valor mostrado no cambia hasta que la
lectura se aleja más de 0.6 divisiones. Eso solo ya quita el baile entre
−0.1, 0.0 y 0.1; el afinado hace que llegue antes a estable y que el valor
filtrado sea más limpio.

## Antes de empezar

- La báscula **en la mesa donde va a trabajar**, con su fuente definitiva y
  el WiFi como lo vaya a usar (si lo normal es tenerlo conectado, déjalo
  conectado: su ruido también cuenta).
- Calibrada (el asistente funciona sin calibrar, pero entonces reporta el
  ruido en cuentas y no en gramos).
- Un objeto cualquiera de **2 g o más** a la mano (una moneda, un tornillo,
  una tapa). No importa cuánto pese: no se usa para calibrar, solo para
  medir cuánto tarda la celda en asentarse.
- Durante los 3 pasos **no toques la mesa ni la báscula**, y evita
  ventiladores o corrientes de aire sobre el plato.

## Los tres pasos

Entra con **MENÚ → Afinar ruido** (está al final del menú, después de
Diagnóstico).

### Paso 1 · plato vacío

La pantalla dice *Retira todo del plato y no toques la mesa*, con la lectura
en vivo y si está estable. Quita todo y presiona **OK**.

Escucha **6 segundos**: el número grande cuenta hacia atrás, el anillo se
llena en azul y abajo va saliendo el ruido medido hasta el momento
(*ruido ±0.08 g*). Mientras dice *no toques nada*, no toques nada.

### Paso 2 · con un objeto

Dice *Coloca cualquier objeto de 2 g o más* y *lo detecta sola*: no hay que
presionar nada. Pon el objeto en el plato y retira la mano.

En cuanto la lectura se mueve, pasa a *asentándose…* y cronometra cuánto
tarda en llegar a **ESTABLE** (el anillo ámbar es el avance). Al asentarse,
vuelve a escuchar 6 segundos con el objeto encima.

Si en 8 segundos no se asienta, sigue de todos modos y lo anota (al final
dirá *no se asentó en 8 s*: ver "Si algo sale raro").

### Paso 3 · sin el objeto

Dice *Retira el objeto y no toques la mesa*. Quítalo y retira la mano.
Cronometra otra vez el asentamiento, y con eso termina.

### Listo

La pantalla **LISTO** resume lo que midió y aplicó:

```
Ruido ±0.08 g          ← desviación estándar de las muestras crudas, en reposo
filtro de 9 muestras   ← la ventana que eligió (de fábrica eran 5)
se asienta en 1.4 s    ← lo que tardó en llegar a ESTABLE, el peor de los dos
```

**OK** regresa a pesar, ya con los filtros nuevos. Por el monitor serie sale
una línea `[afinar]` con los números crudos y otra `[bascula]` con la ventana,
la banda y el alfa que quedaron.

## Cómo saber si sirvió

- Con el plato vacío y estable, el **0.0 debe quedarse quieto**.
- Al poner un peso, debe decir **ESTABLE** en más o menos el tiempo que
  reportó el asistente.
- En la página web, pestaña **Tiempo real**: la línea gris (cada muestra
  del HX711) baila; la azul (filtro) mucho menos; la blanca (lo que muestra
  la pantalla) va en escalones limpios. Abajo, *Filtro* dice las muestras y
  el ruido del perfil aplicado; si dice *de fábrica (sin afinar)*, el
  asistente no llegó a guardar.

## Cuándo repetirlo

- Si **mueves la báscula de mesa** o cambias la fuente de alimentación.
- Si **cambias de celda** o de capacidad en Ajustes (la división cambia y
  con ella la ventana que conviene).
- Si recalibras no hace falta, pero no estorba.

Repetirlo siempre sobreescribe el perfil anterior; no se acumula nada.

## Si algo sale raro

| Qué ves | Qué pasa | Qué hacer |
|---|---|---|
| *Sin celda: revisa el HX711* al presionar OK | No hay lecturas | Revisa DOUT, SCK y los 3.3 V del HX711 |
| *Sin lecturas: revisa la celda* al terminar una escucha | Llegaron menos de 10 muestras en 6 s | Igual que arriba; mira el Diagnóstico (muestras por segundo) |
| No detecta el objeto en el paso 2 o 3 | Movió la lectura menos del umbral (20 divisiones, o 8 sigmas si el ruido es grande) | Usa un objeto más pesado; MENÚ cancela y vuelve a empezar |
| Detectó el objeto antes de ponerlo | Un golpe en la mesa o una corriente de aire | MENÚ para cancelar y repite sin tocar nada |
| *no se asentó en 8 s* en la pantalla final | Con la banda que tenía, la lectura no se quedó quieta un segundo seguido | Normal en celdas ruidosas: el perfil nuevo ensancha la banda. Repite el asistente una vez más ya con el perfil aplicado; la segunda vez debería asentarse |
| Ruido de ±0.5 g o más con la celda de 1 kg | Mucho más de lo normal (lo típico es 0.05 a 0.15 g) | Revisa la alimentación (el HX711 quiere 3.3 V limpios), cables cortos y lejos del radio, la celda bien atornillada y nada rozando el plato. La prueba de WiFi del Diagnóstico dice si el radio es el culpable |
| Después de afinar tarda más en estabilizar que antes | Eligió una ventana grande porque el ruido medido fue alto | Es el precio de un último dígito quieto. Si prefieres respuesta rápida, reduce el ruido (arriba) y vuelve a afinar |

Para **volver a los filtros de fábrica** no hay opción en el menú: basta
repetir el asistente en condiciones limpias, o borrar las claves `ruido*`
del espacio `bascula` de la NVS (por ejemplo cargando una vez un sketch que
llame `nvs_flash_erase()`; también borra la calibración y el WiFi).

## Cómo lo calcula (para quien quiera meterse al código)

Todo está en `ruido.h` (funciones puras, sin Arduino) y se aplica en
`bascula.cpp`:

1. **Escucha.** La tarea de la celda acumula media, desviación estándar y
   pico a pico de las muestras **crudas** durante `AFINAR_ESCUCHA_MS`
   (6 s ≈ 60 muestras a 10 SPS). Se hace con el plato vacío y con el objeto;
   se toma la mayor de las dos desviaciones (σ).
2. **Ventana.** Promediar n muestras baja el ruido en √n. Se pide que a la
   salida del filtro quede en `RUIDO_SIGMA_OBJETIVO_DIV` = 0.3 divisiones:
   `n = (σ / 0.3 div)²`, acotado a 5…15 e impar (para que la mediana sea
   exacta). Con σ = 0.08 g y división 0.1 g: (0.8/0.3)² = 7.1 → 9.
3. **Banda de estabilidad.** `max(1 división, 3 · σ/√n)`: con una celda
   limpia sigue siendo ±1 división; con una ruidosa se ensancha lo justo
   para que *estable* sea alcanzable.
4. **Promedio largo.** Un promedio exponencial reduce el ruido en
   √(α/2). Se pide que lo mostrado quede en `RUIDO_PANTALLA_OBJETIVO_DIV` =
   0.15 divisiones: `α = 2 · (0.15 div / (σ/√n))²`, entre 0.03 y 0.2.
5. **Asentamiento.** Solo informativo: va a la pantalla final y a la pestaña
   Tiempo real.

Los objetivos (0.3 y 0.15 divisiones), los 6 s de escucha, el tope de 8 s y
el umbral de detección (20 divisiones) están en `config.h`, sección
*AFINAR RUIDO*.

# Pruebas de la fase 4 — página web y contador de piezas

Guía para probar el firmware **v0.4** en la báscula desde otro equipo
(teléfono u otra computadora). Escrita el 2 de octubre de 2026.

## Dónde quedó el trabajo

- La fase 4 está escrita y **compila para las 4 placas** (C6 DevKit, C6 Mini,
  C5 DevKit, C5 Mini), sin avisos en los archivos del proyecto.
- El simulador de pantalla corre limpio con las 49 pantallas y la página web
  se probó con datos de ejemplo.
- **Nada de la v0.4 se ha probado en una placa real.** La báscula (ESP32-C6
  Mini) sigue con la v0.3.
- Los cambios **no tienen commit**: el código de la fase 3 y la fase 4 solo
  está en la computadora donde se escribió.

## Qué llevar al otro equipo

El binario de la C6 Mini:

```
Bascula_Multiusos/Bascula_Multiusos/build/esp32.esp32.esp32c6/Bascula_v0.4_C6-Mini.ino.bin
```

Pesa 1.3 MB. **No viaja por git** (la carpeta `build/` está en el
`.gitignore`): hay que copiarlo a mano, por USB, Drive o correo.

Es solo para la **C6 Mini**. En otra placa arranca con los pines cambiados.

## Antes de empezar

- La báscula ya debe estar **conectada al WiFi**. Si no lo está, conéctate con
  el teléfono a la red `Tostatronic-Bascula`, elige tu red y pon la clave.
- El equipo de prueba debe estar **en la misma red WiFi** que la báscula.
- La dirección es `http://tostabascula.local`. Si no abre (pasa en algunos
  Android y Windows), usa la **IP** que muestra la báscula en
  **MENÚ → Conexión**.
- Para probar peso y conteo, la celda y el HX711 deben estar conectados. En la
  última prueba la pantalla decía "SIN CELDA".

## Pruebas

Van en este orden. Anota qué viste en cada una.

### 1. Actualizar el firmware por WiFi

Es la primera vez que se prueba la actualización sin cable.

1. En la báscula: **MENÚ → Conexión → OK → Actualizar**.
2. En el navegador: `http://tostabascula.local/actualizar`, elige el `.bin` y
   toca **Actualizar**.

| Debe pasar | Dónde se ve |
|---|---|
| La báscula muestra la dirección y "permitido 5:00" bajando | Pantalla |
| Avanza un porcentaje con "recibiendo…" | Pantalla |
| Se reinicia sola | Pantalla |
| Dice "instalado: v0.4" | MENÚ → Conexión → OK → Actualizar |
| La calibración y la red guardada siguen ahí | Pesa sin pedir calibrar y se reconecta sola |

Prueba también el seguro: **sin** abrir el permiso en la báscula, la página
debe rechazar el archivo.

### 2. Pestaña Báscula

Abre `http://tostabascula.local`.

| Haz esto | Debe pasar |
|---|---|
| Pon y quita peso | El número cambia en vivo, unas 5 veces por segundo |
| Deja el peso quieto | Cambia de "midiendo…" a "ESTABLE" |
| Toca **Tara** | La página y la pantalla quedan en cero y marcan la tara |
| Toca **Cero** con el plato vacío | Queda en cero |
| Cambia la unidad (g, kg, oz) | Cambia también en la pantalla de la báscula |
| Presiona teclas en la báscula | La página refleja el cambio |

### 3. Pestaña Piezas

1. Pon el contenedor vacío y tara.
2. Coloca una muestra (10 piezas iguales, por ejemplo) y dile cuántas son.
3. Agrega y quita piezas: el conteo debe seguirlas.
4. Guarda la pieza y ponle un nombre ("Tornillo M3×10").

Fuerza los tres avisos para ver que salen:

| Aviso | Cómo provocarlo |
|---|---|
| *Mejor con N piezas o más* | Muestra de menos de 5 g (con la celda de 1 kg) |
| *pieza ligera: no confiable* | Piezas de menos de 0.1 g cada una |
| *entre 12 y 13 piezas* | Agrega media pieza, o algo que pese la mitad |

### 4. Contador desde las teclas

**MENÚ → Contar piezas.**

- Sigue los tres pasos de la pantalla: contenedor, muestra y cantidad.
- En el conteo, **▲ / ▼** abre las opciones: *Guardar pieza*, *Otra muestra*,
  *Elegir pieza* y *Terminar*.
- En *Elegir pieza* debe aparecer la que guardaste desde la web, con su nombre.
- Apaga y enciende la báscula: las piezas guardadas deben seguir ahí.

### 5. Recalibrar desde la web

En la pestaña **Ajustes**.

| Caso | Debe pasar |
|---|---|
| Sin permiso | La página dice "permiso cerrado" y pide abrirlo en la báscula |
| Con **MENÚ → Conexión → OK → Calibrar web** | Dice "permiso abierto" con el tiempo restante y deja recalibrar |
| Después de recalibrar | Un peso conocido se lee correcto |

### 6. Precisión con el WiFi encendido

**MENÚ → Diagnóstico → ▲** enciende la prueba de WiFi. Con la celda conectada,
"lentas" y "atípicas" deben quedarse en 0 y el ruido no debe subir.

## Si algo falla

| Síntoma | Qué hacer |
|---|---|
| La página rechaza el `.bin` | Abre el permiso en la báscula; dura 5 minutos |
| El firmware nuevo no arranca | La placa regresa sola a la v0.3 |
| La pantalla se ve mal tras actualizar | El `.bin` era de otra placa: carga el correcto, por WiFi o por USB |
| `tostabascula.local` no abre | Usa la IP de MENÚ → Conexión |
| La página abre pero el peso no se mueve | Recarga la página; revisa que no haya más de 4 navegadores abiertos a la vez |

Para ver qué reporta la placa, conecta el USB y abre el monitor serie a
**115200**: al arrancar imprime la versión, la placa y la calibración.

## Decisiones pendientes de confirmar

Se aplicaron tal como se propusieron, sin respuesta de Jorge:

1. **Nombres de las piezas.** Desde la báscula se guardan como "Pieza 1",
   "Pieza 2"…; el nombre real se escribe desde la página web.
2. **Recalibrar desde la web pide permiso físico** en la báscula. Tara, cero,
   unidad y conteo quedan libres para cualquiera en la red.

## Lo que sigue

- Corregir lo que salga de estas pruebas.
- Hacer el commit de las fases 3 y 4.
- Fase 5: modo calorías y alimentos.

---

Desarrollado por Tostatronic - Ing. Jorge Alvarado

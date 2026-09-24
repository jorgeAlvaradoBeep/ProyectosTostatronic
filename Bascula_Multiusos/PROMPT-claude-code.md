# Prompt para Claude Code — Báscula Multiusos Tostatronic

> Pega todo este documento en Claude Code, o dile:
> *"Lee `Bascula_Multiusos/PROMPT-claude-code.md` y ejecútalo."*

---

Vamos a construir una **báscula multiusos** para Tostatronic. Es un proyecto
demostrativo que se va a publicar, así que el código, la interfaz y la
documentación tienen que estar a nivel de producto.

**Antes de escribir código, preséntame un plan por fases y espera mi
confirmación.** Es un proyecto grande: quiero revisar la arquitectura, el mapa
de pines y el diseño de la interfaz antes de que implementes.

---

## Atribución (obligatorio)

Todo lo que generes debe mostrar que fue **desarrollado por Tostatronic —
Ing. Jorge Alvarado**: encabezado del código, pantalla de arranque, pie de las
páginas web y el README. Atribución visible, no implícita.

---

## Ubicación

Crea el proyecto en `ProyectosTostatronic/Bascula_Multiusos/`.

El Arduino IDE exige que el `.ino` esté dentro de una carpeta con su mismo
nombre, así que la estructura es:

```
Bascula_Multiusos/
├── README.md
├── Bascula_Multiusos/
│   ├── Bascula_Multiusos.ino
│   ├── config.h          ← TODOS los pines y constantes, por placa
│   └── ...               ← los módulos que decidas
└── docs/
```

---

## Hardware

| Componente | Detalle |
|---|---|
| Microcontrolador | **ESP32-C6** (objetivo principal) y **ESP32-C5** (segundo objetivo). Tengo ambos en versión DevKit normal y versión mini. |
| Celda de carga | Galga extensométrica tipo barra |
| Amplificador | HX711 |
| Pantalla | **GC9A01**, redonda, 1.28", 240×240, SPI |
| Entrada | Teclado de membrana **1×4** (4 teclas + común) |

Quiero promocionar el C6 y el C5, así que la báscula tiene que aprovechar lo
que los distingue: WiFi 6 en el C6, y **WiFi 6 de doble banda (2.4 y 5 GHz)**
en el C5. La pantalla de conexión debe mostrar la banda y el estándar WiFi.

---

## Restricciones técnicas que DEBES respetar

### 1. Librería de pantalla — no uses TFT_eSPI

TFT_eSPI tiene problemas documentados de compilación en el ESP32-C6 con el core
3.x de Arduino (errores de `VSPI` no declarado, reinicios en bucle; hay varios
issues abiertos en su repositorio). **No la uses.**

Usa **Arduino_GFX_Library** (moononournation), que tiene driver nativo para
GC9A01. Si no compila para alguno de los dos chips, usa **LovyanGFX** como
respaldo.

Encapsula la pantalla detrás de **una capa propia delgada**, para que cambiar
de librería sea tocar un solo archivo. Verifica que compile para C6 **y** para
C5 antes de avanzar.

### 2. El HX711 en un chip de un solo núcleo

El C6 y el C5 son de **un solo núcleo**. El HX711 se lee por bit-banging, y las
interrupciones del WiFi pueden romper el timing a media lectura. El síntoma es
feo y engañoso: la báscula funciona perfecto sin WiFi y mete lecturas basura
en cuanto se conecta.

Tienes que:
- Leer el HX711 en una sección crítica (o equivalente) para que el WiFi no
  interrumpa a media trama.
- Aplicar **filtro de mediana + descarte de valores atípicos** sobre una
  ventana de lecturas, no solo promedio.
- **Probar explícitamente la lectura con el WiFi activo y transmitiendo**, no
  solo en reposo.
- Implementar detección de estabilidad: mostrar un indicador cuando el peso se
  asentó, y usar solo lecturas estables para tara, calibración y conteo.

### 3. Pines

Centraliza **todos** los pines en `config.h`, con una sección por placa (C6
DevKit, C6 mini, C5 DevKit, C5 mini) seleccionable por `#define`.

En el **ESP32-C6** evita:

| GPIO | Motivo |
|---|---|
| 4, 5, 8, 9, 15 | Strapping — definen el modo de arranque |
| 12, 13 | USB-JTAG — perderías la programación por USB |
| 24 – 30 | Flash SPI interno |
| 16, 17 | UART0 (monitor serie) |

Para el **ESP32-C5**, verifica los pines equivalentes en la documentación
oficial de Espressif **antes** de asignarlos. No asumas que coinciden con el C6.

Presupuesto de GPIO:
- GC9A01: SCK, MOSI, CS, DC, RST, BL → 6
- HX711: DOUT, SCK → 2
- Teclado 1×4: 4 teclas (el común a GND, entradas con `INPUT_PULLUP`) → 4

Son 12. **Las placas mini exponen pocos pines**, así que para ellas propón
ahorrar dos: el backlight (BL) directo a 3.3 V y el RST de la pantalla a EN o a
3.3 V. Documenta esa variante.

Propón el mapeo en el plan y espérame antes de fijarlo.

---

## Funciones

### A. Báscula

- Lectura de peso en tiempo real, en **gramos** (con opción de kg y onzas).
- **Tara**.
- **Calibración en el primer arranque** (asistente guiado en pantalla):
  1. Retira todo el peso → toma el cero.
  2. Coloca un peso conocido → ingresa su valor con las teclas.
  3. Calcula el factor y guarda **factor y offset en NVS** (`Preferences`).
- Recalibración disponible después desde el menú y desde la web.
- Si al arrancar no hay calibración guardada, entra directo al asistente.

### B. Conexión a internet — portal cautivo

En este mismo repositorio ya hay un portal cautivo funcionando en
`ESP32_WROVER_CAM/ESP32_WROVER_CAM_Portal/`. **Revísalo y reutiliza el mismo
enfoque**, adaptado a este proyecto:

- Sin claves de WiFi escritas en el código.
- Red abierta tipo `Tostatronic-Bascula` que abre el portal automáticamente.
- Lista de redes escaneadas, se elige una y se escribe la clave.
- Al conectar: **pantalla de conexión** en la báscula con la **IP**, la banda,
  el estándar WiFi y la intensidad de señal.
- Acceso también por mDNS (`tostabascula.local`).
- Opción para olvidar la red, desde el menú y desde la web.
- **La báscula funciona completa sin internet.** El WiFi es un extra, nunca un
  requisito para pesar, contar o calcular calorías.

### C. Contador de piezas por peso

Modo manual (teclado) y modo web.

Flujo:
1. Tara con el contenedor vacío.
2. El usuario coloca una muestra de N piezas.
3. La báscula pregunta: *"El peso actual es de X g. ¿Cuántas unidades son?"*
4. El usuario ingresa N (con las teclas o en la web).
5. Se calcula el **peso unitario = peso / N**.
6. A partir de ahí muestra el conteo: `round(peso_total / peso_unitario)`.

Salvaguardas que quiero:
- **Si el peso unitario está por debajo de la resolución real de la báscula**,
  avisa que el conteo no será confiable y sugiere una muestra más grande.
- Recomienda muestras más grandes para piezas ligeras.
- Muestra una advertencia cuando el conteo quede cerca de la mitad entre dos
  enteros (ambigüedad).
- Poder guardar perfiles de pieza con nombre (ej. "Tornillo M3×10") para no
  recalibrar cada vez.

### D. Calorías por peso

Modo manual (teclado, lista de alimentos) y modo web.

El usuario elige un alimento, lo coloca, y la báscula muestra las calorías
correspondientes al peso: *"200 g de carne de res molida cruda ≈ X kcal"*.
Idealmente también proteína, grasa y carbohidratos.

**Base de datos — reglas estrictas:**

- **No inventes valores nutricionales.** Cada entrada debe venir de una fuente
  real y citada: **USDA FoodData Central** o el **SMAE** (Sistema Mexicano de
  Alimentos Equivalentes). Registra la fuente en el archivo de datos.
- **Distingue crudo de cocido** y el corte o variante. "Carne de res" sola no
  sirve: varía muchísimo según el corte y la cocción. Usa entradas específicas
  como "Res, molida 80/20, cruda" o "Pechuga de pollo, cocida, sin piel".
- Incluye unos **50 alimentos comunes del mercado mexicano**: carnes, pollo,
  pescado, huevo, tortilla, arroz, frijol, frutas, verduras, lácteos, pan,
  aceites.
- Guarda la base en **LittleFS** como JSON, en kcal por 100 g.
- Desde la web: agregar, editar y borrar alimentos propios.
- **Búsqueda en línea opcional**: si el usuario captura en ajustes una API key
  gratuita de USDA FoodData Central, permite buscar alimentos en línea y
  guardarlos localmente. Sin la key, la base local funciona sola.

---

## Interfaz — pantalla redonda

La GC9A01 es **circular**: las esquinas del cuadro de 240×240 no existen.

- Todo texto debe caber **dentro del círculo inscrito**. Nada pegado a los
  bordes rectos.
- Diseño **radial y centrado**: el número de peso grande al centro, el modo en
  un arco superior, estado (WiFi, estabilidad, tara) en un arco inferior.
- Usa un anillo o arco perimetral como indicador visual (ej. progreso de
  estabilidad, o porcentaje de capacidad).
- Pantalla de arranque con el logo y la atribución de Tostatronic.
- Paleta coherente con la marca: fondo oscuro, azul de marca como acento.

## Interfaz — teclado 1×4

Propón el esquema completo en el plan. Mi punto de partida:

| Tecla | Pulsación corta | Pulsación larga |
|---|---|---|
| **1 · MENÚ** | Abrir menú / atrás | Volver a la pantalla principal |
| **2 · ▲** | Subir / aumentar | Aumento rápido |
| **3 · ▼** | Bajar / disminuir | Disminución rápida |
| **4 · OK** | Confirmar · **TARA** en la pantalla de peso | Cero |

Con antirrebote. Menú principal: Pesar · Contar piezas · Calorías · Conexión ·
Calibrar · Ajustes.

## Interfaz — web

Servida desde la propia ESP32, sin depender de CDNs externos (tiene que
funcionar en una red sin internet).

- **Peso en vivo** vía WebSocket o Server-Sent Events.
- Secciones: Báscula · Contador de piezas · Calorías · Alimentos · Ajustes.
- Responsiva: la mayoría la va a abrir desde el teléfono.
- Pie de página con la atribución de Tostatronic.

---

## Plan por fases que espero

1. **Arranque del hardware:** pantalla, HX711 y teclado funcionando, con el
   mapa de pines confirmado. Peso crudo en pantalla.
2. **Calibración, tara y filtrado**, guardado en NVS. Validación con el WiFi
   activo.
3. **Portal cautivo y pantalla de conexión.**
4. **Servidor web** con peso en vivo y el **contador de piezas**.
5. **Modo calorías** con la base de datos.
6. **README**, diagramas de conexión y pulido.

Al terminar cada fase, dime qué quedó, qué probaste y qué falta, antes de
pasar a la siguiente.

---

## README

Al estilo de los otros proyectos del repositorio (`Recorrido_LEDs_595`,
`ESP32_WROVER_CAM`):

- Qué es y para qué sirve.
- Diferencia entre C6 y C5, y cuál conviene para qué.
- Tabla de conexiones por placa, incluida la variante mini con el ahorro de
  pines.
- Advertencia sobre los pines prohibidos.
- Cómo calibrar, cómo contar piezas, cómo usar el modo calorías.
- Las fuentes de los datos nutricionales.
- Lista de materiales.
- Atribución de Tostatronic.

---

**Desarrollado por Tostatronic** — Ing. Jorge Alvarado

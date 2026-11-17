# Delia

<img width="1228" height="805" alt="Captura de pantalla_2026-11-17_16-37-22" src="https://github.com/user-attachments/assets/c4ac4954-27db-4af1-bcf7-1713cdf2d7a7" />


Taller de cinta en un plugin VST3: hace música con el procedimiento de trabajo de Delia Derbyshire en el BBC Radiophonic Workshop, usando ficheros de audio en lugar de cinta.

Versión 0.2.0 · Eloy Platas · licencia GPLv3

Manual completo en PDF: [castellano](docs/Delia_Manual_ES.pdf) · [English](docs/Delia_Manual_EN.pdf)

## Qué hace

1. **Makeup.** Cargas un sonido, recortas la región útil (por ejemplo, la resonancia sin el golpe) y eliges raíz y escala. Cada nota de la escala es el mismo sonido leído a otra velocidad: más agudo dura menos, más grave dura más.
2. **Empalme.** Pegas notas una tras otra como trozos de cinta medidos en centímetros. El ritmo es longitud.
3. **Máquinas.** Cuatro magnetófonos reproducen cada uno su tira en bucle, con velocidad variable, deriva e inercia de motor.
4. **Volcado.** Grabas la mezcla en un carrete nuevo. Cada volcado es una generación más y pasa por el modelo de cinta: saturación, pérdida de agudos, soplido, wow.

Además: laboratorio (oscilador de prueba, ruido blanco, wobbulator), tratamientos (eco de cinta de tres cabezales, placa, filtros pasivos, modulador en anillo) y un cajón con doce osciladores y ocho puertas para síntesis aditiva y resíntesis de parciales.

## Descarga e instalación

Las versiones compiladas están en la página [Releases](https://github.com/eloyplatas/DELIA/releases): un zip por sistema con `Delia.vst3`, este README, la licencia y los manuales.

| Sistema | Dónde copiar `Delia.vst3` |
| --- | --- |
| Linux x86_64 (glibc 2.35 o posterior) | `~/.vst3/` |
| Windows 64 bits | `C:\Program Files\Common Files\VST3\` |
| macOS (Intel y Apple Silicon) | `~/Library/Audio/Plug-Ins/VST3/` |

En macOS el plugin está firmado ad hoc pero no notarizado; si el sistema lo bloquea: `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Delia.vst3`.

## Compilar

```
sudo apt install build-essential cmake ninja-build libasound2-dev libx11-dev libxrandr-dev \
     libxinerama-dev libxcursor-dev libxext-dev libfreetype6-dev libfontconfig1-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

CMake descarga JUCE 8.0.10 si no hay una carpeta `JUCE/` junto al proyecto. El resultado queda en `build/Delia_artefacts/Release/`.

- `-DDELIA_BUILD_TESTS=ON` compila `DeliaTest`, la prueba del motor sin interfaz.
- `-DDELIA_BUILD_SHOT=ON` compila `DeliaShot`, que guarda capturas de la interfaz y comprueba volcado y estado.
- `.github/workflows/build.yml` compila los VST3 de Linux, Windows y macOS con GitHub Actions; al subir una etiqueta `v*` publica una versión con los zips y los manuales.
- `docs/manual/build_manual.py` genera los manuales en PDF (necesita ReportLab y las fuentes DejaVu).

## Uso paso a paso

### Carretes y Makeup

- **CARGAR...** o arrastrar ficheros a la ventana (WAV, AIFF, FLAC, OGG). Los carretes son mono; con **ESTÉREO** encendido, los que cargues o vuelques a partir de ese momento conservan los dos canales.
- **Punto de inicio de lectura:** arrastra el borde izquierdo de la forma de onda, escribe el tiempo en **INICIO s** o haz doble clic donde quieras empezar. **FIN s** fija el final. Los dos puntos de arriba son los fundidos de entrada y de salida: un fundido de entrada largo quita el ataque.
- **Preescucha:** **▶ ESCUCHAR** reproduce la región desde el inicio, a velocidad original y con los fundidos (así suena la nota 0). Un clic en la regla inferior de la forma de onda escucha el fichero desde ese punto; arrastrar recorre el sonido.
- **RAÍZ Hz** es la altura del sonido original. **DETECTAR** la calcula; en sonidos inarmónicos escríbela a mano.
- **ESCALA**: temperada, pitagórica, mesotónica de cuarto de coma o libre. La libre admite razones (`1 9/8 5/4 3/2 5/3`) o un fichero Scala (`.SCL...`).
- **DESDE / HASTA** fijan cuántas notas por debajo y por encima de la raíz aparecen en el teclado.
- Cada carrete muestra su generación (G0 = fichero original). Se puede arrastrar un carrete desde la lista a REAPER.

### Empalme

- Elige la máquina (1 a 4) arriba a la derecha o pulsando su platina.
- **Clic en una nota** del teclado: se escucha y se pega un trozo con la longitud de la figura activa. **Clic derecho**: solo se escucha.
- **FIGURA**: redonda a fusa, con puntillo y tresillo. El panel muestra cuántos cm mide a la velocidad de cinta y el tempo actuales. A 38 cm/s y 120 pulsos por minuto, una negra mide 19 cm.
- **Trozo seleccionado**: nota, largo en cm, inicio en cm (desde qué punto del sonido empieza), corte (recto, 45° o 60°), invertir, mover, duplicar, cortar.
- Arrastra el borde derecho de un trozo para cambiar su largo. Se ajusta a un cuarto de la figura; con Mayúsculas, libre.
- **+ VIRGEN** añade cinta sin grabar (silencio).
- **PASOS POR MIDI**: cada nota del canal 1 pega un trozo (do central = nota 0).
- **ERROR DE CORTE mm** desvía cada largo unos milímetros, siempre igual para el mismo proyecto.
- **EXPORTAR TIRA** escribe la tira como WAV en la carpeta de carretes.

### Máquinas

- Teclas: ◀ marcha atrás, ▶ marcha/paro, ∞ bucle (apagado: una sola pasada).
- **VARIADOR**: 0,25x a 4x. Cambia altura y tempo de toda la tira.
- **NIVEL, PAN, GRAVES** (paso alto), **AGUDOS** (paso bajo), **ECO** y **PLACA** (envíos). **12 dB/oct** dobla la pendiente de los filtros.
- **SINCRO**: Libre, Arranque (empieza con el transporte) o Enganchado (sigue la posición del proyecto, sin deriva).
- **ANILLO**: modula la máquina con el oscilador de prueba o con la máquina vecina.
- En la cabecera: **CINTA cm/s** (9,5, 19 o 38), **DESGASTE** (cuánto degrada cada pasada por cinta; 0 = copia exacta), **DERIVA** (las máquinas se separan con el tiempo), **INERCIA** (arranque y parada del motor) y **SALIDA**.
- **PASILLO** extiende las cuatro tiras a la misma escala para ver qué trozo desajusta.

### Volcado

- **VOLCAR** graba la mezcla en tiempo real, con los movimientos que hagas. Pulsa otra vez para parar.
- **VOLCAR FUERA DE LÍNEA** renderiza al instante los cm indicados. **= BUCLE** pone la longitud de la tira más larga.
- El carrete nuevo aparece en la estantería con una generación más y se escribe como WAV de 24 bits en la carpeta de carretes (**CARPETA...**; por defecto `Documentos/Delia/Carretes`).

### Laboratorio y tratamientos

- Oscilador de prueba (senoidal o cuadrada), ruido blanco y wobbulator (oscilador con barrido periódico). **CAPTURAR LAB.** los graba a un carrete.
- Eco: velocidad de cinta, realimentación (hasta 110 %), separación de cabezales y tres cabezales conmutables.
- Placa: caída de 1 a 5 s, amortiguación y predelay.

### Doce osciladores

El interruptor **12 OSCILADORES** de la cabecera despliega el cajón.

- **ENCENDIDO** activa el banco. Cada oscilador tiene frecuencia en Hz (0 = apagado), nivel, onda cuadrada opcional y puerta asignada (— = siempre abierto).
- **Puertas P1 a P8**: mantén pulsado para abrir; Mayúsculas + clic la deja abierta. Cada una tiene ataque y caída.
- **ANALIZAR CARRETE ACTIVO** busca los doce parciales más fuertes de la región de Makeup y los reparte en los osciladores.
- **TRANSPOSICIÓN** conserva las relaciones entre parciales. **DERIVA VÁLV.** desafina cada oscilador lentamente.
- **CAPTURAR** graba el banco a un carrete.

## MIDI

| Canal | Notas | Función |
| --- | --- | --- |
| 1 | todas | Escuchar las notas del carrete activo; entrada por pasos en Empalme (60 = nota 0) |
| 2 | 60 a 67 | Abrir las puertas 1 a 8 del banco |
| 3 | 60 a 63 | Marcha/paro de las máquinas 1 a 4 |

## Salidas

Estéreo principal y cuatro salidas mono opcionales, una por máquina.

## Estado y ficheros

El proyecto guarda tiras, escalas, ajustes y la lista de carretes con su ruta. Los ficheros de audio no se incrustan: si falta uno, el carrete aparece como «FALTA EL FICHERO» y se reubica pulsando sobre él.

## Límites conocidos de esta versión

- La interfaz tiene tamaño fijo (1240 × 820, más el cajón).
- Las rutas de los carretes son absolutas.
- El volcado en tiempo real admite hasta 3 minutos por toma.
- Las versiones de Windows y macOS se compilan en GitHub Actions y no se han probado en un anfitrión real.
- La interfaz está solo en castellano.

## Licencia y créditos

Delia se distribuye bajo GPLv3 (ver `LICENSE`).

- La histéresis magnética del modelo de cinta (`Source/Tape.h`) adapta el modelo de **ChowTape / AnalogTapeModel** de Jatin Chowdhury (GPLv3), descrito en «Real-time Physical Modelling for Analog Tape Machines», DAFx 2019.
- La reverberación de placa sigue la topología publicada por Jon Dattorro («Effect Design, Part 1», JAES 1997).
- Construido con JUCE 8, usado bajo su licencia AGPLv3.

Delia es un homenaje al método de trabajo de Delia Derbyshire. No está afiliado a su legado ni a la BBC.

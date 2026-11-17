# Delia user manual, English content
L = {
"meta_title": "Delia: user manual",
"meta_subject": "VST3 tape workshop inspired by the methods of Delia Derbyshire",
"cover_sub": "TAPE WORKSHOP  \u00b7  USER MANUAL",
"cover_ver": "Version 0.2.0  \u00b7  VST3 for Linux, Windows and macOS",
"cover_line1": "Music made the way it was made at the BBC Radiophonic Workshop in the sixties:",
"cover_line2": "sounds tuned by speed, spliced by length and bounced from tape to tape.",
"header": "User manual \u00b7 version 0.2.0",
"toc": "Contents",
"content": [

("h1", "1. What Delia is"),
("p", "Delia is a VST3 virtual instrument that recreates the <b>working method</b> of Delia Derbyshire and the BBC Radiophonic "
      "Workshop in the 1960s, using audio files instead of magnetic tape. It is not a playable sampler or a tape effect: it is a "
      "workshop. You make notes from a sound by changing its speed, splice them one after another as pieces of tape measured in "
      "centimetres, play them on four tape machines, and bounce the mix to a new reel that loses quality with every generation."),
("p", "The point is to let the method impose its constraints, because much of that sound came from them: pitch and duration "
      "are always linked, a note is an object with a length rather than a MIDI event, mixing is destructive, and the natural way "
      "of working is to squeeze a few sounds rather than load many. Delia does not try to copy any particular machine and contains "
      "no recordings by Derbyshire; it offers the tools and rules of the workshop so that the result is yours."),
("img", "delia-principal.png", 1.0, "Main view: four machines at the top; reels and Makeup, Splice, and Laboratory below. The interface labels are in Spanish."),
("h2", "Features at a glance"),
("ul", [
    "<b>Reels and Makeup:</b> load WAV, AIFF, FLAC or OGG; choose the reading start and end points, fades, root and scale; preview.",
    "<b>Splice:</b> a sequencer in which every note is a piece of tape with a length in cm, a straight or diagonal cut, reversal and cutting error.",
    "<b>Four machines:</b> loops of independent length, varispeed from 0.25x to 4x, drift, motor inertia, passive filters, ring modulator, host sync.",
    "<b>Bounce:</b> records the mix to a new reel, one generation further on, with tape loss (saturation, treble loss, hiss, wow and flutter).",
    "<b>Laboratory:</b> test oscillator, white noise and wobbulator, all capturable to a reel.",
    "<b>Treatments:</b> three-head tape echo and plate reverb.",
    "<b>Twelve oscillators:</b> a drop-down drawer with twelve oscillators, eight gates and partial analysis for additive resynthesis.",
]),
("note", "The interface is labelled in Spanish. Each control in this manual is given with its on-screen label in capitals followed by its meaning, "
         "for example VOLCAR (bounce), EMPALME (splice), CARRETES (reels)."),

("h1", "2. History and purpose of the project"),
("p", "Delia was born in November 2026 from a question: how would you make music today with Delia Derbyshire's techniques if you used "
      "audio files instead of tape? Her procedures were researched first, as documented in interviews, in the archive at the University of "
      "Manchester and in studies of the Radiophonic Workshop. A specification followed that put production techniques ahead of imitating "
      "the sound, and the plug-in was named Delia."),
("p", "The main design decisions were these:"),
("ul", [
    "Four machines. The Doctor Who theme used three synchronised tape recorders plus a fourth machine for the bounce; Delia offers four players with the bounce on top.",
    "Mono reels, like the workshop's quarter-inch tape, with an option to keep stereo.",
    "Saturation from ChowTape's physical model of magnetic hysteresis, the most rigorous open tape model available.",
    "The twelve oscillators inside the plug-in, in a drawer opened with a switch.",
    "No sound library: the plug-in starts empty, like the workshop.",
    "A code-drawn interface evoking tape recorders of the fifties and sixties: hammertone enamel, cream panels, bakelite knobs, piano keys and needle meters.",
]),
("h2", "Versions"),
("table", [["Version", "Date", "Changes"],
           ["0.1.0", "2026-11-16", "First complete version: reels, Makeup, Splice, four machines, tape model, laboratory, treatments, bounce, twelve oscillators, corridor."],
           ["0.2.0", "2026-11-17", "Reel reading start and end points in seconds, region preview, listening ruler to play from any point, double click to set the start. Builds for Linux, Windows and macOS. PDF manuals."]],
 [0.12, 0.14, 0.74]),

("h1", "3. Delia Derbyshire (1937-2001)"),
("p", "Delia Ann Derbyshire was born on 5 May 1937 in Coventry. She spent part of the war evacuated to Preston and said that the sound "
      "of air-raid sirens sparked her interest in electronic sound. She went to Barr's Hill grammar school and won a scholarship to read "
      "mathematics at Girton College, Cambridge; after a year she switched to music and graduated in 1959 in mathematics and music. That "
      "double training shaped all her work."),
("p", "In 1959 she approached Decca and was told that the company did not employ women in its recording studios. She worked in Geneva "
      "for the United Nations, taught in Coventry and spent time at the music publisher Boosey &amp; Hawkes before joining the BBC in "
      "November 1960 as a trainee assistant studio manager. In April 1962 she was assigned to the Radiophonic Workshop, the first "
      "institutional electronic music studio in Britain."),
("p", "In 1963 she realised the electronic version of Ron Grainer's <i>Doctor Who</i> theme, built, in her words, inch by inch on "
      "quarter-inch mono tape, with no musicians and no synthesisers. It was the show's theme from 1963 to 1980. Over a decade she made "
      "more than two hundred pieces for radio and television, many uncredited, because the BBC attributed work to the Workshop rather than "
      "to its members. They include the <i>Inventions for Radio</i> with Barry Bermange, <i>Blue Veils and Golden Sands</i> (1967), "
      "<i>The Delian Mode</i> (1968), <i>Pot au Feu</i> (1968) and <i>Dance from Noah</i> (1971)."),
("p", "Outside the BBC she founded Unit Delta Plus in 1966 with Brian Hodgson and Peter Zinovieff, and with Hodgson and David Vorhaus "
      "the Kaleidophon studio, where as White Noise they recorded the album <i>An Electric Storm</i> (Island, 1969). In 1973 she left the "
      "BBC, disillusioned by the limits on her creativity and the arrival of mass-produced synthesisers; with Hodgson she made the "
      "soundtrack for <i>The Legend of Hell House</i> at the Electrophon studio. She then left music for years: she worked as a radio "
      "operator on a gas pipeline project, in a bookshop and in an art gallery, and from 1980 lived in Northampton with her partner, "
      "Clive Blackburn."),
("p", "In the late nineties interest in her work grew and she began working again, among others with Peter Kember (Sonic Boom). She "
      "died of renal failure on 3 July 2001 in Northampton, aged 64. In 2007 her personal archive of tapes and papers went to the "
      "University of Manchester, where it can be consulted at the John Rylands Library, and every 23 November, the anniversary of the "
      "first broadcast of <i>Doctor Who</i>, is Delia Derbyshire Day."),
("note", "Delia is a tribute to her working method. It is not affiliated with her estate, the BBC or the rights holders of her works, and contains none of her recordings."),

("h1", "4. The workshop's techniques and how Delia implements them"),
("p", "Work at the Radiophonic Workshop was a craft: tape recorders, valve oscillators, filters, a razor blade and a lot of arithmetic. "
      "Each technique in the table has its counterpart in Delia; chapter 7 explains every control."),
("table", [["Tape technique", "How it was done", "How Delia does it"],
    ["Varispeed", "A recording was copied at different speeds: slower, lower and longer; faster, higher and shorter. That gave the low and high sounds of a rhythm.",
     "Makeup: each scale degree is the same sound read at another speed (ratio = target frequency / root). No time-stretch: pitch and duration stay linked. Reading uses windowed-sinc interpolation with an anti-aliasing filter when speeding up."],
    ["One note, one piece of tape", "The Doctor Who bass was a plucked string recorded at several pitches and spliced note by note with razor blade, editing block, chinagraph pencil and sticky tape. A note's duration was its length.",
     "EMPALME (splice): each piece has a note, a length in cm, a start point and a cut type. The ruler shows how many cm each note value measures at the current tape speed and tempo."],
    ["Straight or diagonal cut", "A straight cut clicks; a diagonal cut on the block blends one piece into the next over a few millimetres.",
     "Straight cut (direct jump), 45° or 60° (equal-power crossfade whose length comes from the tape width, 6.35 mm, and the speed)."],
    ["Manual imprecision", "Splices were never exact; on Doctor Who one slightly long splice put the tapes out of sync.",
     "ERROR DE CORTE (cutting error): offsets each length by a few millimetres, always the same for a given project. PASILLO (corridor) view to spot the offending piece by eye, as when the tapes were laid out along the corridor."],
    ["Synchronised machines", "Three Philips EL3503 machines modified to run together carried the submixes.",
     "Four machines with loops of independent length, slow independent speed drift, motor inertia and three host-sync modes."],
    ["Loops", "Tape loops up to fifteen metres long, sometimes running out of the studio.",
     "Each strip plays as a loop; strips of different lengths drift against each other and make long cycles."],
    ["Bouncing and generations", "Submixes were re-recorded onto another machine, with level changes and effects made during the bounce. Every copy added hiss and lost treble.",
     "VOLCAR (bounce) records the mix in real time (or offline) to a new reel, with a generation counter and one pass through the tape model. The reel goes back into Makeup."],
    ["Makeup libraries", "Derbyshire built up a stock of sounds and then assembled: ten sounds feed several pieces in the Pot au Feu family.",
     "The reel shelf: any reel, loaded, captured or bounced, can be the source of any strip."],
    ["Removing the attack", "With the green lampshade she recorded the strike and faded in on the ringing part, without the attack.",
     "Reading start point (INICIO) and fade-in in Makeup, with preview to hear exactly what will play."],
    ["Twelve oscillators and eight gates", "Twelve Jason valve oscillators and eight gating circuits built in-house; the keying unit only switched the generators on and off.",
     "DOCE OSCILADORES drawer: frequency in Hz, level, waveform, valve drift and assignment to eight gates with attack and decay."],
    ["Analysis and resynthesis", "For Blue Veils and Golden Sands she analysed the lampshade sound into its partials and rebuilt the twelve strongest on the twelve oscillators.",
     "ANALIZAR CARRETE ACTIVO (analyse active reel): averaged FFT, parabolic peak interpolation, and the twelve strongest partials spread across the oscillators."],
    ["Calculated tuning", "She worked out tunings with log tables, in Pythagorean and meantone scales.",
     "Equal-tempered, Pythagorean, quarter-comma meantone and free scales (ratios or a Scala .scl file)."],
    ["Wobbulator and filtered noise", "The wobbulator, a frequency-sweep generator, made the low bubbling drones; the clouds were filtered white noise.",
     "Laboratory: wobbulator with centre, width, rate and shape; white noise; test oscillator; passive filters. All capturable to a reel."],
    ["Echo and plate", "Echo from a tape loop running past several heads; EMT 140 plate reverb; passive EQ.",
     "Tape echo with three heads, spacing and speed, passing through the tape model on every repeat; Dattorro-topology plate; passive filters on each machine."],
 ], [0.17, 0.39, 0.44]),

("h1", "5. Installation"),
("table", [["System", "File", "Where to copy it"],
    ["Linux (x86_64)", "Delia.vst3", "~/.vst3/ (user) or /usr/lib/vst3/ (system)"],
    ["Windows (64-bit)", "Delia.vst3", "C:\\Program Files\\Common Files\\VST3\\"],
    ["macOS (Intel and Apple Silicon)", "Delia.vst3", "~/Library/Audio/Plug-Ins/VST3/ or /Library/Audio/Plug-Ins/VST3/"]],
 [0.24, 0.18, 0.58]),
("p", "After copying, rescan plug-ins in your host. In REAPER: Options, Preferences, Plug-ins, VST, Re-scan."),
("note", "<b>macOS:</b> the plug-in is ad-hoc signed but not notarised. If macOS blocks it, open Terminal and run "
         "<font face='Mono'>xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Delia.vst3</font>. "
         "<b>Linux:</b> the published build is compiled on Ubuntu 22.04 and runs on distributions with glibc 2.35 or later."),
("p", "To build from source, follow the README in the github.com/eloyplatas/DELIA repository."),

("h1", "6. Getting started"),
("ol", [
    "Insert Delia on a REAPER track as an instrument. The main output is stereo.",
    "Press <b>CARGAR...</b> (load) in the CARRETES (reels) panel, or drag an audio file onto the window. A short, resonant sound (a plucked string, a struck metal object) is a good start.",
    "On the waveform, drag the left edge to where you want reading to start, or type the time in <b>INICIO s</b> (start, seconds). Press <b>ESCUCHAR</b> (listen) to hear the region as note 0 will sound.",
    "Check the <b>RAÍZ Hz</b> (root; DETECTAR detects it) and choose the <b>ESCALA</b> (scale).",
    "In EMPALME (splice), choose machine 1, a note value (for example 1/8) and click notes on the keyboard: each click splices a piece.",
    "Press the red ▶ key on machine 1. The strip plays as a loop.",
    "Build another strip on machine 2, start it, and set levels, filters and sends.",
    "Press <b>VOLCAR</b> (bounce) to record the mix to a new reel. That reel is generation 1 and can be cut up again in Makeup.",
]),

("h1", "7. Interface reference"),
("h2", "7.1 Header"),
("table", [["Control", "Function"],
    ["CINTA cm/s (tape speed)", "Global tape speed: 9.5, 19 or 38 cm/s. Sets the conversion between length and time on every strip, and the frequency response of the tape model (less treble at lower speed). Changing it remounts the strips."],
    ["DESGASTE (wear)", "How much each pass through tape degrades the sound (bounce and echo), 0 to 100 %. At 0 the copy is exact."],
    ["DERIVA (drift)", "Slow, independent deviation of each machine's speed, 0 to 0.5 %. With drift, machines move apart over time like real motors."],
    ["INERCIA (inertia)", "Motor start and stop time, 0 to 2 s. Heard as a glissando."],
    ["SALIDA (output)", "Main output level."],
    ["IZQ / DCHA meters", "Main output level, left and right."],
    ["PASILLO (corridor)", "Replaces the Reels and Splice panels with a view of all four strips at the same scale."],
    ["12 OSCILADORES", "Opens the twelve-oscillator drawer below the window."],
    ["Status line", "Messages: reels loaded, bounces, missing files, analysis results."]],
 [0.24, 0.76]),

("h2", "7.2 Machines"),
("img", "delia-maquina.png", 0.52, "One machine: transport with reels, cm counter and heads; the control panel below."),
("p", "Each machine plays the strip mounted on it in EMPALME. The reels turn according to speed and the amount of tape on each side; the "
      "counter shows the position in cm and the lamp lights while the machine runs. Clicking a machine makes it the active machine in the "
      "EMPALME panel."),
("table", [["Control", "Function"],
    ["◀", "Reverse: the strip is read backwards."],
    ["▶ (red key)", "Run and stop, with the header's inertia."],
    ["∞", "Loop. Off, the strip plays once. The loop is part of the assembly: the join between the last piece and the first gets its crossfade."],
    ["VARIADOR (varispeed)", "Speed from 0.25x to 4x. Changes pitch and tempo of the whole strip together. Double click: 1x."],
    ["VU meter", "Machine level."],
    ["12 dB/oct", "Doubles the slope of the GRAVES and AGUDOS filters (6 or 12 dB per octave)."],
    ["NIVEL, PAN", "Level (−60 to +6 dB) and pan in the mix."],
    ["GRAVES (bass cut)", "Passive high-pass filter in fixed steps: off, 40, 80, 160, 320, 640, 1250 or 2500 Hz."],
    ["AGUDOS (treble cut)", "Passive low-pass filter in fixed steps: off, 12, 8, 5.6, 4, 2.8, 2, 1.4 or 1 kHz."],
    ["ECO, PLACA", "Sends to the tape echo and the plate."],
    ["SINCRO (sync)", "Libre (free): runs at its own speed, ignoring the transport. Arranque (start): rewinds when the host transport starts. Enganchado (locked): position follows the project position and drift is disabled."],
    ["ANILLO (ring)", "Ring modulator: No, with the laboratory's test oscillator, or with the neighbouring machine."]],
 [0.2, 0.8]),

("h2", "7.3 Reels and Makeup"),
("img", "delia-carretes.png", 0.5, "Reels and Makeup panel, with preview running."),
("p", "The shelf holds every reel in the session: loaded, captured and bounced. Each row shows the generation (G0 is an original file), "
      "whether it is mono (M) or stereo (ST) and its duration. Clicking a row makes that reel active; if its file is missing, a dialog "
      "opens to relocate it. A row can be dragged into REAPER to use the WAV."),
("table", [["Control", "Function"],
    ["CARGAR... (load)", "Loads one or more WAV, AIFF, FLAC, OGG or MP3 files. Files can also be dropped on the window."],
    ["QUITAR (remove)", "Removes the active reel from the shelf (does not delete the file)."],
    ["CARPETA... (folder)", "Folder where bounced and captured reels and exported strips are written. Default: Documents/Delia/Carretes."],
    ["ESTÉREO", "Off: new reels are summed to mono. On: reels loaded or bounced from then on keep both channels."],
    ["▶ ESCUCHAR / ■ PARAR", "Preview of the region, from the start point to the end point, at original speed and with the fades: exactly how note 0 sounds. A red line runs across the waveform."],
    ["Waveform", "Drag the left edge (reading start) or the right edge (end). The two dots at the top are the fade-in and fade-out. Double click: put the start there and preview it."],
    ["Listening ruler", "Bottom strip of the waveform, marked in seconds. Click to hear the file from that point to the end, without fades; drag to scrub. Triangles mark the start and end."],
    ["INICIO s / FIN s", "Reading start and end points in seconds, for precise placement. Notes start reading at INICIO."],
    ["FUND. ENT. ms / FUND. SAL. ms", "Fade-in and fade-out of every note. A long fade-in removes the attack."],
    ["RAÍZ Hz / DETECTAR", "Pitch of the original sound (note 0). DETECTAR detects it; for inharmonic sounds, type the reference partial by hand."],
    [".SCL...", "Loads a Scala scale and sets it as the free scale."],
    ["DESDE / HASTA (from / to)", "Notes below and above the root shown on the EMPALME keyboard."],
    ["ESCALA (scale)", "Temperada (12-tone equal), Pitagórica, Mesotónica (quarter-comma meantone) or Libre (free). Libre shows a field for ratios: 1 9/8 5/4 3/2 5/3."],
    ["Bottom line", "Frequency of note 0, region length in seconds and in cm at the current tape speed, and generation."]],
 [0.27, 0.73]),
("note", "<b>Pitch and duration are linked.</b> A note an octave above the root is read at twice the speed and lasts half as long. "
         "If the piece is longer than the sound, the rest is blank tape with its hiss."),

("h2", "7.4 Splice (EMPALME)"),
("img", "delia-empalme.png", 0.8, "Splice panel: the strip on the block, the cm ruler and the active reel's note keyboard."),
("p", "The active machine's strip sits on a splicing block, with a ruler in centimetres and red marks at each note value. Each piece shows "
      "its note; reversed pieces are hatched and blank tape is white. A red line shows the play position. Buttons 1 to 4 at the top switch machine."),
("table", [["Control", "Function"],
    ["Note keyboard", "Click: hear the note and splice a piece of the current note value after the selected one. Right click: listen only. Hovering shows the frequency and how many cm of sound that note has."],
    ["Strip", "Click: select a piece. Drag the right edge: change its length (snaps to a quarter of the note value; with Shift, free). Wheel: scroll."],
    ["FIGURA (note value)", "Whole, half, quarter, eighth, sixteenth and thirty-second, with dot (.) and triplet (3). The panel shows the length in cm."],
    ["TEMPO / DEL ANFITRIÓN", "Tempo used for note values: the project's (from host) or one typed by hand."],
    ["NOTA (note)", "Scale degree of the selected piece (0 = root)."],
    ["LARGO cm (length)", "Length of the piece."],
    ["INICIO cm (start)", "Where in the sound the piece starts reading, in cm of tape from the reel's reading start."],
    ["Cut", "Straight, 45° or 60°. The cut applies to the join with the previous piece."],
    ["INVERTIR (reverse)", "Splices the piece backwards."],
    ["◀ ▶", "Moves the piece towards the start or the end."],
    ["DUPLICAR, CORTAR", "Duplicates the piece or removes it from the strip."],
    ["+ VIRGEN (blank)", "Adds a piece of blank tape (silence with hiss) one note value long."],
    ["VACIAR (clear)", "Removes every piece from the strip."],
    ["PASOS POR MIDI (MIDI step input)", "Each note on MIDI channel 1 splices a piece (middle C, note 60, is degree 0)."],
    ["ERROR DE CORTE mm (cutting error)", "Offsets each length by up to this many millimetres, fixed for the project."],
    ["EXPORTAR TIRA (export strip)", "Writes the mounted strip as a WAV to the reels folder."]],
 [0.24, 0.76]),
("p", "At 38 cm/s and 120 beats per minute, a quarter note is 19 cm. At 19 cm/s the same quarter note is 9.5 cm. There are no bars to "
      "respect: a strip of 11 or 13 eighth notes, like the loops in <i>Pot au Feu</i>, is built the same way as one of 8."),

("h2", "7.5 Laboratory, treatments and bounce"),
("img", "delia-laboratorio.png", 0.5, "Laboratory (LABORATORIO), treatments (TRATAMIENTOS) and bounce (VOLCADO)."),
("table", [["Control", "Function"],
    ["OSC Hz, OSC NIV., CUADRADA", "Test oscillator: frequency from 20 Hz to 20 kHz, level, sine or square wave. It is also the ring modulator's carrier."],
    ["RUIDO (noise)", "White noise level."],
    ["GRAVES, AGUDOS", "Laboratory passive filters."],
    ["A ECO, A PLACA", "Laboratory sends to the treatments."],
    ["WOB Hz, ANCHO, VELOC., WOB NIV., FORMA", "Wobbulator: an oscillator whose frequency sweeps periodically around a centre. Width in Hz, rate from 0.1 to 50 Hz, shape sine, triangle or saw."],
    ["CAPTURAR LAB.", "Records the laboratory to a reel. Press again to stop."],
    ["ECO cm/s, REALIM., SEPAR., RET. ECO", "Tape echo: loop speed, feedback (up to 110 %), head spacing in cm and return level."],
    ["CABEZAL 1, 2, 3 (heads)", "Active playback heads. Each head is one spacing further than the previous one."],
    ["CAÍDA, AMORT., PREDEL., RET. PL.", "Plate: decay from 1 to 5 s, treble damping, predelay and return level."],
    ["VOLCAR (bounce)", "Records the mix in real time, including your moves, up to 3 minutes. Press again to stop."],
    ["cm / = BUCLE", "Length of the offline bounce. = BUCLE sets the length of the longest strip."],
    ["VOLCAR FUERA DE LÍNEA", "Offline bounce: renders that length instantly, with the machines running from the start."]],
 [0.32, 0.68]),
("p", "Each bounced reel takes the generation of its oldest source plus one, goes once through the tape model (the loss is recorded into the "
      "audio) and is written as a 24-bit WAV. A bounced reel does not remember its pieces: bouncing is destructive, as in the workshop."),

("h2", "7.6 Twelve oscillators"),
("img", "delia-osciladores.png", 1.0, "Full window with the twelve-oscillator drawer open."),
("table", [["Control", "Function"],
    ["Frequency (1 to 12)", "Frequency of each oscillator in Hz. 0 switches it off."],
    ["Slider", "Level of each oscillator."],
    ["CUAD.", "Square wave instead of sine."],
    ["Gate (— or P1 to P8)", "Gate that opens the oscillator. — = always open."],
    ["P1 to P8", "Hold to open the gate; Shift + click latches it open. The amber lamp shows it is open. Also via MIDI channel 2."],
    ["ATAQUE, CAÍDA", "Attack and decay of each gate."],
    ["ENCENDIDO (power)", "Switches the bank on."],
    ["TRANSPOSICIÓN", "Multiplies every frequency (0.25x to 4x), preserving inharmonic ratios between partials."],
    ["DERIVA VÁLV. (valve drift)", "Slow, independent detuning of each oscillator, 0 to 5 cents."],
    ["NIVEL", "Bank level."],
    ["ANALIZAR CARRETE ACTIVO", "Finds the twelve strongest partials in the active reel's region and spreads them across the oscillators."],
    ["CAPTURAR", "Records the bank to a reel. Press again to stop."]],
 [0.27, 0.73]),

("h2", "7.7 Corridor (PASILLO)"),
("img", "delia-pasillo.png", 1.0, "PASILLO view: the four strips at the same scale."),
("p", "Lays out the four strips at the same scale on a linoleum floor, with their total length and play position. It shows at a glance "
      "which piece puts the strips out of step. Clicking a row selects that machine."),

("h1", "8. Automatable parameters"),
("p", "These parameters appear in the host and can be automated. Host names are in Spanish. In the table, n is the machine number (1 to 4), oscillator (1 to 12) or gate (1 to 8)."),
("table", [["Identifier", "Host name (meaning)", "Range", "Default"],
    ["m<i>n</i>_speed", "Máquina n velocidad (speed)", "0.25x to 4x (logarithmic)", "1x"],
    ["m<i>n</i>_run", "Máquina n marcha (run)", "off / on", "off"],
    ["m<i>n</i>_rev", "Máquina n atrás (reverse)", "off / on", "off"],
    ["m<i>n</i>_loop", "Máquina n bucle (loop)", "off / on", "on"],
    ["m<i>n</i>_level", "Máquina n nivel (level)", "−60 to +6 dB", "−3 dB"],
    ["m<i>n</i>_pan", "Máquina n panorama", "−1 (left) to +1 (right)", "0"],
    ["m<i>n</i>_hp", "Máquina n paso alto (high-pass)", "--, 40, 80, 160, 320, 640, 1250, 2500 Hz", "--"],
    ["m<i>n</i>_lp", "Máquina n paso bajo (low-pass)", "--, 12, 8, 5.6, 4, 2.8, 2, 1.4, 1 kHz", "--"],
    ["m<i>n</i>_slope", "Máquina n 12 dB/oct", "off / on", "off"],
    ["m<i>n</i>_echo", "Máquina n envío eco (echo send)", "0 to 1", "0"],
    ["m<i>n</i>_plate", "Máquina n envío placa (plate send)", "0 to 1", "0"],
    ["m<i>n</i>_sync", "Máquina n sincronía (sync)", "Free, Start, Locked", "Free"],
    ["m<i>n</i>_ring", "Máquina n anillo (ring)", "No, Oscillator, Neighbour machine", "No"],
    ["tapeSpeed", "Velocidad de cinta (tape speed)", "9.5, 19, 38 cm/s", "38 cm/s"],
    ["quality", "Desgaste de cinta (tape wear)", "0 to 100 %", "35 %"],
    ["drift", "Deriva (drift)", "0 to 0.5 %", "0.05 %"],
    ["inertia", "Inercia (inertia)", "0 to 2 s", "0.35 s"],
    ["master", "Salida (output)", "−60 to +6 dB", "−3 dB"],
    ["echoSpeed", "Eco velocidad", "9.5 to 38 cm/s", "19 cm/s"],
    ["echoFb", "Eco realimentación (feedback)", "0 to 1.1", "0.35"],
    ["echoSpacing", "Eco separación (head spacing)", "2 to 10 cm", "5 cm"],
    ["echoRet", "Eco retorno (return)", "0 to 1", "0.8"],
    ["echoH1, echoH2, echoH3", "Eco cabezal 1 to 3 (heads)", "off / on", "1 on; 2 and 3 off"],
    ["plateDecay", "Placa caída (decay)", "1 to 5 s", "2.5 s"],
    ["plateDamp", "Placa amortiguación (damping)", "0 to 1", "0.45"],
    ["platePre", "Placa predelay", "0 to 0.2 s", "0.01 s"],
    ["plateRet", "Placa retorno (return)", "0 to 1", "0.8"],
    ["oscFreq", "Oscilador frecuencia", "20 Hz to 20 kHz (log)", "440 Hz"],
    ["oscLevel", "Oscilador nivel", "0 to 1", "0"],
    ["oscWave", "Oscilador onda (wave)", "Sine, Square", "Sine"],
    ["noiseLevel", "Ruido nivel (noise)", "0 to 1", "0"],
    ["wobCenter", "Wobbulator centro", "20 Hz to 10 kHz (log)", "220 Hz"],
    ["wobWidth", "Wobbulator anchura (width)", "0 to 5000 Hz", "120 Hz"],
    ["wobRate", "Wobbulator velocidad (rate)", "0.1 to 50 Hz (log)", "4 Hz"],
    ["wobLevel", "Wobbulator nivel", "0 to 1", "0"],
    ["wobShape", "Wobbulator forma (shape)", "Sine, Triangle, Saw", "Sine"],
    ["labHp, labLp", "Laboratorio paso alto / bajo", "as on the machines", "--"],
    ["labEcho, labPlate", "Laboratorio envíos (sends)", "0 to 1", "0"],
    ["bankOn", "Banco encendido (bank on)", "off / on", "off"],
    ["bankTransp", "Banco transposición", "0.25x to 4x (log)", "1x"],
    ["bankMaster", "Banco nivel", "0 to 1", "0.6"],
    ["bankDrift", "Banco deriva de válvula (valve drift)", "0 to 5 cents", "1 cent"],
    ["bFreq<i>n</i>", "Banco n frecuencia", "0 to 20,000 Hz", "110 Hz × n"],
    ["bLevel<i>n</i>", "Banco n nivel", "0 to 1", "1 / n"],
    ["bWave<i>n</i>", "Banco n onda", "Sine, Square", "Sine"],
    ["bGate<i>n</i>", "Banco n puerta (gate)", "Open, Gate 1 to 8", "gate n (osc. 9 to 12: gates 1 to 4)"],
    ["gAtt<i>n</i>", "Puerta n ataque (attack)", "0.001 to 2 s", "0.01 s"],
    ["gDec<i>n</i>", "Puerta n caída (decay)", "0.01 to 10 s", "1.2 s"]],
 [0.2, 0.28, 0.3, 0.22]),
("h2", "Settings stored in the project"),
("p", "These are not automatable but are saved with the project: the reel list (file path, generation, reading start and end, fades, "
      "root, scale, keyboard range), the four strips with all their pieces, the manual tempo and whether the host tempo is used, the "
      "note value, the cutting error, the reels folder, stereo mode, step input and the open views."),

("h1", "9. MIDI, outputs and files"),
("table", [["Channel", "Notes", "Function"],
    ["1", "all", "Play the active reel's notes (note 60 = degree 0). With PASOS POR MIDI, each note splices a piece."],
    ["2", "60 to 67", "Open gates 1 to 8 of the bank while the note is held."],
    ["3", "60 to 63", "Run / stop machines 1 to 4 (each note toggles)."]],
 [0.1, 0.14, 0.76]),
("p", "Delia has a main stereo output and four optional mono outputs, one per machine (after its filters and level). Enable them in the "
      "host to process each machine on its own track."),
("p", "The project stores file paths, not audio. If a file is missing when the project opens, the reel shows \u201cFALTA EL FICHERO\u201d "
      "(file missing) and the status line says so; click it to relocate. Bounced and captured reels are always written to the reels folder; "
      "it is worth pointing it at a project folder."),

("h1", "10. Technical notes"),
("h3", "Variable-speed reading"),
("p", "Each note is read through a bank of windowed-sinc filters (Blackman-Harris) with 256 interpolated phases. When reading faster than "
      "the original, a kernel with a lower cutoff and more taps is used, so nothing above the new Nyquist frequency folds back. In testing, "
      "a 9 kHz sine read at 4x stays below −100 dB."),
("h3", "Tape model"),
("p", "Saturation uses the Jiles-Atherton magnetic hysteresis model as implemented in ChowTape (Jatin Chowdhury, DAFx 2019), solved with "
      "fourth-order Runge-Kutta at 2x oversampling. Small-signal gain is calibrated so level is preserved. After it come the frequency "
      "response of the tape speed (treble rolled off at roughly 19, 13 or 7.5 kHz at 38, 19 or 9.5 cm/s, lower as wear increases) and a "
      "bass bump, wow (0.9 Hz) and flutter (8.7 Hz) that deepen at low speed, short and sparse dropouts, and hiss. All randomness uses "
      "stored seeds, so a project sounds the same every time it opens."),
("h3", "Echo and plate"),
("p", "The echo is a delay line read by up to three heads, whose spacing depends on the cm and the speed; feedback passes through soft "
      "saturation and the tape model on every repeat, so repeats darken and do not run away even above 100 %. The plate follows the reverb "
      "topology published by Jon Dattorro, with input diffusers, a modulated tank and damping."),
("h3", "Partial analysis"),
("p", "The reel region is analysed with a 16,384 or 32,768-point FFT and a Hann window, averaging overlapping windows. Peaks are refined "
      "with parabolic interpolation and the twelve strongest are kept, at least 20 % of the root frequency apart."),

("h1", "11. Known limitations"),
("ul", [
    "The window has a fixed size: 1240 × 820 pixels, plus 236 with the drawer open.",
    "Reel file paths are absolute.",
    "Real-time bounces are limited to 3 minutes per take; offline bounces to 10 minutes.",
    "Preview always reads at the file's original speed (note 0); to hear other notes, right-click the EMPALME keyboard.",
    "The interface is in Spanish only.",
]),

("h1", "12. Licence and credits"),
("p", "Delia is free software under the GPLv3 licence; the source is at github.com/eloyplatas/DELIA. It includes an adaptation of the "
      "hysteresis model from Jatin Chowdhury's ChowTape / AnalogTapeModel (GPLv3). The plate follows Jon Dattorro's topology (\u201cEffect "
      "Design, Part 1\u201d, Journal of the Audio Engineering Society, 1997). Built with JUCE 8."),
("p", "Design and development: Eloy Platas, with help from Claude (Anthropic)."),

("h1", "13. Sources"),
("p", "The biography and techniques are based on these sources, consulted in November 2026."),
("src", [
    "The Register, \u201cDelia and the Doctor: How to cook up a tune for a Time Lord\u201d (2013): theregister.com/2013/11/19/doctor_who_theme_elements_examined/",
    "WikiDelia: Oxford Dictionary of National Biography entry; Green Lampshade; Pot Au Feu; Tomorrow's World video; Masters of Sound; Chronology: wikidelia.net",
    "Delia Derbyshire Day, guest post by Will Davis on the Doctor Who theme: deliaderbyshireday.com/?p=3950",
    "Sound On Sound, \u201cThe Delia Derbyshire Archive\u201d (podcast, 2023): soundonsound.com/people/delia-derbyshire-archive-podcast",
    "Reverb, \u201cBeyond Doctor Who: Delia Derbyshire's Pioneering Electronic Music\u201d: reverb.com/news/beyond-doctor-who-delia-derbyshires-pioneering-electronic-music",
    "Classical Music, \u201cDelia Derbyshire: electronic music's unsung pioneer\u201d: classical-music.com",
    "Archives Hub, Juvenile Papers of Delia Derbyshire (GB 133 BDD), University of Manchester Library",
    "University of Manchester, \u201cLost tapes of Dr Who composer reveal hidden treasures\u201d: manchester.ac.uk",
    "Teresa Winter, <i>Delia Derbyshire: Sound and Music for the BBC Radiophonic Workshop, 1962-1973</i>, PhD thesis, University of York, 2015",
    "J. Chowdhury, \u201cReal-time Physical Modelling for Analog Tape Machines\u201d, DAFx 2019; code at github.com/jatinchowdhury18/AnalogTapeModel",
]),
]}

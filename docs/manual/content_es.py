# Contenido del manual de Delia en castellano
L = {
"meta_title": "Delia: manual de usuario",
"meta_subject": "Taller de cinta VST3 inspirado en los métodos de Delia Derbyshire",
"cover_sub": "TALLER DE CINTA  \u00b7  MANUAL DE USUARIO",
"cover_ver": "Versión 0.2.0  \u00b7  VST3 para Linux, Windows y macOS",
"cover_line1": "Música hecha como en el BBC Radiophonic Workshop de los años sesenta:",
"cover_line2": "sonidos afinados por velocidad, empalmados por longitud y volcados de cinta a cinta.",
"header": "Manual de usuario \u00b7 versión 0.2.0",
"toc": "Índice",
"content": [

("h1", "1. Qué es Delia"),
("p", "Delia es un instrumento virtual en formato VST3 que reproduce el <b>procedimiento de trabajo</b> de Delia Derbyshire "
      "y del BBC Radiophonic Workshop en los años sesenta, con ficheros de audio en lugar de cinta magnética. "
      "No es un sampler tocable ni un efecto de cinta: es un taller. Se fabrican notas a partir de un sonido cambiando su "
      "velocidad, se pegan una tras otra como trozos de cinta medidos en centímetros, se hacen sonar en cuatro magnetófonos "
      "y se vuelca la mezcla a un carrete nuevo, que pierde calidad en cada generación."),
("p", "El propósito es que el método imponga sus restricciones, porque de ellas salía buena parte de aquel sonido: "
      "la altura y la duración van siempre unidas, una nota es un objeto con longitud y no un evento MIDI, la mezcla es "
      "destructiva, y lo natural es exprimir pocos sonidos en lugar de cargar muchos. Delia no intenta copiar ningún aparato "
      "concreto ni incluye sonidos de Derbyshire; ofrece las herramientas y las reglas del taller para que el resultado sea tuyo."),
("img", "delia-principal.png", 1.0, "Vista principal: cuatro máquinas arriba; carretes y Makeup, Empalme, y Laboratorio abajo."),
("h2", "Resumen de funciones"),
("ul", [
    "<b>Carretes y Makeup:</b> carga WAV, AIFF, FLAC u OGG; elige el punto de inicio y de fin de lectura, los fundidos, la raíz y la escala; preescucha.",
    "<b>Empalme:</b> secuenciador en el que cada nota es un trozo de cinta con longitud en cm, corte recto o en diagonal, inversión y error de corte.",
    "<b>Cuatro máquinas:</b> bucles de longitud propia, variador de velocidad de 0,25x a 4x, deriva, inercia de motor, filtros pasivos, modulador en anillo, sincronía con el anfitrión.",
    "<b>Volcado:</b> graba la mezcla a un carrete nuevo con una generación más y pérdida de cinta (saturación, agudos, soplido, wow y flutter).",
    "<b>Laboratorio:</b> oscilador de prueba, ruido blanco y wobbulator, capturables a carrete.",
    "<b>Tratamientos:</b> eco de cinta de tres cabezales y reverberación de placa.",
    "<b>Doce osciladores:</b> cajón desplegable con doce osciladores, ocho puertas y análisis de parciales para resíntesis aditiva.",
]),

("h1", "2. Historia y propósito del proyecto"),
("p", "Delia nació en noviembre de 2026 de una pregunta: cómo se haría hoy música con las técnicas de Delia Derbyshire si en "
      "lugar de cinta se usaran ficheros de audio. Primero se investigaron sus procedimientos, documentados en entrevistas, en el "
      "archivo de la Universidad de Manchester y en estudios sobre el Radiophonic Workshop. Después se escribió una especificación "
      "que daba prioridad a las técnicas de producción sobre la imitación del sonido, y el plugin se llamó Delia."),
("p", "Las decisiones de diseño principales fueron estas:"),
("ul", [
    "Cuatro máquinas. Para el tema de Doctor Who se usaron tres magnetófonos sincronizados y una cuarta máquina para el volcado; Delia ofrece cuatro reproductores y el volcado aparte.",
    "Carretes mono, como la cinta de un cuarto de pulgada del taller, con la opción de conservar el estéreo.",
    "Saturación con el modelo físico de histéresis magnética de ChowTape, el modelo de cinta abierto más riguroso disponible.",
    "Los doce osciladores dentro del plugin, en un cajón que se abre con un interruptor.",
    "Sin banco de sonidos: el plugin empieza vacío, como el taller.",
    "Interfaz dibujada por código que evoca los magnetófonos de los años cincuenta y sesenta: esmalte martelé, paneles crema, mandos de baquelita, teclas de piano y medidores de aguja.",
]),
("h2", "Versiones"),
("table", [["Versión", "Fecha", "Cambios"],
           ["0.1.0", "16-11-2026", "Primera versión completa: carretes, Makeup, Empalme, cuatro máquinas, modelo de cinta, laboratorio, tratamientos, volcado, doce osciladores, pasillo."],
           ["0.2.0", "17-11-2026", "Punto de inicio y fin de lectura del carrete en segundos, preescucha de la región, regla de escucha desde cualquier punto, doble clic para fijar el inicio. Compilación para Linux, Windows y macOS. Manuales en PDF."]],
 [0.12, 0.14, 0.74]),

("h1", "3. Delia Derbyshire (1937-2001)"),
("p", "Delia Ann Derbyshire nació el 5 de mayo de 1937 en Coventry. Pasó parte de la guerra evacuada en Preston y contaba que "
      "el sonido de las sirenas antiaéreas despertó su interés por el sonido electrónico. Estudió en el instituto Barr's Hill y ganó "
      "una beca para estudiar matemáticas en el Girton College de Cambridge; al cabo de un año se pasó a la música y se graduó "
      "en 1959 en matemáticas y música. Esa doble formación marcó toda su obra."),
("p", "En 1959 se presentó a Decca y le dijeron que la compañía no empleaba mujeres en sus estudios de grabación. Trabajó en "
      "Ginebra para las Naciones Unidas, dio clases en Coventry y pasó por la editorial musical Boosey &amp; Hawkes antes de entrar "
      "en la BBC en noviembre de 1960 como ayudante de estudio en prácticas. En abril de 1962 fue destinada al Radiophonic Workshop, "
      "el primer estudio institucional de música electrónica de Gran Bretaña."),
("p", "En 1963 realizó la versión electrónica del tema de <i>Doctor Who</i> compuesto por Ron Grainer, construida, en sus palabras, "
      "pulgada a pulgada sobre cinta mono de un cuarto de pulgada, sin músicos ni sintetizadores. Fue la sintonía de la serie de 1963 a "
      "1980. Durante una década creó más de doscientas piezas para radio y televisión, muchas sin acreditar, porque la BBC atribuía "
      "el trabajo al Workshop y no a sus miembros. Entre ellas están las <i>Inventions for Radio</i> con Barry Bermange, "
      "<i>Blue Veils and Golden Sands</i> (1967), <i>The Delian Mode</i> (1968), <i>Pot au Feu</i> (1968) y <i>Dance from Noah</i> (1971)."),
("p", "Fuera de la BBC fundó en 1966 Unit Delta Plus con Brian Hodgson y Peter Zinovieff, y con Hodgson y David Vorhaus el estudio "
      "Kaleidophon, donde grabaron como White Noise el álbum <i>An Electric Storm</i> (Island, 1969). En 1973 dejó la BBC, desencantada "
      "por las limitaciones a su creatividad y por la llegada de sintetizadores de serie; con Hodgson hizo en el estudio Electrophon la "
      "banda sonora de <i>The Legend of Hell House</i>. Después abandonó la música durante años: fue operadora de radio en un gasoducto, "
      "trabajó en una librería y en una galería de arte, y desde 1980 vivió en Northampton con su pareja, Clive Blackburn."),
("p", "A finales de los noventa el interés por su obra creció y volvió a trabajar, entre otros con Peter Kember (Sonic Boom). "
      "Murió de insuficiencia renal el 3 de julio de 2001 en Northampton, a los 64 años. En 2007 su archivo personal de cintas y "
      "papeles pasó a la Universidad de Manchester, donde puede consultarse en la John Rylands Library, y cada 23 de noviembre, "
      "aniversario de la primera emisión de <i>Doctor Who</i>, se celebra el Delia Derbyshire Day."),
("note", "Delia es un homenaje a su método de trabajo. No está afiliado a su legado, a la BBC ni a los titulares de sus obras, y no contiene grabaciones suyas."),

("h1", "4. Las técnicas del taller y cómo las implementa Delia"),
("p", "El trabajo en el Radiophonic Workshop era artesanal: magnetófonos, osciladores de válvulas, filtros, una cuchilla y mucha "
      "aritmética. Cada técnica de la tabla tiene su equivalente en Delia; el capítulo 7 explica cada control."),
("table", [["Técnica en cinta", "Cómo se hacía", "Cómo lo hace Delia"],
    ["Varispeed", "Una misma grabación se copiaba a distintas velocidades: más lenta, más grave y más larga; más rápida, más aguda y más corta. Así se sacaban los graves y los agudos de un ritmo.",
     "Makeup: cada grado de la escala es el mismo sonido leído a otra velocidad (razón = frecuencia destino / raíz). No hay time-stretch: altura y duración van unidas. Lectura con interpolación sinc enventanada y filtro antialiasing cuando se acelera."],
    ["Una nota, un trozo de cinta", "El bajo de Doctor Who se hizo con una cuerda pulsada grabada a varias alturas y empalmada nota a nota con cuchilla, bloque de edición, lápiz graso y cinta adhesiva. La duración de una nota era su longitud.",
     "Empalme: cada trozo tiene nota, largo en cm, punto de inicio y tipo de corte. La regla muestra cuántos cm mide cada figura a la velocidad de cinta y el tempo actuales."],
    ["Corte recto o diagonal", "Un corte recto da un clic; un corte en diagonal sobre el bloque funde un trozo con el siguiente durante unos milímetros.",
     "Corte recto (salto directo), a 45° o a 60° (fundido cruzado de potencia constante cuya duración sale de la anchura de la cinta, 6,35 mm, y la velocidad)."],
    ["Imprecisión manual", "Los empalmes nunca eran exactos; en Doctor Who un empalme un poco largo desincronizó las cintas.",
     "ERROR DE CORTE: desvía cada largo unos milímetros, siempre igual para el mismo proyecto. Vista PASILLO para encontrar a ojo el trozo que desajusta, como cuando extendían las cintas por el pasillo."],
    ["Máquinas sincronizadas", "Tres Philips EL3503 modificados para correr juntos llevaban las submezclas.",
     "Cuatro máquinas con bucle de longitud propia, deriva lenta e independiente de velocidad, inercia de motor y tres modos de sincronía con el anfitrión."],
    ["Bucles", "Bucles de cinta de hasta quince metros, a veces fuera del estudio.",
     "Cada tira se reproduce en bucle; tiras de longitudes distintas se desfasan entre sí y generan ciclos largos."],
    ["Volcado y generaciones", "Las submezclas se regrababan en otra máquina, con los cambios de volumen y efectos hechos durante el volcado. Cada copia sumaba soplido y perdía agudos.",
     "VOLCAR graba la mezcla en tiempo real (o fuera de línea) a un carrete nuevo, con contador de generación y una pasada por el modelo de cinta. El carrete vuelve a entrar en Makeup."],
    ["Bibliotecas de makeup", "Derbyshire fabricaba un fondo de sonidos y luego montaba: diez sonidos alimentan varias piezas de la familia de Pot au Feu.",
     "La estantería de carretes: cualquier carrete, cargado, capturado o volcado, sirve como fuente en cualquier tira."],
    ["Quitar el ataque", "Con la pantalla de lámpara verde grabó el golpe y entró con un fundido en la parte resonante, sin el ataque.",
     "Punto de INICIO de lectura y fundido de entrada en Makeup, con preescucha para oír exactamente lo que sonará."],
    ["Doce osciladores y ocho puertas", "Doce osciladores de válvulas Jason y ocho circuitos de puerta construidos en la casa; la unidad de teclas solo abría y cerraba los generadores.",
     "Cajón DOCE OSCILADORES: frecuencia en Hz, nivel, onda, deriva de válvula y asignación a ocho puertas con ataque y caída."],
    ["Análisis y resíntesis", "Para Blue Veils and Golden Sands analizó el sonido de la pantalla en sus parciales y reconstruyó los doce más fuertes con los doce osciladores.",
     "ANALIZAR CARRETE ACTIVO: FFT promediada, picos con interpolación parabólica y reparto de los doce más fuertes en los osciladores."],
    ["Afinación calculada", "Calculaba las afinaciones con tablas de logaritmos, en escalas pitagórica y mesotónica.",
     "Escalas temperada, pitagórica, mesotónica de cuarto de coma y libre (razones o fichero Scala .scl)."],
    ["Wobbulator y ruido filtrado", "El wobbulator, un generador de barrido de frecuencia, dio las burbujas graves; las nubes eran ruido blanco filtrado.",
     "Laboratorio: wobbulator con centro, anchura, velocidad y forma; ruido blanco; oscilador de prueba; filtros pasivos. Todo se captura a carrete."],
    ["Eco y placa", "Eco con un bucle que pasaba por varios cabezales; reverberación de placa EMT 140; EQ pasiva.",
     "Eco de cinta con tres cabezales, separación y velocidad, que pasa por el modelo de cinta en cada vuelta; placa de topología Dattorro; filtros pasivos por máquina."],
 ], [0.17, 0.39, 0.44]),

("h1", "5. Instalación"),
("table", [["Sistema", "Fichero", "Dónde copiarlo"],
    ["Linux (x86_64)", "Delia.vst3", "~/.vst3/ (usuario) o /usr/lib/vst3/ (sistema)"],
    ["Windows (64 bits)", "Delia.vst3", "C:\\Program Files\\Common Files\\VST3\\"],
    ["macOS (Intel y Apple Silicon)", "Delia.vst3", "~/Library/Audio/Plug-Ins/VST3/ o /Library/Audio/Plug-Ins/VST3/"]],
 [0.24, 0.18, 0.58]),
("p", "Después de copiarlo, vuelve a escanear los plugins en el anfitrión. En REAPER: Opciones, Preferencias, Plug-ins, VST, Re-scan."),
("note", "<b>macOS:</b> el plugin está firmado ad hoc pero no notarizado. Si macOS lo bloquea, abre el Terminal y ejecuta "
         "<font face='Mono'>xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Delia.vst3</font>. "
         "<b>Linux:</b> la versión publicada se compila en Ubuntu 22.04 y funciona en distribuciones con glibc 2.35 o posterior."),
("p", "Para compilarlo desde el código, sigue las instrucciones del README del repositorio github.com/eloyplatas/DELIA."),

("h1", "6. Primeros pasos"),
("ol", [
    "Inserta Delia en una pista de REAPER como instrumento. La salida principal es estéreo.",
    "Pulsa <b>CARGAR...</b> en el panel CARRETES o arrastra un fichero de audio a la ventana. Un sonido corto y resonante (una cuerda pulsada, un objeto metálico golpeado) es un buen comienzo.",
    "En la forma de onda, arrastra el borde izquierdo hasta donde quieras que empiece la lectura, o escribe el tiempo en <b>INICIO s</b>. Pulsa <b>ESCUCHAR</b> para oír la región tal como sonará la nota 0.",
    "Comprueba la <b>RAÍZ Hz</b> (DETECTAR la calcula) y elige la <b>ESCALA</b>.",
    "En EMPALME, elige la máquina 1, una figura (por ejemplo 1/8) y pulsa notas en el teclado: cada clic pega un trozo.",
    "Pulsa la tecla roja ▶ de la máquina 1. La tira suena en bucle.",
    "Prepara otra tira en la máquina 2, ponla en marcha y ajusta niveles, filtros y envíos.",
    "Pulsa <b>VOLCAR</b> para grabar la mezcla en un carrete nuevo. Ese carrete es de generación 1 y puede volver a cortarse en Makeup.",
]),

("h1", "7. Referencia de la interfaz"),
("h2", "7.1 Cabecera"),
("table", [["Control", "Función"],
    ["CINTA cm/s", "Velocidad de cinta global: 9,5, 19 o 38 cm/s. Fija la conversión entre longitud y tiempo en todas las tiras y la respuesta en frecuencia del modelo de cinta (menos agudos a menos velocidad). Al cambiarla se vuelven a montar las tiras."],
    ["DESGASTE", "Cuánto degrada cada pasada por cinta (volcado y eco), de 0 a 100 %. A 0 la copia es exacta."],
    ["DERIVA", "Desviación lenta e independiente de la velocidad de cada máquina, de 0 a 0,5 %. Con deriva, las máquinas se separan con el tiempo como motores reales."],
    ["INERCIA", "Tiempo de arranque y parada del motor, de 0 a 2 s. Se oye como un glissando."],
    ["SALIDA", "Nivel de la salida principal."],
    ["Medidores IZQ / DCHA", "Nivel de la salida principal."],
    ["PASILLO", "Sustituye los paneles de Carretes y Empalme por la vista de las cuatro tiras a la misma escala."],
    ["12 OSCILADORES", "Despliega el cajón de los doce osciladores bajo la ventana."],
    ["Línea de estado", "Mensajes: carretes cargados, volcados, ficheros que faltan, resultado del análisis."]],
 [0.24, 0.76]),

("h2", "7.2 Máquinas"),
("img", "delia-maquina.png", 0.52, "Una máquina: transporte con carretes, contador en cm y cabezas; debajo, el panel de mandos."),
("p", "Cada máquina reproduce la tira que se le ha montado en EMPALME. Los carretes giran según la velocidad y la cantidad de cinta "
      "de cada lado; el contador marca la posición en cm y el piloto se enciende cuando la máquina está en marcha. Al pulsar sobre "
      "una máquina se convierte en la máquina activa del panel EMPALME."),
("table", [["Control", "Función"],
    ["◀", "Marcha atrás: la tira se lee al revés."],
    ["▶ (tecla roja)", "Marcha y paro, con la inercia de la cabecera."],
    ["∞", "Bucle. Apagado, la tira suena una sola vez. El bucle forma parte del montaje: la unión entre el último trozo y el primero lleva su fundido."],
    ["VARIADOR", "Velocidad de 0,25x a 4x. Cambia altura y tempo de toda la tira a la vez. Doble clic: 1x."],
    ["Medidor VU", "Nivel de la máquina."],
    ["12 dB/oct", "Dobla la pendiente de los filtros GRAVES y AGUDOS (6 o 12 dB por octava)."],
    ["NIVEL, PAN", "Nivel (−60 a +6 dB) y panorama en la mezcla."],
    ["GRAVES", "Filtro paso alto pasivo en pasos fijos: apagado, 40, 80, 160, 320, 640, 1250 o 2500 Hz."],
    ["AGUDOS", "Filtro paso bajo pasivo en pasos fijos: apagado, 12, 8, 5,6, 4, 2,8, 2, 1,4 o 1 kHz."],
    ["ECO, PLACA", "Envíos al eco de cinta y a la placa."],
    ["SINCRO", "Libre: gira a su velocidad sin atender al transporte. Arranque: vuelve al principio cuando arranca el transporte del anfitrión. Enganchado: la posición sale de la posición del proyecto y la deriva se desactiva."],
    ["ANILLO", "Modulador en anillo: No, con el oscilador de prueba del laboratorio, o con la máquina vecina."]],
 [0.2, 0.8]),

("h2", "7.3 Carretes y Makeup"),
("img", "delia-carretes.png", 0.5, "Panel de carretes y Makeup, con la preescucha en marcha."),
("p", "La estantería guarda todos los carretes de la sesión: los cargados, los capturados y los volcados. Cada fila muestra la "
      "generación (G0 es un fichero original), si es mono (M) o estéreo (ST) y la duración. Al pulsar una fila, ese carrete pasa a ser "
      "el activo; si falta su fichero, se abre un diálogo para reubicarlo. Una fila puede arrastrarse a REAPER para usar el WAV."),
("table", [["Control", "Función"],
    ["CARGAR...", "Carga uno o varios ficheros WAV, AIFF, FLAC, OGG o MP3. También se pueden arrastrar a la ventana."],
    ["QUITAR", "Quita el carrete activo de la estantería (no borra el fichero)."],
    ["CARPETA...", "Carpeta donde se escriben los carretes volcados, capturados y las tiras exportadas. Por defecto, Documentos/Delia/Carretes."],
    ["ESTÉREO", "Apagado, los carretes nuevos se suman a mono. Encendido, los que se carguen o vuelquen a partir de ese momento conservan los dos canales."],
    ["▶ ESCUCHAR / ■ PARAR", "Preescucha de la región, desde el punto de inicio hasta el de fin, a velocidad original y con los fundidos: exactamente como suena la nota 0. Una línea roja recorre la forma de onda."],
    ["Forma de onda", "Arrastra el borde izquierdo (inicio de lectura) o el derecho (fin). Los dos puntos de arriba son los fundidos de entrada y de salida. Doble clic: coloca ahí el inicio y lo preescucha."],
    ["Regla de escucha", "Franja inferior de la forma de onda, con los segundos. Un clic escucha el fichero desde ese punto hasta el final, sin fundidos; arrastrar recorre el sonido. Los triángulos marcan el inicio y el fin."],
    ["INICIO s / FIN s", "Punto de inicio y de fin de lectura en segundos, para colocarlos con precisión. Las notas empiezan a leer en el INICIO."],
    ["FUND. ENT. ms / FUND. SAL. ms", "Fundidos de entrada y de salida de cada nota. Un fundido de entrada largo quita el ataque."],
    ["RAÍZ Hz / DETECTAR", "Altura del sonido original (nota 0). DETECTAR la calcula; en sonidos inarmónicos, escribe a mano el parcial de referencia."],
    [".SCL...", "Carga una escala Scala y la pone como escala libre."],
    ["DESDE / HASTA", "Notas por debajo y por encima de la raíz que aparecen en el teclado de EMPALME."],
    ["ESCALA", "Temperada (12), Pitagórica, Mesotónica de cuarto de coma o Libre. En Libre aparece un campo para escribir razones: 1 9/8 5/4 3/2 5/3."],
    ["Línea inferior", "Frecuencia de la nota 0, duración de la región en segundos y en cm a la velocidad de cinta actual, y generación."]],
 [0.27, 0.73]),
("note", "<b>Altura y duración van unidas.</b> Una nota una octava por encima de la raíz se lee al doble de velocidad y dura la mitad. "
         "Si el trozo es más largo que el sonido, el resto es cinta virgen con su soplido."),

("h2", "7.4 Empalme"),
("img", "delia-empalme.png", 0.8, "Panel de Empalme: tira sobre el bloque, regla en cm y teclado de notas del carrete activo."),
("p", "La tira de la máquina activa aparece sobre un bloque de empalme, con una regla en centímetros y marcas rojas cada figura. "
      "Cada trozo lleva su nota escrita; los invertidos aparecen rayados y la cinta virgen en blanco. Una línea roja indica por dónde "
      "va la reproducción. Los botones 1 a 4 de arriba cambian de máquina."),
("table", [["Control", "Función"],
    ["Teclado de notas", "Clic: escucha la nota y pega un trozo con la figura activa a continuación del seleccionado. Clic derecho: solo escucha. Al pasar el ratón, muestra la frecuencia y cuántos cm de sonido tiene esa nota."],
    ["Tira", "Clic: selecciona un trozo. Arrastrar el borde derecho: cambia su largo (se ajusta a un cuarto de la figura; con Mayúsculas, libre). Rueda: desplaza."],
    ["FIGURA", "Redonda, blanca, negra, corchea, semicorchea y fusa, con puntillo (.) y tresillo (3). El panel muestra cuántos cm mide."],
    ["TEMPO / DEL ANFITRIÓN", "Tempo para calcular las figuras: el del proyecto o uno escrito a mano."],
    ["NOTA", "Grado de la escala del trozo seleccionado (0 = raíz)."],
    ["LARGO cm", "Longitud del trozo."],
    ["INICIO cm", "Desde qué punto del sonido empieza a leer el trozo, en cm de cinta a partir del inicio de lectura del carrete."],
    ["Corte", "Recto, a 45° o a 60°. El corte se aplica a la unión con el trozo anterior."],
    ["INVERTIR", "Pega el trozo al revés."],
    ["◀ ▶", "Mueve el trozo hacia el principio o el final."],
    ["DUPLICAR, CORTAR", "Duplica el trozo o lo quita de la tira."],
    ["+ VIRGEN", "Añade un trozo de cinta virgen (silencio con soplido) con la longitud de la figura."],
    ["VACIAR", "Quita todos los trozos de la tira."],
    ["PASOS POR MIDI", "Cada nota del canal MIDI 1 pega un trozo (do central, nota 60, es el grado 0)."],
    ["ERROR DE CORTE mm", "Desvía cada largo hasta esos milímetros, de forma fija para el proyecto."],
    ["EXPORTAR TIRA", "Escribe la tira montada como WAV en la carpeta de carretes."]],
 [0.22, 0.78]),
("p", "A 38 cm/s y 120 pulsos por minuto, una negra mide 19 cm. A 19 cm/s, la misma negra mide 9,5 cm. No hay compases obligatorios: "
      "una tira de 11 o 13 corcheas, como los bucles de <i>Pot au Feu</i>, se construye igual que una de 8."),

("h2", "7.5 Laboratorio, tratamientos y volcado"),
("img", "delia-laboratorio.png", 0.5, "Laboratorio, tratamientos y volcado."),
("table", [["Control", "Función"],
    ["OSC Hz, OSC NIV., CUADRADA", "Oscilador de prueba: frecuencia de 20 Hz a 20 kHz, nivel, onda senoidal o cuadrada. También es la portadora del modulador en anillo."],
    ["RUIDO", "Nivel de ruido blanco."],
    ["GRAVES, AGUDOS", "Filtros pasivos del laboratorio."],
    ["A ECO, A PLACA", "Envíos del laboratorio a los tratamientos."],
    ["WOB Hz, ANCHO, VELOC., WOB NIV., FORMA", "Wobbulator: oscilador cuya frecuencia barre periódicamente alrededor de un centro. Anchura en Hz, velocidad de 0,1 a 50 Hz, forma seno, triángulo o sierra."],
    ["CAPTURAR LAB.", "Graba el laboratorio a un carrete. Pulsa otra vez para parar."],
    ["ECO cm/s, REALIM., SEPAR., RET. ECO", "Eco de cinta: velocidad del bucle, realimentación (hasta 110 %), separación entre cabezales en cm y retorno."],
    ["CABEZAL 1, 2, 3", "Cabezales de lectura activos. Cada cabezal está una separación más lejos que el anterior."],
    ["CAÍDA, AMORT., PREDEL., RET. PL.", "Placa: tiempo de caída de 1 a 5 s, amortiguación de agudos, predelay y retorno."],
    ["VOLCAR", "Graba la mezcla en tiempo real, con los movimientos que hagas, hasta 3 minutos. Pulsa otra vez para parar."],
    ["cm / = BUCLE", "Longitud del volcado fuera de línea. = BUCLE pone la de la tira más larga."],
    ["VOLCAR FUERA DE LÍNEA", "Renderiza al instante esa longitud con las máquinas en marcha desde el principio."]],
 [0.32, 0.68]),
("p", "Cada carrete volcado tiene la generación de su fuente más vieja más uno, pasa una vez por el modelo de cinta (la pérdida queda "
      "grabada en el audio) y se escribe como WAV de 24 bits. El carrete volcado no recuerda sus trozos: el volcado es destructivo, "
      "como en el taller."),

("h2", "7.6 Doce osciladores"),
("img", "delia-osciladores.png", 1.0, "Ventana completa con el cajón de los doce osciladores desplegado."),
("table", [["Control", "Función"],
    ["Frecuencia (1 a 12)", "Frecuencia de cada oscilador en Hz. 0 lo apaga."],
    ["Deslizador", "Nivel de cada oscilador."],
    ["CUAD.", "Onda cuadrada en lugar de senoidal."],
    ["Puerta (— o P1 a P8)", "Puerta que abre el oscilador. — = siempre abierto."],
    ["P1 a P8", "Mantén pulsado para abrir la puerta; Mayúsculas + clic la deja abierta. El piloto ámbar indica que está abierta. También por MIDI, canal 2."],
    ["ATAQUE, CAÍDA", "Envolvente de cada puerta."],
    ["ENCENDIDO", "Activa el banco."],
    ["TRANSPOSICIÓN", "Multiplica todas las frecuencias (0,25x a 4x) y conserva las relaciones inarmónicas entre parciales."],
    ["DERIVA VÁLV.", "Desafinación lenta e independiente de cada oscilador, de 0 a 5 cents."],
    ["NIVEL", "Nivel del banco."],
    ["ANALIZAR CARRETE ACTIVO", "Busca los doce parciales más fuertes de la región del carrete activo y los reparte en los osciladores."],
    ["CAPTURAR", "Graba el banco a un carrete. Pulsa otra vez para parar."]],
 [0.27, 0.73]),

("h2", "7.7 Pasillo"),
("img", "delia-pasillo.png", 1.0, "Vista PASILLO: las cuatro tiras a la misma escala."),
("p", "Extiende las cuatro tiras a la misma escala, sobre un suelo de linóleo, con su longitud total y la posición de reproducción. "
      "Sirve para ver de un vistazo qué trozo desajusta las tiras. Un clic sobre una fila elige esa máquina."),

("h1", "8. Tabla de parámetros automatizables"),
("p", "Estos parámetros aparecen en el anfitrión y se pueden automatizar. En la tabla, n es el número de máquina (1 a 4), de oscilador (1 a 12) o de puerta (1 a 8)."),
("table", [["Identificador", "Nombre", "Rango", "Por defecto"],
    ["m<i>n</i>_speed", "Máquina n velocidad", "0,25x a 4x (logarítmico)", "1x"],
    ["m<i>n</i>_run", "Máquina n marcha", "apagado / encendido", "apagado"],
    ["m<i>n</i>_rev", "Máquina n atrás", "apagado / encendido", "apagado"],
    ["m<i>n</i>_loop", "Máquina n bucle", "apagado / encendido", "encendido"],
    ["m<i>n</i>_level", "Máquina n nivel", "−60 a +6 dB", "−3 dB"],
    ["m<i>n</i>_pan", "Máquina n panorama", "−1 (izq.) a +1 (dcha.)", "0"],
    ["m<i>n</i>_hp", "Máquina n paso alto", "--, 40, 80, 160, 320, 640, 1250, 2500 Hz", "--"],
    ["m<i>n</i>_lp", "Máquina n paso bajo", "--, 12, 8, 5,6, 4, 2,8, 2, 1,4, 1 kHz", "--"],
    ["m<i>n</i>_slope", "Máquina n 12 dB/oct", "apagado / encendido", "apagado"],
    ["m<i>n</i>_echo", "Máquina n envío eco", "0 a 1", "0"],
    ["m<i>n</i>_plate", "Máquina n envío placa", "0 a 1", "0"],
    ["m<i>n</i>_sync", "Máquina n sincronía", "Libre, Arranque, Enganchado", "Libre"],
    ["m<i>n</i>_ring", "Máquina n anillo", "No, Oscilador, Máquina vecina", "No"],
    ["tapeSpeed", "Velocidad de cinta", "9,5, 19, 38 cm/s", "38 cm/s"],
    ["quality", "Desgaste de cinta", "0 a 100 %", "35 %"],
    ["drift", "Deriva", "0 a 0,5 %", "0,05 %"],
    ["inertia", "Inercia", "0 a 2 s", "0,35 s"],
    ["master", "Salida", "−60 a +6 dB", "−3 dB"],
    ["echoSpeed", "Eco velocidad", "9,5 a 38 cm/s", "19 cm/s"],
    ["echoFb", "Eco realimentación", "0 a 1,1", "0,35"],
    ["echoSpacing", "Eco separación", "2 a 10 cm", "5 cm"],
    ["echoRet", "Eco retorno", "0 a 1", "0,8"],
    ["echoH1, echoH2, echoH3", "Eco cabezales 1 a 3", "apagado / encendido", "1 encendido; 2 y 3 apagados"],
    ["plateDecay", "Placa caída", "1 a 5 s", "2,5 s"],
    ["plateDamp", "Placa amortiguación", "0 a 1", "0,45"],
    ["platePre", "Placa predelay", "0 a 0,2 s", "0,01 s"],
    ["plateRet", "Placa retorno", "0 a 1", "0,8"],
    ["oscFreq", "Oscilador frecuencia", "20 Hz a 20 kHz (log.)", "440 Hz"],
    ["oscLevel", "Oscilador nivel", "0 a 1", "0"],
    ["oscWave", "Oscilador onda", "Senoidal, Cuadrada", "Senoidal"],
    ["noiseLevel", "Ruido nivel", "0 a 1", "0"],
    ["wobCenter", "Wobbulator centro", "20 Hz a 10 kHz (log.)", "220 Hz"],
    ["wobWidth", "Wobbulator anchura", "0 a 5000 Hz", "120 Hz"],
    ["wobRate", "Wobbulator velocidad", "0,1 a 50 Hz (log.)", "4 Hz"],
    ["wobLevel", "Wobbulator nivel", "0 a 1", "0"],
    ["wobShape", "Wobbulator forma", "Seno, Triángulo, Sierra", "Seno"],
    ["labHp, labLp", "Laboratorio paso alto / bajo", "como los de las máquinas", "--"],
    ["labEcho, labPlate", "Laboratorio envíos", "0 a 1", "0"],
    ["bankOn", "Banco encendido", "apagado / encendido", "apagado"],
    ["bankTransp", "Banco transposición", "0,25x a 4x (log.)", "1x"],
    ["bankMaster", "Banco nivel", "0 a 1", "0,6"],
    ["bankDrift", "Banco deriva de válvula", "0 a 5 cents", "1 cent"],
    ["bFreq<i>n</i>", "Banco n frecuencia", "0 a 20 000 Hz", "110 Hz × n"],
    ["bLevel<i>n</i>", "Banco n nivel", "0 a 1", "1 / n"],
    ["bWave<i>n</i>", "Banco n onda", "Senoidal, Cuadrada", "Senoidal"],
    ["bGate<i>n</i>", "Banco n puerta", "Abierto, Puerta 1 a 8", "puerta n (osc. 9 a 12: puertas 1 a 4)"],
    ["gAtt<i>n</i>", "Puerta n ataque", "0,001 a 2 s", "0,01 s"],
    ["gDec<i>n</i>", "Puerta n caída", "0,01 a 10 s", "1,2 s"]],
 [0.2, 0.26, 0.32, 0.22]),
("h2", "Ajustes guardados en el proyecto"),
("p", "No son parámetros automatizables, pero se guardan con el proyecto: la lista de carretes (ruta del fichero, generación, inicio y fin "
      "de lectura, fundidos, raíz, escala, ámbito del teclado), las cuatro tiras con todos sus trozos, el tempo manual y si se usa el "
      "del anfitrión, la figura, el error de corte, la carpeta de carretes, el modo estéreo, la entrada por pasos y las vistas abiertas."),

("h1", "9. MIDI, salidas y ficheros"),
("table", [["Canal", "Notas", "Función"],
    ["1", "todas", "Escuchar las notas del carrete activo (nota 60 = grado 0). Con PASOS POR MIDI, cada nota pega un trozo."],
    ["2", "60 a 67", "Abrir las puertas 1 a 8 del banco mientras la nota está pulsada."],
    ["3", "60 a 63", "Marcha / paro de las máquinas 1 a 4 (cada nota alterna)."]],
 [0.1, 0.14, 0.76]),
("p", "Delia tiene una salida estéreo principal y cuatro salidas mono opcionales, una por máquina (después de sus filtros y su nivel). "
      "Actívalas en el anfitrión si quieres procesar cada máquina en una pista distinta."),
("p", "El proyecto guarda las rutas de los ficheros, no el audio. Si al abrirlo falta un fichero, el carrete aparece como «FALTA EL FICHERO» "
      "y la línea de estado lo indica; pulsa sobre él para reubicarlo. Los carretes volcados y capturados se escriben siempre en la carpeta "
      "de carretes; conviene apuntarla a una carpeta del proyecto."),

("h1", "10. Notas técnicas"),
("h3", "Lectura a velocidad variable"),
("p", "Cada nota se lee con un banco de filtros sinc enventanados (Blackman-Harris) de 256 fases interpoladas. Cuando la lectura es más "
      "rápida que la original, se usa un núcleo con frecuencia de corte más baja y más coeficientes, para que nada por encima de la nueva "
      "frecuencia de Nyquist se repliegue. En las pruebas, una senoide de 9 kHz leída a 4x queda por debajo de −100 dB."),
("h3", "Modelo de cinta"),
("p", "La saturación usa el modelo de histéresis magnética de Jiles-Atherton tal como lo implementa ChowTape (Jatin Chowdhury, DAFx 2019), "
      "resuelto con Runge-Kutta de cuarto orden y sobremuestreo 2x. La ganancia de pequeña señal se calibra para que el nivel se mantenga. "
      "Después se aplican la respuesta en frecuencia de la velocidad de cinta (corte de agudos de unos 19, 13 o 7,5 kHz a 38, 19 o 9,5 cm/s, "
      "más bajo cuanto mayor es el desgaste) y un realce de graves, wow (0,9 Hz) y flutter (8,7 Hz) más profundos a baja velocidad, caídas "
      "de señal breves y escasas, y soplido. Todo el azar usa semillas guardadas, así que un proyecto suena igual cada vez que se abre."),
("h3", "Eco y placa"),
("p", "El eco es una línea de retardo leída por hasta tres cabezales, cuya separación depende de los cm y de la velocidad; la realimentación pasa "
      "por una saturación suave y por el modelo de cinta en cada vuelta, así que las repeticiones se oscurecen y no se disparan aunque superen el 100 %. "
      "La placa sigue la topología de reverberación publicada por Jon Dattorro, con difusores de entrada, un tanque modulado y amortiguación."),
("h3", "Análisis de parciales"),
("p", "La región del carrete se analiza con FFT de 16 384 o 32 768 puntos y ventana de Hann, promediando ventanas solapadas. Los picos se afinan con "
      "interpolación parabólica y se conservan los doce más fuertes, separados al menos un 20 % de la raíz."),

("h1", "11. Límites conocidos"),
("ul", [
    "La ventana tiene tamaño fijo: 1240 × 820 píxeles, más 236 con el cajón abierto.",
    "Las rutas de los carretes son absolutas.",
    "El volcado en tiempo real admite hasta 3 minutos por toma; el volcado fuera de línea, hasta 10 minutos.",
    "La preescucha lee siempre a la velocidad original del fichero (nota 0); para oír otras notas, usa el teclado de EMPALME con clic derecho.",
]),

("h1", "12. Licencia y créditos"),
("p", "Delia es software libre bajo la licencia GPLv3; el código está en github.com/eloyplatas/DELIA. Incluye una adaptación del modelo de "
      "histéresis de ChowTape / AnalogTapeModel de Jatin Chowdhury (GPLv3). La placa sigue la topología de Jon Dattorro (\u00abEffect Design, Part 1\u00bb, "
      "Journal of the Audio Engineering Society, 1997). Está construido con JUCE 8."),
("p", "Diseño y desarrollo: Eloy Platas, con la ayuda de Claude (Anthropic)."),

("h1", "13. Fuentes"),
("p", "La biografía y las técnicas se basan en estas fuentes, consultadas en noviembre de 2026."),
("src", [
    "The Register, \u00abDelia and the Doctor: How to cook up a tune for a Time Lord\u00bb (2013): theregister.com/2013/11/19/doctor_who_theme_elements_examined/",
    "WikiDelia: Oxford Dictionary of National Biography entry; Green Lampshade; Pot Au Feu; Tomorrow's World video; Masters of Sound; Chronology: wikidelia.net",
    "Delia Derbyshire Day, entrada de Will Davis sobre el tema de Doctor Who: deliaderbyshireday.com/?p=3950",
    "Sound On Sound, \u00abThe Delia Derbyshire Archive\u00bb (podcast, 2023): soundonsound.com/people/delia-derbyshire-archive-podcast",
    "Reverb, \u00abBeyond Doctor Who: Delia Derbyshire's Pioneering Electronic Music\u00bb: reverb.com/news/beyond-doctor-who-delia-derbyshires-pioneering-electronic-music",
    "Classical Music, \u00abDelia Derbyshire: electronic music's unsung pioneer\u00bb: classical-music.com",
    "Archives Hub, Juvenile Papers of Delia Derbyshire (GB 133 BDD), University of Manchester Library",
    "Universidad de Manchester, \u00abLost tapes of Dr Who composer reveal hidden treasures\u00bb: manchester.ac.uk",
    "Teresa Winter, <i>Delia Derbyshire: Sound and Music for the BBC Radiophonic Workshop, 1962-1973</i>, tesis doctoral, University of York, 2015",
    "J. Chowdhury, \u00abReal-time Physical Modelling for Analog Tape Machines\u00bb, DAFx 2019; código en github.com/jatinchowdhury18/AnalogTapeModel",
]),
]}

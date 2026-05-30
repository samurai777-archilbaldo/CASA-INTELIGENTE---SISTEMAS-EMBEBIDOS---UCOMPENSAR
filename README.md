# CASA-INTELIGENTE---SISTEMAS-EMBEBIDOS---UCOMPENSAR

Descripción General
Este proyecto consiste en una plataforma de automatización domótica desarrollada mediante sistemas embebidos, reconocimiento de voz offline, arquitectura IoT y monitoreo web centralizado.

La solución implementa una arquitectura distribuida basada en múltiples microcontroladores Arduino controlados desde una aplicación central desarrollada en Python. El sistema permite gestionar diferentes zonas de una vivienda mediante comandos de voz y visualizar el estado de cada módulo desde un dashboard web en tiempo real.

El proyecto integra tecnologías de:

sistemas embebidos,
inteligencia artificial,
automatización,
comunicación serial,
interfaces web,
e Internet de las Cosas (IoT).
Objetivos del Proyecto
Objetivo General
Diseñar e implementar una plataforma domótica inteligente basada en sistemas embebidos y reconocimiento de voz para automatizar diferentes áreas de una vivienda mediante una arquitectura centralizada y escalable.

Objetivos Específicos
Implementar reconocimiento de voz offline utilizando inteligencia artificial.
Automatizar zonas específicas de una vivienda mediante actuadores físicos.
Desarrollar un dashboard web de monitoreo en tiempo real.
Integrar múltiples microcontroladores Arduino mediante comunicación serial.
Diseñar una arquitectura modular y escalable para futuras ampliaciones.
Aplicar conceptos de IoT y sistemas embebidos en un entorno domótico funcional.
Arquitectura del Sistema
La arquitectura implementada se basa en un modelo centralizado distribuido.

Python actúa como núcleo principal del sistema, encargándose de:

reconocimiento de voz,
procesamiento lógico,
gestión de estados,
comunicación serial,
y actualización del dashboard.
Los Arduinos funcionan como nodos embebidos independientes responsables de ejecutar acciones físicas sobre actuadores y dispositivos electrónicos.

Flujo General del Sistema
Usuario
   ↓
Comando de voz
   ↓
Captura de audio
   ↓
Modelo IA Vosk
   ↓
Conversión voz → texto
   ↓
Python central
   ↓
Interpretación de comandos
   ↓
Comunicación serial USB
   ↓
Arduino correspondiente
   ↓
Actuadores físicos
   ↓
Actualización dashboard web
Tecnologías Utilizadas
Software
Tecnología	Función
Python	Núcleo lógico del sistema
Flask	Servidor web y dashboard
Vosk	Reconocimiento de voz offline
HTML5	Estructura de interfaz
CSS3	Diseño visual del dashboard
PySerial	Comunicación serial con Arduino
SoundDevice	Captura de audio
JSON	Procesamiento de respuestas IA
Hardware
Componente	Función
Arduino UNO	Control embebido
Pantallas OLED SSD1306	Visualización local
Servo SG90	Automatización de persiana
LEDs RGB	Iluminación inteligente
Fuente 5V 2A	Alimentación externa
Protoboard y cableado	Integración electrónica
Reconocimiento de Voz
El sistema implementa reconocimiento de voz offline mediante Vosk.

Vosk es una librería de inteligencia artificial especializada en reconocimiento automático de voz (ASR - Automatic Speech Recognition), optimizada para funcionamiento local sin dependencia de internet.

Funcionamiento
El micrófono captura audio continuamente mediante la librería sounddevice.

Posteriormente:

El audio es procesado por Vosk.
Vosk convierte voz a texto.
Python analiza el texto detectado.
El sistema identifica comandos específicos.
Se ejecuta la acción correspondiente.
Modelo Utilizado
vosk-model-small-es-0.42
Modelo preentrenado para reconocimiento de voz en español.

Ventajas de la Solución Offline
No requiere conexión a internet.
Menor latencia.
Mayor privacidad.
Independencia de servicios externos.
Mejor compatibilidad con sistemas embebidos.
Comunicación Serial
La comunicación entre Python y Arduino se realiza mediante puertos seriales USB utilizando la librería PySerial.

Cada Arduino permanece escuchando instrucciones enviadas desde el sistema central.

Configuración
Todos los microcontroladores trabajan a:

9600 baudios
Ejemplo de Comunicación
Python
habitacion.write(b"abrirpersiana\n")
Arduino
if (comando == "abrirpersiana") {

    moverPersiana();

}
Distribución Modular
El sistema se divide en múltiples zonas independientes.

Arduino 1 - Baño Inteligente
Puerto serial:

COM5
Funcionalidades
Modo SPA
Modo MAÑANA
Modo NOCHE
Iluminación RGB
Pantalla OLED
Modos
Modo	Función
SPA	Ambiente relajante
MAÑANA	Iluminación matutina
NOCHE	Iluminación nocturna
Arduino 2 - Habitación Inteligente
Puerto serial:

COM6
Funcionalidades
Persiana automática
Iluminación
Reloj OLED
Servo motor SG90
Actuadores
Actuador	Función
Servo SG90	Apertura/cierre persiana
LEDs	Iluminación habitación
OLED	Visualización reloj
Arduino 3 - Cocina y Parqueadero
Puerto serial:

COM7
Cocina
Funciones:

Nevera
Cocina eléctrica
Extractor
Luces
Parqueadero
Funciones:

Apertura automática
Cierre automático
Dashboard Web
La interfaz gráfica fue desarrollada utilizando Flask, HTML y CSS.

El dashboard permite monitorear el estado global del sistema en tiempo real.

Características
Arquitectura web cliente-servidor.
Monitoreo centralizado.
Interfaz modular.
Actualización automática.
Diseño responsive.
Integración dinámica con Python.
Módulos Visuales
Zona	Información
Baño	Modo actual
Habitación	Luces y persiana
Cocina	Estados de dispositivos
Parqueadero	Estado acceso
Actualización Automática
El dashboard actualiza estados periódicamente mediante recarga automática HTML:

<meta http-equiv="refresh" content="2">
Problemas Encontrados y Soluciones
Problema 1 - Servo Motor
Problema
El servo SG90 presentaba:

bajo torque,
movimiento errático,
reinicios.
Causa
Insuficiente corriente desde el puerto USB del Arduino.

Solución
Implementación de:

fuente externa 5V 2A,
GND compartido,
alimentación independiente.
Problema 2 - Comunicación Serial
Problema
Bloqueo de puertos COM.

Solución
Evitar uso simultáneo de:

Arduino Serial Monitor,
Flask,
Python serial.
Problema 3 - Múltiples Pantallas OLED
Problema
Conflicto de direcciones I2C.

Solución
Separación modular utilizando múltiples Arduinos independientes.

Escalabilidad
La arquitectura fue diseñada para permitir futuras ampliaciones.

Posibles Mejoras
Sensores de temperatura.
Sensores de humedad.
Cámaras IP.
Base de datos SQLite.
Históricos de eventos.
Aplicación móvil.
MQTT.
ESP32.
Integración cloud.
Automatización avanzada.
Machine Learning.
Control remoto.
Instalación
Clonar repositorio
git clone <repositorio>
Instalar dependencias
pip install flask pyserial vosk sounddevice
Descargar modelo Vosk
Descargar:

vosk-model-small-es-0.42
y ubicarlo en la raíz del proyecto.

Ejecución
Ejecutar sistema
python app.py
Dashboard local
http://127.0.0.1:5000
Dashboard en red local
http://IP_LOCAL:5000
Estructura del Proyecto
CasaInteligente/
│
├── app.py
├── templates/
│   └── index.html
├── static/
│   └── style.css
├── vosk-model-small-es-0.42/
└── README.md
Resultados Obtenidos
El proyecto logró:

automatización funcional,
reconocimiento de voz offline,
monitoreo web,
integración embebida,
arquitectura modular,
y control distribuido de múltiples zonas.
La plataforma demostró estabilidad funcional y capacidad de expansión para futuras implementaciones IoT.

Conclusiones
El proyecto permitió integrar múltiples áreas de conocimiento:

sistemas embebidos,
inteligencia artificial,
IoT,
automatización,
electrónica,
y desarrollo web.
La solución desarrollada demuestra la viabilidad de implementar sistemas domóticos inteligentes mediante tecnologías accesibles y arquitecturas escalables.

Además, la separación modular entre:

procesamiento lógico,
reconocimiento de voz,
hardware embebido,
y monitoreo web,
facilita el mantenimiento y la expansión futura del sistema.

Autores
Karen Stefania Rivera Carrero Lina Marcela Contreras Sanabria Luis Alejandro Naranjo Garavito Carlos Alberto Castro Castillo Samuel Felipe Rojas Heredia Brandon Andres Leon Caro

Proyecto desarrollado para: Diego Alejandro Barragan Vargas

Fundación Universitaria Compensar
Ingeniería de Telecomunicaciones
Asignatura: Sistemas Embebidos

"""
Módulo de Control por Voz - Sistema Domótico
Escucha comandos de voz a través del micrófono y los transmite
vía puerto Serial a un microcontrolador.
"""
import sys
import time
import serial
import speech_recognition as sr

# --- Configuración del Sistema ---
PUERTO_SERIAL = 'COM4'
BAUD_RATE = 9600
TIEMPO_ESPERA = 1

def inicializar_conexion(puerto: str, baud_rate: int) -> serial.Serial:
    """Inicializa y retorna la conexión serial de forma segura."""
    try:
        conexion = serial.Serial(puerto, baud_rate, timeout=TIEMPO_ESPERA)
        time.sleep(2)  # Tiempo de estabilización del bootloader del Arduino
        print(f"Conexión establecida en {puerto}.")
        return conexion
    except serial.SerialException:
        print(f"Error: No se pudo abrir el puerto {puerto}. Verifique la conexión física.")
        sys.exit(1)

def procesar_comando(texto: str, conexion: serial.Serial) -> None:
    """Mapea el texto reconocido a tramas seriales y las transmite."""
    comandos = {
        "abrir parqueadero": b"G_OPEN\n",
        "cerrar parqueadero": b"G_CLOSE\n",
        "encender nevera": b"N_ON\n",
        "apagar nevera": b"N_OFF\n",
        "encender cocina": b"E_ON\n",
        "apagar cocina": b"E_OFF\n",
        "encender extractor": b"EXT_ON\n",
        "apagar extractor": b"EXT_OFF\n",
        "encender luces cocina": b"L_ON\n",
        "apagar luces cocina": b"L_OFF\n"
    }
    
    comando_ejecutado = False
    for frase, trama in comandos.items():
        if frase in texto:
            conexion.write(trama)
            print(f"Enviando orden asociada a: '{frase.upper()}'")
            comando_ejecutado = True
            break
            
        if not comando_ejecutado:
            print("Comando no registrado en la base de datos operativa.")

def escuchar_comandos(reconocedor: sr.Recognizer, conexion: serial.Serial) -> None:
    """Captura audio del micrófono y lo procesa mediante la API de Google."""
    with sr.Microphone() as source:
        print("\n--- ESPERANDO ORDEN ---")
        reconocedor.adjust_for_ambient_noise(source, duration=0.5)
        
        try:
            audio = reconocedor.listen(source)
            texto = reconocedor.recognize_google(audio, language="es-ES").lower()
            print(f"Comandante dice: '{texto}'")
            procesar_comando(texto, conexion)
            
        except sr.UnknownValueError:
            print("El sistema no pudo entender el audio.")
        except sr.RequestError:
            print("Error de conexión: Fallo en la API de reconocimiento de voz.")

def main():
    arduino = inicializar_conexion(PUERTO_SERIAL, BAUD_RATE)
    reconocedor = sr.Recognizer()
    
    try:
        while True:
            escuchar_comandos(reconocedor, arduino)
    except KeyboardInterrupt:
        print("\nSistema detenido por el usuario.")
    finally:
        if 'arduino' in locals() and arduino.is_open:
            arduino.close()
            print("Puerto serial cerrado de forma segura.")

if __name__ == "__main__":
    main()
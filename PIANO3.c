// ========================================
// PIANO DIGITAL CON PIC 16F887
// MPLAB X IDE - XC8 Compiler
// ========================================

#include <xc.h>

// Configuración de bits
#pragma config FOSC = INTRC_CLKOUT  // Oscilador interno
#pragma config WDTE = OFF            // Watchdog OFF
#pragma config PWRTE = OFF           // Power-up timer OFF
#pragma config MCLRE = ON            // MCLR pin enabled
#pragma config CP = OFF              // Code protection OFF
#pragma config CPD = OFF             // Data code protection OFF
#pragma config BOREN = ON            // Brown-out reset ON
#pragma config IESO = ON             // Internal/External Switchover ON
#pragma config FCMEN = ON            // Fail-Safe Clock Monitor ON
#pragma config LVP = OFF             // Low Voltage Programming OFF

// Definición de frecuencias (notas musicales)
#define DO      261
#define RE      293
#define MI      329
#define FA      349
#define SOL     392
#define LA      440
#define SI      493
#define DO_ALT  523

// Prototipos de funciones
void Inicializar_Sistema(void);
void Generar_Nota(unsigned int frecuencia);
void Detener_Nota(void);
void Leer_Botones(void);
void Delay_ms(unsigned int ms);

// ========== FUNCIÓN PRINCIPAL ==========
void main(void) {
    Inicializar_Sistema();
    
    while(1) {
        Leer_Botones();
        Delay_ms(10);  // Anti-rebote
    }
}

// ========== INICIALIZACIÓN DEL SISTEMA ==========
void Inicializar_Sistema(void) {
    // Configurar oscilador interno a 8 MHz
    OSCCON = 0b01110001;  // 8 MHz interno, estable
    
    // Configurar entrada/salida
    TRISB = 0xFF;         // PORTB como entrada (botones RB0-RB7)
    TRISC = 0b11111011;   // RC2 como salida (PWM), resto entrada
    
    // Inicializar puertos
    PORTB = 0;
    PORTC = 0;
    
    // Deshabilitar entrada analógica en PORTB
    ANSELH = 0x00;
    ANSEL = 0x00;
    
    // Configurar Timer2 para PWM
    // Fosc = 8 MHz, Prescaler = 1:16
    T2CON = 0b01001110;   // Timer2 ON, Prescaler 1:16, Postscaler 1:1
    PR2 = 249;            // Periodo = 10 ms (frecuencia PWM = 1 kHz)
    
    // Configurar CCP1 en modo PWM
    CCP1CON = 0b00001100; // PWM mode (bits 3:0 = 1100)
    
    // Inicializar CCPR1L
    CCPR1L = 0;           // Comienza sin sonido
    
    // Esperar a que Timer2 se inicie
    TMR2IF = 0;           // Limpiar bandera de Timer2
    while(!TMR2IF);       // Esperar a que Timer2 se reinicie
    TMR2IF = 0;           // Limpiar bandera
}

// ========== GENERAR NOTA (PWM) ==========
void Generar_Nota(unsigned int frecuencia) {
    // Fórmula: CCPR1L = (Frecuencia * PR2) / (Fosc / 4)
    // Para Fosc = 8MHz, PR2 = 249:
    // CCPR1L ? (Frecuencia * 249) / 2000000
    
    if (frecuencia == DO)      CCPR1L = 122;   // 261 Hz
    else if (frecuencia == RE) CCPR1L = 109;   // 293 Hz
    else if (frecuencia == MI) CCPR1L = 97;    // 329 Hz
    else if (frecuencia == FA) CCPR1L = 91;    // 349 Hz
    else if (frecuencia == SOL) CCPR1L = 81;   // 392 Hz
    else if (frecuencia == LA) CCPR1L = 72;    // 440 Hz
    else if (frecuencia == SI) CCPR1L = 64;    // 493 Hz
    else if (frecuencia == DO_ALT) CCPR1L = 61; // 523 Hz
}

// ========== DETENER NOTA ==========
void Detener_Nota(void) {
    CCPR1L = 0;
}

// ========== LEER BOTONES ==========
void Leer_Botones(void) {
    if (PORTBbits.RB0) {          // Botón 1 ? Do
        Generar_Nota(DO);
    }
    else if (PORTBbits.RB1) {     // Botón 2 ? Re
        Generar_Nota(RE);
    }
    else if (PORTBbits.RB2) {     // Botón 3 ? Mi
        Generar_Nota(MI);
    }
    else if (PORTBbits.RB3) {     // Botón 4 ? Fa
        Generar_Nota(FA);
    }
    else if (PORTBbits.RB4) {     // Botón 5 ? Sol
        Generar_Nota(SOL);
    }
    else if (PORTBbits.RB5) {     // Botón 6 ? La
        Generar_Nota(LA);
    }
    else if (PORTBbits.RB6) {     // Botón 7 ? Si
        Generar_Nota(SI);
    }
    else if (PORTBbits.RB7) {     // Botón 8 ? Do'
        Generar_Nota(DO_ALT);
    }
    else {
        Detener_Nota();     // Ningún botón presionado
    }
}

// ========== FUNCIÓN DELAY EN ms ==========
void Delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 123; j++);  // Calibrado para 8 MHz
}
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// 1. Configuración de Radio 
#define CE_PIN 9
#define CSN_PIN 10

// 2. Sensor JSN-SR04T (Pines definidos por el usuario)
#define SU_TRIGGER 4
#define SU_ECHO 5

// 3. Los 4 Switches (Pines definidos por el usuario)
#define SW1 2
#define SW2 6
#define SW3 7
#define SW4 8

RF24 radio(CE_PIN, CSN_PIN);
byte direccion[5] = {'c', 'a', 'u', 'n', 'a'};

// Estructura de 5 datos: 4 switches y la distancia [cite: 3, 13, 14]
struct Paquete {
    float s1;
    float s2;
    float s3;
    float s4;
    float distancia;
};

Paquete registro;

void setup() {
    Serial.begin(9600);
    
    // Configuración de los 4 switches con Pull-Up [cite: 5]
    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(SW3, INPUT_PULLUP);
    pinMode(SW4, INPUT_PULLUP);

    // Configuración de sensor en D4 y D5 [cite: 5]
    pinMode(SU_TRIGGER, OUTPUT);
    pinMode(SU_ECHO, INPUT);
    digitalWrite(SU_TRIGGER, LOW);

    // Inicialización de Radio [cite: 6]
    if (!radio.begin()) {
        Serial.println("Error: Radio no encontrada");
    }
    
    radio.setPALevel(RF24_PA_HIGH);
    radio.openWritingPipe(direccion);
    radio.stopListening();
    
    Serial.println("--- TRANSMISOR LISTO (Loop 3s) ---");
}

void loop() {
    // 1. Medir Distancia (Lógica probada por el usuario) [cite: 14, 21, 22]
    registro.distancia = devuelve_distancia();

    // 2. Leer los 4 Switches (Invertido por ser INPUT_PULLUP)
    registro.s1 = !digitalRead(SW1);
    registro.s2 = !digitalRead(SW2);
    registro.s3 = !digitalRead(SW3);
    registro.s4 = !digitalRead(SW4);

    // 3. Monitor Serial Informativo [cite: 18, 19]
    Serial.print("Dist: ");
    Serial.print(registro.distancia);
    Serial.print(" cm | SWs: ");
    Serial.print((int)registro.s1);
    Serial.print((int)registro.s2);
    Serial.print((int)registro.s3);
    Serial.print((int)registro.s4);

    // 4. Intento de Envío por Radio [cite: 15, 16]
    bool ok = radio.write(&registro, sizeof(registro));
    Serial.println(ok ? " [RADIO OK]" : " [OFFLINE]");

    // 5. Ciclo de 3 segundos solicitado [cite: 20]
    delay(3000);
}

float devuelve_distancia() {
    float d;
    long tiempo;

    // Tu secuencia de disparo que funciona [cite: 21]
    digitalWrite(SU_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(SU_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(SU_TRIGGER, LOW);

    // Medición en Pin D5 con timeout de 30ms [cite: 21]
    tiempo = pulseIn(SU_ECHO, HIGH, 30000); 

    if (tiempo == 0) return 0.00;

    // Cálculo original[cite: 22]: (tiempo/2) * 0.0343
    d = (tiempo / 2.0) * 0.0343;
    
    return d;
}
#include <SPI.h>
#include <LoRa.h>

// Pines para LoRa (Ajustar según tu conexión física)
const int csPin = 10;    // NSS
const int resetPin = 9;  // RST
const int irqPin = 2;    // DIO0

// 2. Pines del LED Bicolor (Naranja-Naranja-Marrón 330 ohm)
#define PIN_ROJO 7
#define PIN_VERDE 8
// 2. Sensor JSN-SR04T (Pines definidos por el usuario)
#define SU_TRIGGER 4
#define SU_ECHO 5

// 3. Los 4 Switches (Pines definidos por el usuario)
#define SW1 3
#define SW2 6
#define SW3 7
#define SW4 8

// RF24 radio(CE_PIN, CSN_PIN);
// byte direccion[5] = {'c', 'a', 'u', 'n', 'a'};

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
  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);
  digitalWrite(PIN_ROJO, HIGH);  // Inicia en Rojo (Standby)
  digitalWrite(PIN_VERDE, LOW);
  // Configuración de los 4 switches con Pull-Up [cite: 5]
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SU_TRIGGER, OUTPUT);
  pinMode(SU_ECHO, INPUT);
  digitalWrite(SU_TRIGGER, LOW);

  Serial.println("Iniciando LoRa Transmisor...");
  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(433E6)) {  // Debe coincidir con el receptor
    Serial.println("Error: No se encuentra el modulo LoRa");
    while (1)
      ;
  }
  LoRa.setSyncWord(0xF3);
  LoRa.enableCrc();
  Serial.println("--- TRANSMISOR LORA LISTO ---");
}

void loop() {
  // 1. Medir Distancia (Lógica probada por el usuario) [cite: 14, 21, 22]
  registro.distancia = devuelve_distancia();
  registro.s1 = !digitalRead(SW1);
  registro.s2 = !digitalRead(SW2);
  registro.s3 = !digitalRead(SW3);
  registro.s4 = !digitalRead(SW4);
  Serial.print("Enviando Dist: ");
  Serial.print(registro.distancia);
  Serial.print(" cm | SWs: ");
  Serial.print((int)registro.s1);
  Serial.print((int)registro.s2);
  Serial.print((int)registro.s3);
  Serial.print((int)registro.s4);


  // Envío del paquete binario (Estructura)
  LoRa.beginPacket();
  LoRa.write((uint8_t *)&registro, sizeof(registro));
  if (LoRa.endPacket()) {
    // --- ÉXITO: Destello VERDE ---
    digitalWrite(PIN_VERDE, HIGH);
    digitalWrite(PIN_ROJO, LOW);
    Serial.println(F(" [ENVÍO OK]"));

    delay(300);  // Duración del destello verde
  } else {
    // --- FALLO: Destello ROJO ---
    digitalWrite(PIN_VERDE, LOW);
    digitalWrite(PIN_ROJO, HIGH);
    Serial.println(F(" [FALLO DE HARDWARE EN ENVIO]"));

    delay(1000);  // Duración del aviso rojo
  }
  digitalWrite(PIN_VERDE, LOW);
  digitalWrite(PIN_ROJO, LOW);
  Serial.println(" [ENVIADO]");
  delay(3000);
}

float devuelve_distancia() {
  digitalWrite(SU_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(SU_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(SU_TRIGGER, LOW);
  long tiempo = pulseIn(SU_ECHO, HIGH, 30000);
  if (tiempo == 0) return 0.00;
  return (tiempo / 2.0) * 0.0343;
}
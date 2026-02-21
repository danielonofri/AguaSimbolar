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

struct __attribute__((packed)) LoRaPayload {
  float dist;  // 4 bytes
  byte p_in;   // 1 byte
  byte p_out;  // 1 byte
};

// --- VARIABLES GLOBALES DE COMUNICACIÓN ---
LoRaPayload data;                    // Estructura principal de datos [cite: 72]
volatile byte comandoRecibido = 0;   // Almacena el byte p_out recibido
volatile bool nuevoComando = false;  // Bandera para la interrupción

void setup() {
  Serial.begin(9600);
  configurarHardware();

  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);
  digitalWrite(PIN_ROJO, HIGH);  // Inicia en Rojo (Standby)
  digitalWrite(PIN_VERDE, LOW);
  // Configuración de los 4 switches con Pull-Up [cite: 5]

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

  // 1. Si la interrupción detectó un comando nuevo, lo aplicamos
  if (nuevoComando) {
    escribirSalidas(comandoRecibido);
    nuevoComando = false;
    Serial.print(F("[LoRa] Nuevo comando aplicado: "));
    Serial.println(comandoRecibido);
  }

  // 2. Envío periódico de datos (Reporte)
  static unsigned long tEnvio = 0;
  if (millis() - tEnvio > 5000) {  // Enviamos cada 5 segundos
    // --- CORRECCIÓN EN EL LOOP ---
    LoRaPayload reporte;
    reporte.dist = devuelve_distancia();  // Obtiene la distancia real
    reporte.p_in = leerEntradas();
    reporte.p_out = 0;

    Serial.print("Enviando Dist: ");
    Serial.print(reporte.dist);  // <--- CAMBIAR 'data.dist' POR 'reporte.dist'
    Serial.print(" cm | SWs: ");
    Serial.println(reporte.p_in);
    Serial.print(reporte.p_out);
    // --- DENTRO DEL IF DE ENVÍO ---
    digitalWrite(PIN_VERDE, HIGH);  // Encendemos verde al empezar a enviar
    LoRa.beginPacket();
    LoRa.write((uint8_t *)&reporte, sizeof(reporte));
    if (LoRa.endPacket()) {
      // --- ÉXITO: Destello VERDE ---
      delay(100);
      digitalWrite(PIN_VERDE, LOW);  // Apagamos verde, envío terminado
      Serial.println(F(" [ENVÍO OK]"));

      delay(300);  // Duración del destello verde
    } else {
      // --- FALLO: Destello ROJO ---
      digitalWrite(PIN_VERDE, LOW);
      digitalWrite(PIN_ROJO, HIGH);  // Error de radio
      delay(500);
      digitalWrite(PIN_ROJO, LOW);

      delay(1000);  // Duración del aviso rojo
    }

    // MUY IMPORTANTE: Volver a modo recepción después de enviar
    LoRa.receive();
    tEnvio = millis();
  }
}

float devuelve_distancia() {
  digitalWrite(SU_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(SU_TRIGGER, HIGH);
  delayMicroseconds(20);
  digitalWrite(SU_TRIGGER, LOW);
  long tiempo = pulseIn(SU_ECHO, HIGH, 100000);
  if (tiempo == 0) return 0.00;
  return (tiempo / 2.0) * 0.0343;
}
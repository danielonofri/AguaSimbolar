// --- CONFIGURACIÓN DE PINES ---
// Entradas (p_in): Usamos los 4 primeros analógicos como digitales
#define PIN_IN_1 A0
#define PIN_IN_2 A1
#define PIN_IN_3 A2
#define PIN_IN_4 A3

// Salidas (p_out): Usamos los 2 analógicos restantes + 2 digitales seguros
#define PIN_OUT_1 A4
#define PIN_OUT_2 A5
#define PIN_OUT_3 7   // Pin Digital 7 (Libre de SPI)
#define PIN_OUT_4 8   // Pin Digital 8 (Libre de SPI)

void configurarHardware() {
  // Configuración de Entradas con Pull-Up interno
  // Esto permite conectar botones directamente a GND
  pinMode(PIN_IN_1, INPUT_PULLUP);
  pinMode(PIN_IN_2, INPUT_PULLUP);
  pinMode(PIN_IN_3, INPUT_PULLUP);
  pinMode(PIN_IN_4, INPUT_PULLUP);

  // Configuración de Salidas
  pinMode(PIN_OUT_1, OUTPUT);
  pinMode(PIN_OUT_2, OUTPUT);
  pinMode(PIN_OUT_3, OUTPUT);
  pinMode(PIN_OUT_4, OUTPUT);

  // Estado inicial: Todo apagado
  digitalWrite(PIN_OUT_1, LOW);
  digitalWrite(PIN_OUT_2, LOW);
  digitalWrite(PIN_OUT_3, LOW);
  digitalWrite(PIN_OUT_4, LOW);
}

// Función para empaquetar 4 entradas en un solo byte (Bitmasking)
byte leerEntradas() {
  byte resultado = 0;
  
  // Si el pin está en LOW (presionado), ponemos el bit correspondiente en 1
  if (digitalRead(PIN_IN_1) == LOW) resultado |= (1 << 0); // Bit 0
  if (digitalRead(PIN_IN_2) == LOW) resultado |= (1 << 1); // Bit 1
  if (digitalRead(PIN_IN_3) == LOW) resultado |= (1 << 2); // Bit 2
  if (digitalRead(PIN_IN_4) == LOW) resultado |= (1 << 3); // Bit 3
  
  return resultado;
}

// Función para desempaquetar el byte de la API y actuar sobre las salidas
void escribirSalidas(byte comando) {
  // Verificamos cada bit del byte 'comando'
  digitalWrite(PIN_OUT_1, (comando & (1 << 0)) ? HIGH : LOW);
  digitalWrite(PIN_OUT_2, (comando & (1 << 1)) ? HIGH : LOW);
  digitalWrite(PIN_OUT_3, (comando & (1 << 2)) ? HIGH : LOW);
  digitalWrite(PIN_OUT_4, (comando & (1 << 3)) ? HIGH : LOW);
}
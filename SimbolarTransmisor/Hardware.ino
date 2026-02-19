// --- CONFIGURACIÓN DE PINES (Mapeo numérico para Arduino Uno) ---
// Entradas (p_in)
#define PIN_IN_1 14 // Equivale a A0 
#define PIN_IN_2 15 // Equivale a A1 
#define PIN_IN_3 16 // Equivale a A2 
#define PIN_IN_4 17 // Equivale a A3 

// Salidas (p_out)
#define PIN_OUT_1 18 // Equivale a A4 
#define PIN_OUT_2 19 // Equivale a A5 
#define PIN_OUT_3 7  // Pin Digital 7 [cite: 69]
#define PIN_OUT_4 8  // Pin Digital 8 [cite: 69]

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
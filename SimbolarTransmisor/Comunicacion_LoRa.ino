// Función de Interrupción (ISR) - Se ejecuta al recibir un paquete
void onReceive(int packetSize) {
  if (packetSize == sizeof(LoRaPayload)) {
    LoRaPayload entrada;
    LoRa.readBytes((uint8_t *)&entrada, sizeof(entrada));
    
    // Solo nos interesa el byte p_out que viene del Receptor
    comandoRecibido = entrada.p_out;
    nuevoComando = true;
  }
}

void configurarLoRaAsincrono() {
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    while (1); // Error de hardware
  }
  LoRa.setSyncWord(0xF3);
  LoRa.enableCrc();

  // Registrar la función que se ejecutará al recibir datos
  LoRa.onReceive(onReceive);
  // Poner el módulo en modo audición continua
  LoRa.receive(); 
}
void gestionarBotonMultifuncion() {
  static unsigned long tPresionado = 0;
  static bool ultimoEstado = HIGH;
  bool estadoActual = digitalRead(PIN_BOTON_MULTIFUNCION);

  // Al presionar (LOW)
  if (ultimoEstado == HIGH && estadoActual == LOW) {
    tPresionado = millis();
  }

  // Al soltar (HIGH)
  else if (ultimoEstado == LOW && estadoActual == HIGH) {
    unsigned long duracion = millis() - tPresionado;

    // CLIC CORTO (menos de 2 segundos): Toggle LCD
    if (duracion > 50 && duracion < 2000) {
      lcdEncendido = !lcdEncendido;
      if (lcdEncendido) lcd.backlight();
      else lcd.noBacklight();
      Serial.println(F("[BOTÓN] LCD Conmutado"));
    }
    tPresionado = 0;
  }

  // CLIC LARGO (10 segundos): RESET TOTAL
  if (estadoActual == LOW && (millis() - tPresionado > 10000)) {
    Serial.println(F("[BOTÓN] !!! INICIANDO RESET DE FÁBRICA !!!"));
    lcd.clear();
    lcd.backlight();
    lcd.print(F("Borrando todo..."));

    wm.resetSettings();               // Borra WiFi
    LittleFS.remove("/config.json");  // Borra parámetros de la API

    delay(2000);
    ESP.restart();
  }
  ultimoEstado = estadoActual;
}
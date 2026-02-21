void gestionarBotonMultifuncion() {
  static unsigned long tPresionado = 0;
  static bool ultimoEstado = HIGH;
  static bool resetEjecutado = false; // Evita que el reset se dispare mil veces
  
  bool estadoActual = digitalRead(PIN_BOTON_MULTIFUNCION);

  // 1. DETECTAR EL MOMENTO DEL CLIC (FLANCO DE BAJADA)
  if (ultimoEstado == HIGH && estadoActual == LOW) {
    tPresionado = millis();
    resetEjecutado = false;
    Serial.println(F("[BOTÓN] Presionado..."));
  }

  // 2. DETECTAR ACCIÓN AL SOLTAR (FLANCO DE SUBIDA)
  else if (ultimoEstado == LOW && estadoActual == HIGH) {
    unsigned long duracion = millis() - tPresionado;

    // Solo procesamos si no se ejecutó el reset de 15s mientras estaba presionado
    if (!resetEjecutado) {
      
      // ACCIÓN 1: CLIC CORTO (Menos de 1.5 segundos) -> ROTAR PANTALLA
      if (duracion > 50 && duracion < 1500) {
        pantallaActual++;
        if (pantallaActual > 3) pantallaActual = 0; // Ciclo entre 3 pantallas
        actualizarPantallaInmediato(); // Función para refrescar el LCD rápido
        Serial.print(F("[BOTÓN] Rotar Pantalla: ")); Serial.println(pantallaActual);
      }
      
      // ACCIÓN 2: PULSACIÓN MEDIA (Entre 5 y 10 segundos) -> TOGGLE LCD
      else if (duracion >= 5000 && duracion < 10000) {
        lcdEncendido = !lcdEncendido;
        if (lcdEncendido) {
          lcd.backlight();
          actualizarPantallaInmediato(); // Redibuja al encender
        } else {
          lcd.clear();
          lcd.noBacklight();
        }
        Serial.print(F("[BOTÓN] LCD: ")); Serial.println(lcdEncendido ? "ON" : "OFF");
      }
    }
    tPresionado = 0; // Resetear cronómetro al soltar
  }

  // 3. ACCIÓN 3: PULSACIÓN CRÍTICA (15 segundos) -> RESET TOTAL
  // Se ejecuta mientras aún está presionado para dar feedback visual
  if (estadoActual == LOW && !resetEjecutado) {
    unsigned long tiempoAbajo = millis() - tPresionado;

    if (tiempoAbajo >= 15000) {
      resetEjecutado = true;
      Serial.println(F("[BOTÓN] !!! INICIANDO RESET DE FÁBRICA !!!"));
      
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print(F("BORRANDO TODO..."));
      lcd.setCursor(0,1);
      lcd.print(F("REINICIANDO..."));

      wm.resetSettings();               // Borra WiFi
      LittleFS.remove("/config.json");  // Borra parámetros locales
      
      delay(2000); // Único delay aceptable antes del reinicio
      ESP.restart();
    }
  }

  ultimoEstado = estadoActual;
}
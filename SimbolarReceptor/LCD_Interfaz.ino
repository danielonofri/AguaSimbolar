void rotarPantallas() {
  if (!lcdEncendido) return;

  // Auto-scroll cada 5 segundos
  if (millis() - tScroll > 5000) { 
    pantallaActual = (pantallaActual + 1) % 4;
    actualizarPantallaInmediato();
  }
}

void actualizarPantallaInmediato() {
  if (!lcdEncendido) return;
  
  lcd.clear();
  tScroll = millis(); // Reiniciamos el contador para que el auto-scroll no salte justo después

  switch (pantallaActual) { // <--- Cambiado a pantallaActual
    case 0: // Nivel y Altura
      lcd.print(F("Nivel: "));
      lcd.print(porcentaje);
      lcd.print(F("%"));
      lcd.setCursor(0, 1);
      lcd.print(F("Altura: "));
      lcd.print(altura_agua);
      lcd.print(F("cm"));
      break;

    case 1: // Configuración local
      lcd.print(F("Conf Max: "));
      lcd.print(tank_h);
      lcd.setCursor(0, 1);
      lcd.print(F("Conf Min: "));
      lcd.print(sensor_m);
      break;

    case 2: // Estado de los 4 Pines del Transmisor
      lcd.print(F("R1:")); lcd.write(sw_remotos[1] ? 0 : 1);
      lcd.print(F(" R2:")); lcd.write(sw_remotos[2] ? 0 : 1);
      lcd.setCursor(0, 1);
      lcd.print(F("R3:")); lcd.write(sw_remotos[3] ? 0 : 1);
      lcd.print(F(" R4:")); lcd.write(sw_remotos[4] ? 0 : 1);
      break;

    case 3: // WiFi, API y Hora
      lcd.print(F("WiFi:"));
      lcd.write(WiFi.status() == WL_CONNECTED ? 0 : 1); 
      lcd.print(F(" API:"));
      if (apiStatus == 1)      lcd.write(0); 
      else if (apiStatus == 2) lcd.write(2); 
      else                     lcd.write(1); 
      lcd.setCursor(0, 1);
      lcd.print(obtenerHora());
      break;
  }
}

String obtenerHora() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  if (p_tm->tm_year < 100) return "Sincronizando...";
  char buf[12];
  sprintf(buf, "%02d:%02d:%02d", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
  return String(buf);
}
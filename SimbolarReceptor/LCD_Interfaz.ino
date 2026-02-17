void rotarPantallas() {
  if (millis() - tScroll > 3500) {
    if (!lcdEncendido) return;
    lcd.clear();
    switch (pantalla) {
      case 0:
        lcd.print(F("Nivel: "));
        lcd.print(porcentaje);
        lcd.print(F("%"));
        lcd.setCursor(0, 1);
        lcd.print(F("Altura: "));
        lcd.print(altura_agua);
        lcd.print(F("cm"));
        break;
      case 1:
        lcd.print(F("Config Max: "));
        lcd.print(tank_h);
        lcd.setCursor(0, 1);
        lcd.print(F("Config Min: "));
        lcd.print(sensor_m);
        break;
      case 2:
        lcd.print(F("Salto Max: "));
        lcd.print(delta_max);
        lcd.setCursor(0, 1);
        lcd.print(F("Filtro Activo"));
        break;
      case 3: // Nueva pantalla de Relés
        lcd.print(F("R1:")); lcd.write(sw_remotos[1] ? 0 : 1); // Check o Cross [cite: 11, 12, 71]
        lcd.print(F(" R2:")); lcd.write(sw_remotos[2] ? 0 : 1);
        lcd.setCursor(0, 1);
        lcd.print(F("R3:")); lcd.write(sw_remotos[3] ? 0 : 1);
        lcd.print(F(" R4:")); lcd.write(sw_remotos[4] ? 0 : 1);
        break;

      case 4: // WiFi y API (Antes era case 3)
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
    pantalla = (pantalla + 1) % 5;
    tScroll = millis();
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
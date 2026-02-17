void cargarConfiguracion() {
  if (LittleFS.begin()) {
    if (LittleFS.exists("/config.json")) {
      File f = LittleFS.open("/config.json", "r");
      if (f) {
        JsonDocument doc;
        deserializeJson(doc, f);
        strncpy(tank_h, doc["h"] | "220", sizeof(tank_h));
        strncpy(sensor_m, doc["m"] | "20", sizeof(sensor_m));
        strncpy(tank_delta, doc["d"] | "15", sizeof(tank_delta));
        delta_max = atoi(tank_delta);
        strncpy(api_url, doc["u"] | "http://45.234.117.236:54625/api/Sensores", sizeof(api_url));
        f.close();
      }
    }
  }
}

void procesarGuardadoConfig() {
  if (shouldSaveConfig) {
    Serial.println(F("Guardando nueva configuración..."));
    strncpy(tank_h, c_h.getValue(), sizeof(tank_h));
    strncpy(sensor_m, c_m.getValue(), sizeof(sensor_m));
    strncpy(tank_delta, c_d.getValue(), sizeof(tank_delta));
    strncpy(api_url, c_u.getValue(), sizeof(api_url));
    delta_max = atoi(tank_delta);

    JsonDocument doc;
    doc["h"] = tank_h;
    doc["m"] = sensor_m;
    doc["d"] = tank_delta;
    doc["u"] = api_url;
    File f = LittleFS.open("/config.json", "w");
    if (f) {
      serializeJson(doc, f);
      f.close();
    }
    shouldSaveConfig = false;
  }
}

void gestionarSerial() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "force_reset") {
      Serial.println(F("[SERIAL] Reset de WiFi y Configuración..."));
      wm.resetSettings();
      LittleFS.remove("/config.json");
      delay(1000);
      ESP.restart();
    } else if (cmd == "lcd_on") {
      lcdEncendido = true;
      lcd.backlight();
      Serial.println(F("[SERIAL] LCD Encendido"));
    } else if (cmd == "lcd_off") {
      lcdEncendido = false;
      lcd.noBacklight();
      Serial.println(F("[SERIAL] LCD Apagado"));
    }
  }
}
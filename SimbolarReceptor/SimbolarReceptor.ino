#include <LittleFS.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

// --- Hardware ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define PIN_RESET D4        // Pulsador Reset (D4 a GND)
#define PIN_TOGGLE_LCD D3   // Pulsador LCD (D3 a GND)

// --- Variables Globales ---
char tank_delta[5] = "15"; // Valor por defecto: 15cm
int delta_max = 15;
int distanciaAnterior = 0;
char tank_h[6] = "220", sensor_m[6] = "20";
char api_url[100] = "http://45.234.117.236:54625/api/Sensores";
bool shouldSaveConfig = false;
int distancia = 0, porcentaje = 0, pantalla = 0, altura_agua = 0;
bool sw_remotos[5] = {false};
bool apiOk = false;
bool lcdEncendido = true; // Control de estado del display
unsigned long tScroll = 0, tApi = 0;
int lecturasEstables = 0; // Contador para el arranque
const int LECTURAS_PARA_ARRANQUE = 5;
// Iconos ✔ y ❌
byte check[8] = {0x00,0x01,0x03,0x16,0x1C,0x08,0x00,0x00};
byte cross[8] = {0x00,0x1B,0x0E,0x04,0x0E,0x1B,0x00,0x00};

// --- Prototipos de Funciones ---
void gestionarResets();
void gestionarLCD();
void ejecutarAPI();
void rotarPantallas();
String obtenerHora();
void saveConfigCallback() { shouldSaveConfig = true; }

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RESET, INPUT_PULLUP);
  pinMode(PIN_TOGGLE_LCD, INPUT_PULLUP);
  
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, check);
  lcd.createChar(1, cross);

  // 1. Cargar Configuración de LittleFS
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
        strncpy(api_url, doc["u"] | "http://...", sizeof(api_url));
        f.close();
      }
    }
  }

  // 2. WiFiManager
  WiFiManager wm;
  wm.setSaveConfigCallback(saveConfigCallback);
  IPAddress _ip(10,0,0,1);
  wm.setAPStaticIPConfig(_ip, _ip, IPAddress(255,255,255,0));

  WiFiManagerParameter c_h("h", "Altura", tank_h, 5);
  WiFiManagerParameter c_m("m", "Margen", sensor_m, 5);
  WiFiManagerParameter c_d("d", "Delta Max (cm)", tank_delta, 5);
  WiFiManagerParameter c_u("u", "API URL", api_url, 100);
  wm.addParameter(&c_h); wm.addParameter(&c_m); wm.addParameter(&c_d); wm.addParameter(&c_u);

  lcd.print(F("AP: AguaSimbolar"));
  if (!wm.autoConnect("AguaSimbolar_AP")) ESP.restart();

  // 3. Configurar Hora (Argentina UTC-3)
  configTime(-3 * 3600, 0, "ar.pool.ntp.org", "time.google.com");

  // 4. Guardar si hubo cambios
  if (shouldSaveConfig) {
    strncpy(tank_h, c_h.getValue(), sizeof(tank_h));
    strncpy(sensor_m, c_m.getValue(), sizeof(sensor_m));
    strncpy(tank_delta, c_d.getValue(), sizeof(tank_delta));
    delta_max = atoi(tank_delta);
    strncpy(api_url, c_u.getValue(), sizeof(api_url));
    JsonDocument doc;
    doc["h"] = tank_h; doc["m"] = sensor_m; doc["d"] = tank_delta; doc["u"] = api_url;
    File f = LittleFS.open("/config.json", "w");
    serializeJson(doc, f);
    f.close();
  }
  lcd.clear();
}

void loop() {
  gestionarResets(); 
  gestionarLCD();    
  ejecutarAPI();     
  rotarPantallas();  
}

// --- Lógica de Resets y Comandos Serial ---
void gestionarResets() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "force_reset") {
      WiFiManager wm;
      wm.resetSettings();
      ESP.restart();
    } 
    else if (cmd == "lcd_off") {
      lcdEncendido = false;
      lcd.noBacklight();
      Serial.println(F("LCD: OFF (Serial)"));
    } 
    else if (cmd == "lcd_on") {
      lcdEncendido = true;
      lcd.backlight();
      Serial.println(F("LCD: ON (Serial)"));
    }
  }

  static unsigned long tPresionado = 0; 
  if (digitalRead(PIN_RESET) == LOW) {
    if (tPresionado == 0) tPresionado = millis();
    if (millis() - tPresionado > 3000) { 
      WiFiManager wm;
      wm.resetSettings();
      LittleFS.remove("/config.json");
      ESP.restart();
    }
  } else {
    tPresionado = 0;
  }
}

// --- Lógica del Pulsador del LCD ---
void gestionarLCD() {
  // Guardamos el estado anterior para detectar el momento justo del click
  static bool ultimoEstadoBoton = HIGH; 
  bool estadoActual = digitalRead(PIN_TOGGLE_LCD);

  // ¿El botón pasó de NO presionado (HIGH) a PRESIONADO (LOW)?
  if (ultimoEstadoBoton == HIGH && estadoActual == LOW) {
    
    delay(50); // Anti-rebote: esperamos a que la vibración del metal pase
    
    // Si después de 50ms sigue presionado, es un click real
    if (digitalRead(PIN_TOGGLE_LCD) == LOW) {
      lcdEncendido = !lcdEncendido; // Cambiamos el estado (Toggle)

      if (lcdEncendido) {
        lcd.backlight();
        Serial.println(F("Botón Físico: LCD ON"));
      } else {
        lcd.noBacklight();
        Serial.println(F("Botón Físico: LCD OFF"));
      }
    }
  }
  
  // Guardamos el estado para la próxima vuelta del loop
  ultimoEstadoBoton = estadoActual;
}
// void gestionarLCD() {
//   static bool ultimoEstadoBoton = HIGH;
//   bool estadoActual = digitalRead(PIN_TOGGLE_LCD);

//   if (ultimoEstadoBoton == HIGH && estadoActual == LOW) {
//     delay(50); // Debounce
//     lcdEncendido = !lcdEncendido;
//     if (lcdEncendido) lcd.backlight(); else lcd.noBacklight();
//     Serial.print(F("LCD Toggle: ")); Serial.println(lcdEncendido ? "ON" : "OFF");
//   }
//   ultimoEstadoBoton = estadoActual;
// }

// --- Comunicación con la API ---
void ejecutarAPI() {
  if (millis() - tApi > 10000) {
    int fondo = atoi(tank_h);   
    int margen = atoi(sensor_m); 
    int lecturaCruda = random(margen, fondo);

    distancia = aplicarFiltroDelta(lecturaCruda);
    
    altura_agua = fondo - distancia; 
    porcentaje = map(distancia, fondo, margen, 0, 100);
    porcentaje = constrain(porcentaje, 0, 100);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      
      // POST Datos
      http.begin(client, api_url);
      http.addHeader(F("Content-Type"), F("application/json"));
      JsonDocument doc;
      doc["distancia"] = distancia;
      doc["porcentaje"] = porcentaje;
      doc["tank_h"] = atoi(tank_h);     // Valor del WiFiManager
      doc["sensor_m"] = atoi(sensor_m); // Valor del WiFiManager
      doc["delta_max"] = delta_max;     // Valor del WiFiManager
      String payload;
      serializeJson(doc, payload);
      apiOk = (http.POST(payload) >= 200);
      http.end();

      // GET Comandos
      http.begin(client, String(api_url) + "/comandos");
      if (http.GET() == 200) {
        String res = http.getString();
        JsonDocument cmdDoc;
        deserializeJson(cmdDoc, res);
        sw_remotos[1] = cmdDoc["relay1ON"];
        sw_remotos[2] = cmdDoc["relay2ON"];
        sw_remotos[3] = cmdDoc["relay3ON"];
        sw_remotos[4] = cmdDoc["relay4ON"];
        // 2. Control del LCD desde la API
        // Verificamos si el estado enviado es distinto al actual para evitar parpadeos
        bool estadoApiLcd = cmdDoc["lcd"] | true; // Por defecto true si no viene
        if (estadoApiLcd != lcdEncendido) {
          lcdEncendido = estadoApiLcd;
          if (lcdEncendido) lcd.backlight(); else lcd.noBacklight();
          Serial.print(F("API: LCD cambiado a ")); Serial.println(lcdEncendido ? "ON" : "OFF");
        }
      }
      http.end();
    }
    tApi = millis();
  }
}


void rotarPantallas() {
  if (millis() - tScroll > 3500) {
    if (!lcdEncendido) return; 

    lcd.clear();
    switch(pantalla) {
      case 0: 
        lcd.print(F("Llenado: ")); lcd.print(porcentaje); lcd.print(F("%"));
        lcd.setCursor(0,1); lcd.print(F("Altura: ")); lcd.print(altura_agua); lcd.print(F("cm"));
        break;
      case 1:
        lcd.print(F("Maximo: ")); lcd.print(tank_h); lcd.print(F("cm"));
        lcd.setCursor(0,1); lcd.print(F("Margen: ")); lcd.print(sensor_m); lcd.print(F("cm"));
        break;
      case 2:
        lcd.print(F("Salto Max: ")); lcd.print(delta_max); lcd.print(F("cm"));
        lcd.setCursor(0,1); lcd.print(F("(Max permitido)"));
        break;
      case 3:
        lcd.print(F("Rele1:")); lcd.write(sw_remotos[1]?0:1);
        lcd.print(F(" Rele2:")); lcd.write(sw_remotos[2]?0:1);
        lcd.setCursor(0,1);
        lcd.print(F("Rele3:")); lcd.write(sw_remotos[3]?0:1);
        lcd.print(F(" Rele4:")); lcd.write(sw_remotos[4]?0:1);
        break;
      case 4:
        lcd.print(F("WiFi:")); lcd.write(WiFi.status()==WL_CONNECTED?0:1);
        lcd.setCursor(0,1);
        lcd.print(F("API:")); lcd.write(apiOk?0:1); lcd.print(F("   ")); lcd.print(obtenerHora());
        break;
      case 5:
        lcd.print(F("lcd_on - lcd_off")); 
        lcd.setCursor(0,1);
        lcd.print(F("force_reset")); 
        break;
    }
    pantalla = (pantalla + 1) % 6; // Actualizado a 6 pantallas
    tScroll = millis();
  }
}
// // --- Interfaz de Usuario ---
// void rotarPantallas() {
//   if (millis() - tScroll > 3500) {
//     if (!lcdEncendido) return; 

//     lcd.clear();
//     switch(pantalla) {
//       case 0: 
//         lcd.print(F("Llenado: ")); lcd.print(porcentaje); lcd.print(F("%"));
//         lcd.setCursor(0,1); lcd.print(F("Altura: ")); lcd.print(altura_agua); lcd.print(F("cm"));
//         break;
//       case 1:
//         lcd.print(F("Maximo: ")); lcd.print(tank_h); lcd.print(F("cm"));
//         lcd.setCursor(0,1); lcd.print(F("Margen: ")); lcd.print(sensor_m);lcd.print(F("cm"));
//         break;
//       case 2:
//          lcd.print(F("Rele1:"));lcd.write(sw_remotos[1]?0:1);
//         lcd.print(F(" Rele2:")); lcd.write(sw_remotos[2]?0:1);
//         lcd.setCursor(0,1);
//         lcd.print(F("Rele3:")); lcd.write(sw_remotos[3]?0:1);
//         lcd.print(F(" Rele4:")); lcd.write(sw_remotos[4]?0:1);
//         break;
//       case 3:
//         lcd.print(F("WiFi:")); lcd.write(WiFi.status()==WL_CONNECTED?0:1);
//         lcd.setCursor(0,1);
//         lcd.print(F("API:")); lcd.write(apiOk?0:1); lcd.print(F("   ")); lcd.print(obtenerHora());
//         break;
//       case 4:
//         lcd.print(F("lcd_on - lcd_off")); 
//         lcd.setCursor(0,1);
//         lcd.print(F("force_reset")); 
//         break;
//     }
//     pantalla = (pantalla + 1) % 5;
//     tScroll = millis();
//   }
// }

String obtenerHora() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  if (p_tm->tm_year < 100) return "Sincronizando...";
  char buf[12];
  sprintf(buf, "%02d:%02d:%02d", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
  return String(buf);
}

int aplicarFiltroDelta(int lecturaNueva) {
  // --- FASE DE ARRANQUE ---
  if (distanciaAnterior == 0) {
    // Si es la primera o estamos validando el inicio
    if (lecturasEstables == 0) {
      distanciaAnterior = lecturaNueva;
      lecturasEstables = 1;
      return lecturaNueva;
    }
    
    // Comparamos la nueva lectura con la que estamos validando
    if (abs(lecturaNueva - distanciaAnterior) < delta_max) {
      lecturasEstables++;
    } else {
      // Si son muy distintas, reseteamos porque la anterior pudo ser un error
      distanciaAnterior = lecturaNueva;
      lecturasEstables = 1;
      Serial.println(F("Reintentando validar lectura inicial..."));
    }

    if (lecturasEstables < LECTURAS_PARA_ARRANQUE) {
      return 0; // Todavía no es un valor confiable, no mostramos nada
    }
  }

  // --- FASE DE OPERACIÓN NORMAL ---
  int diferencia = abs(lecturaNueva - distanciaAnterior);

  if (diferencia > delta_max) {
    Serial.println(F("Salto brusco detectado. Ignorando..."));
    return distanciaAnterior; 
  }

  distanciaAnterior = lecturaNueva;
  return lecturaNueva;
}

// int aplicarFiltroDelta(int lecturaNueva) {
//   // Si es la primera vez, inicializamos
//   if (distanciaAnterior == 0) {
//     distanciaAnterior = lecturaNueva;
//     return lecturaNueva;
//   }

//   // Calculamos la diferencia
//   int diferencia = abs(lecturaNueva - distanciaAnterior);

//   // Si el salto es mayor al permitido, desconfiamos
//   if (diferencia > delta_max) {
//     Serial.print(F("Salto detectado: ")); Serial.print(diferencia);
//     Serial.println(F("cm. Ignorando..."));
//     return distanciaAnterior; // Mantenemos el último valor estable
//   }

//   distanciaAnterior = lecturaNueva;
//   return lecturaNueva;
// }
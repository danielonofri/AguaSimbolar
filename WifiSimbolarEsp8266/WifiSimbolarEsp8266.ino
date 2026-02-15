#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Ali";        // tu red WiFi
const char* password = "tanguito2";
const char* serverUrl = "http://45.234.117.236:54625/api/Sensores";

void setup() {
  Serial.begin(9600); // mismo baudrate que el Uno
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi!");
}

void loop() {
  if (Serial.available()) {
    String payload = Serial.readStringUntil('\n');
    payload.trim();

    if (payload.startsWith("{") && payload.endsWith("}")) {
      Serial.println("Recibido del Uno: " + payload);

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;          // nuevo
        HTTPClient http;
        http.begin(client, serverUrl);   // usar cliente + URL
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);

        if (httpCode > 0) {
          Serial.printf("Respuesta HTTP: %d\n", httpCode);
          String respuesta = http.getString();
          Serial.println("Servidor dice: " + respuesta);
        } else {
          Serial.printf("Error en POST: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
      } else {
        Serial.println("WiFi desconectado");
      }
    }
  }
}
void ejecutarAPI() {
  // 1. Verificación de tiempo y conexión física
  if (millis() - tApi < 10000) return; 
  tApi = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[API] Error: WiFi desconectado"));
    apiStatus = 0; // ❌
    return;
  }

  // 2. Preparación del Cliente
  WiFiClient client;
  HTTPClient http;
  
  //Serial.print(F("[API] Conectando a: "));
  //Serial.println(api_url);

  // 3. Inicio de la petición
  if (http.begin(client, api_url)) {
    http.addHeader("Content-Type", "application/json");

    // 4. Construcción del JSON
    JsonDocument doc; // Usamos tamaño fijo para evitar fragmentación
    doc["distancia"] = distancia;
    doc["tank_h"] = atoi(tank_h);
    doc["sensor_m"] = atoi(sensor_m);
    doc["delta_max"] = delta_max;
    doc["boton1"] = sw_remotos[1] ? "1" : "0";
    doc["boton2"] = sw_remotos[2] ? "1" : "0";
    doc["boton3"] = sw_remotos[3] ? "1" : "0";
    doc["boton4"] = sw_remotos[4] ? "1" : "0";

    String payload;
    serializeJson(doc, payload);

    // 5. Envío y captura de respuesta
    int httpCode = http.POST(payload);

    if (httpCode > 0) {
      Serial.printf("[API] Código HTTP: %d\n", httpCode);
      
      if (httpCode == HTTP_CODE_OK || httpCode == 201) {
        apiStatus = 1; // ✔️
      } else {
        apiStatus = 2; // ⚠️ (El servidor respondió pero rechazó el dato)
        Serial.printf("⚠️");
        Serial.println(http.getString()); 
      }
    } else {
      apiStatus = 0; // ❌
      Serial.printf("[API] ❌: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    apiStatus = 0; // ❌
    Serial.println(F("[API] No se pudo establecer conexión con el host."));
  }
}

// #include <ESP8266HTTPClient.h>
// #include <ArduinoJson.h>

// void ejecutarAPI() {
//   if (millis() - tApi > 10000) {
//     if (distancia > 0) {
//       WiFiClient client;
//       HTTPClient http;

//       Serial.println(F("\n[API] Enviando datos..."));
//       http.begin(client, api_url);
//       http.addHeader("Content-Type", "application/json");

//       JsonDocument doc;
//       doc["distancia"] = distancia;
//       doc["tank_h"] = atoi(tank_h);
//       doc["sensor_m"] = atoi(sensor_m);
//       doc["delta_max"] = delta_max;
//       doc["boton1"] = "0";
//       doc["boton2"] = "0";
//       doc["boton3"] = "0";
//       doc["boton4"] = "0";

//       String payload;
//       serializeJson(doc, payload);
//       int httpCode = http.POST(payload);

//       if (httpCode > 0) {
//         Serial.printf("[API] Código HTTP: %d\n", httpCode);
//         if (httpCode >= 200 && httpCode < 300) {
//           apiStatus = 1;
//         } else {
//           apiStatus = 2;
//           Serial.print(F("[API] Detalle del Error: "));
//           Serial.println(http.getString());
//         }
//       } else {
//         apiStatus = 0;
//         Serial.printf("[API] Fallo: %s\n", http.errorToString(httpCode).c_str());
//       }
//       http.end();
//     }
//     tApi = millis();
//   }
// }
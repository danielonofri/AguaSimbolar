void ejecutarAPI() {
  if (millis() - tApi < 10000) return;
  tApi = millis();

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  
  // 👉 EL AJUSTE CLAVE: 
  // Obliga al servidor y al micro a usar fragmentos de 1KB.
  // Esto evita el error de "protocol version unsupported" en servidores modernos.
  client.setBufferSizes(1024, 1024); 

  HTTPClient http;
  http.useHTTP10(false); // Render requiere HTTP/1.1 para POST

  Serial.println(F("[API] Negociando TLS (MFLN 1024)..."));

  if (http.begin(client, api_url)) {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "close");
    http.addHeader("Host", "simbolar-api.onrender.com");

    // Construcción del JSON
    JsonDocument doc;
    doc["altura_agua"] = altura_agua;
    doc["porcentaje"] = porcentaje;
    doc["tank_h"] = atoi(tank_h);
    doc["sensor_m"] = atoi(sensor_m);
    doc["delta_max"] = delta_max;
    doc["boton1"] = sw_remotos[1] ? "1" : "0";
    doc["boton2"] = sw_remotos[2] ? "1" : "0";
    doc["boton3"] = sw_remotos[3] ? "1" : "0";
    doc["boton4"] = sw_remotos[4] ? "1" : "0";

    String payload;
    serializeJson(doc, payload);

    int httpCode = http.POST(payload);

    if (httpCode > 0) {
      Serial.printf("[API] Éxito! Código HTTP: %d\n", httpCode);
      apiStatus = 1;
      
      // Procesar respuesta para el LCD
      String respuesta = http.getString();
      JsonDocument resDoc;
      if (!deserializeJson(resDoc, respuesta)) {
        if (!resDoc["lcd"].isNull()) {
          bool estadoLcd = resDoc["lcd"];
          estadoLcd ? lcd.backlight() : lcd.noBacklight();
          lcdEncendido = estadoLcd;
        }
      }
    } else {
      Serial.printf("[API] Fallo: %s\n", http.errorToString(httpCode).c_str());
      char last_err[100];
      client.getLastSSLError(last_err, 100);
      Serial.printf("[SSL Detalle]: %s\n", last_err);
    }
    http.end();
  }
}


// void ejecutarAPI() {
//   // 1. Verificación de tiempo y conexión física
//   if (millis() - tApi < 10000) return;
//   tApi = millis();

//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println(F("[API] Error: WiFi desconectado"));
//     apiStatus = 0;  // ❌
//     return;
//   }


//   // 2. Preparación del Cliente
//   //WiFiClient client;
//   WiFiClientSecure client;  // <--- USAMOS CLIENTE SEGURO
//   client.setInsecure();     // <--- TRUCO: Salta la verificación de certificado (evita errores)
//   client.setTimeout(15000);
//   client.setBufferSizes(1024, 1024);
//   client.setSession(nullptr);
//   client.setHandshakeTimeout(10000);

//   HTTPClient http;
//     http.useHTTP10(false); // Usar HTTP 1.1 explícitamente

//   Serial.println(F("[API] Intentando POST..."));
//   // 3. Inicio de la petición
//   if (http.begin(client, api_url)) {
//     http.addHeader("Content-Type", "application/json");
//     http.addHeader("Connection", "close");

//     // 4. Construcción del JSON
//     JsonDocument doc;
//     doc["altura_agua"] = altura_agua;
//     doc["porcentaje"] = porcentaje;
//     doc["tank_h"] = atoi(tank_h);
//     doc["sensor_m"] = atoi(sensor_m);
//     doc["delta_max"] = delta_max;
//     doc["boton1"] = sw_remotos[1] ? "1" : "0";
//     doc["boton2"] = sw_remotos[2] ? "1" : "0";
//     doc["boton3"] = sw_remotos[3] ? "1" : "0";
//     doc["boton4"] = sw_remotos[4] ? "1" : "0";

//     String payload;
//     serializeJson(doc, payload);

//     // 5. Envío y captura de respuesta
//     int httpCode = http.POST(payload);

//     // 👇 AGREGA ESTO PARA VER EL ERROR REAL 👇
//     int m_error = 0;
//     char last_err[100];
//     // ... después del http.POST ...
//     if (httpCode < 0) {
//       client.getLastSSLError(last_err, 100);
//       Serial.printf("[SSL ERROR] %s\n", last_err);
//     }

//     if (httpCode > 0) {
//       Serial.printf("[API] Código HTTP: %d\n", httpCode);

//       if (httpCode == HTTP_CODE_OK || httpCode == 201) {
//         apiStatus = 1;  // ✔️

//         // --- NUEVO: Procesar respuesta para el LCD ---
//         String respuesta = http.getString();
//         JsonDocument resDoc;
//         DeserializationError error = deserializeJson(resDoc, respuesta);

//         if (!error) {
//           // En v7, simplemente preguntamos si el valor no es nulo
//           if (!resDoc["lcd"].isNull()) {
//             bool estadoLcd = resDoc["lcd"];  // ArduinoJson convierte automáticamente a bool

//             if (estadoLcd) {
//               lcd.backlight();
//               lcdEncendido = true;  // Sincronizamos tu variable de control
//             } else {
//               lcd.noBacklight();
//               lcdEncendido = false;  // Sincronizamos tu variable de control
//             }
//           }
//         }
//         // --------------------------------------------

//       } else {
//         apiStatus = 2;  // ⚠️
//         Serial.printf("⚠️");
//         Serial.println(http.getString());
//       }
//     } else {
//       apiStatus = 0;  // ❌
//       Serial.printf("[API] ❌: %s\n", http.errorToString(httpCode).c_str());
//     }
//     http.end();
//   } else {
//     apiStatus = 0;  // ❌
//     Serial.println(F("[API] No se pudo establecer conexión con el host."));
//   }
// }

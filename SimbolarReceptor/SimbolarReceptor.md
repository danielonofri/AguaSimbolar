Para completar tu proyecto **Receptor Nivel de Tanque (SimbolarReceptor)**, he preparado la guía de conexión física y la actualización del código necesaria para integrar el módulo **XL1278-SMT (LoRa)** con tu **NodeMCU**.

---

## 1. Conexiones XL1278-SMT a NodeMCU

El módulo XL1278-SMT utiliza comunicación SPI. Es fundamental que el módulo se alimente con la **fuente externa de 3.3V**, ya que el pin de 3.3V del NodeMCU a veces no suministra la corriente necesaria para los picos de transmisión/recepción de LoRa.

| XL1278-SMT | NodeMCU (ESP8266) | Descripción |
| --- | --- | --- |
| **VCC** | Fuente Externa (+) 3.3V | Alimentación positiva. |
| **GND** | Fuente Externa (-) **y** GND NodeMCU | **Tierra Común** (Indispensable). |
| **MISO** | **D6** (GPIO 12) | Master In Slave Out. |
| **MOSI** | **D7** (GPIO 13) | Master Out Slave In. |
| **SCK** | **D5** (GPIO 14) | Serial Clock. |
| **NSS (CS)** | **D8** (GPIO 15) | Chip Select. |
| **DIO0** | **D2** (GPIO 4) | Interrupción (Aviso de paquete recibido). |
| **REST** | **D0** (GPIO 16) | Reset del módulo. |

---

## 2. Actualización del Código (SimbolarReceptor.ino)

Para que el receptor escuche al emisor, debes incluir la librería `LoRa.h` y reemplazar la lectura aleatoria actual por la recepción real de paquetes.

### A. Definiciones y Librería

Agrega estas líneas al principio de tu archivo, junto a las demás librerías:

```cpp
#include <SPI.h>
#include <LoRa.h>

// Definición de pines para XL1278-SMT
const int csPin = 15;          // D8
const int resetPin = 16;       // D0
const int irqPin = 4;          // D2

```

### B. Inicialización en `setup()`

Dentro de la función `setup()`, antes o después de la configuración de WiFiManager, añade la inicialización de LoRa:

```cpp
void setup() {
  // ... (tu código existente de Serial y LCD)
  
  LoRa.setPins(csPin, resetPin, irqPin);

  // Intentar iniciar LoRa en 433MHz (ajustar si tu módulo es de otra frecuencia)
  if (!LoRa.begin(433E6)) { 
    Serial.println(F("Error: No se encuentra el módulo LoRa"));
    lcd.setCursor(0,0);
    lcd.print(F("Error Hardware"));
    delay(2000);
  }
  
  // ... (resto de tu setup)
}

```

### C. Reemplazo de lógica en `ejecutarAPI()`

Actualmente, tu código usa `random(margen, fondo)` para simular la lectura. Debes modificar esa sección para que lea los datos recibidos por LoRa:

```cpp
void ejecutarAPI() {
  if (millis() - tApi > 10000) {
    [cite_start]int fondo = atoi(tank_h); [cite: 36]
    [cite_start]int margen = atoi(sensor_m); [cite: 36]
    
    // --- NUEVA LÓGICA LORA ---
    int lecturaCruda = -1;
    int packetSize = LoRa.parsePacket();
    
    if (packetSize) {
      String recibo = "";
      while (LoRa.available()) {
        recibo += (char)LoRa.read();
      }
      lecturaCruda = recibo.toInt(); // Convierte el mensaje del emisor a entero
      Serial.print(F("Dato LoRa recibido: ")); Serial.println(lecturaCruda);
    }

    // Solo procesamos si recibimos algo válido, sino mantenemos el anterior
    if (lecturaCruda != -1) {
      [cite_start]distancia = aplicarFiltroDelta(lecturaCruda); [cite: 61]
    }
    
    // El resto del procesamiento (map, porcentaje, POST API) se mantiene igual
    [cite_start]altura_agua = fondo - distancia; [cite: 37]
    [cite_start]porcentaje = map(distancia, fondo, margen, 0, 100); [cite: 38]
    [cite_start]porcentaje = constrain(porcentaje, 0, 100); [cite: 38]
    
    [cite_start]// ... (Carga de datos a la API configurada en api_url) [cite: 40, 43]
    
    [cite_start]tApi = millis(); [cite: 50]
  }
}

```

---

## 3. Notas de Funcionamiento

* 
**Filtro Delta:** El código conservará tu función `aplicarFiltroDelta` , la cual es excelente para evitar falsos positivos si el sensor ultrasónico del emisor tiene rebotes de señal, respetando el `delta_max` configurado en el portal WiFi.


* **Sincronización:** Asegúrate de que el Emisor envíe el dato de distancia simplemente como una cadena de texto (ej. `"120"`) para que el `toInt()` del Receptor lo interprete correctamente.

¿Te gustaría que también te ayude a redactar el código básico para el **Nodo Emisor** para que coincida con este protocolo?
Aquí tienes el resumen consolidado de todo el sistema. Este documento sirve como la "hoja de ruta" técnica para tu proyecto **AguaSimbolar**. Puedes copiarlo en un archivo `.txt` para revisarlo y tenerlo a mano para la implementación.

---

### **RESUMEN TÉCNICO: PROYECTO AGUASIMBOLAR (V1.0)**

#### **1. ARQUITECTURA DEL SISTEMA**

* **Transmisor (Arduino Uno):** Encargado de la medición física y ejecución de comandos en campo.
* **Receptor (NodeMCU):** Cerebro central, gestiona configuración local (WiFiManager), comunicación con la API Web y control del LCD.
* **API (C# / Web):** Interfaz de usuario para monitoreo y envío de órdenes en tiempo real.

#### **2. CONFIGURACIÓN DE HARDWARE (ARCO UNO)**

* **Sensor Ultrasónico:** Pines **D3** (Trigger) y **D4** (Echo).
* **Comunicación LoRa:** Pines **D13, D12, D11, D10, D9 y D2**.
* **Entradas de Estado (p_in):** Pines **A0, A1, A2, A3** (digitalizados con `INPUT_PULLUP`). No requieren resistencias externas (pulsadores a GND).
* **Salidas de Actuadores (p_out):** Pines **A4, A5, D7, D8**. No requieren resistencias si usas módulos de relés comerciales.

#### **3. CONFIGURACIÓN DE HARDWARE (NODEMCU)**

* **LCD I2C:** Pines **D1 (SCL)** y **D2 (SDA)**.
* **Control/Reset:** Pin **D4**.
* Presión corta: Encender/Apagar LCD.
* Presión 10s: Reset de configuración WiFiManager.



#### **4. ESTRUCTURA DE DATOS UNIFICADA (VOLÁTIL)**

El sistema no usa base de datos; la verdad reside en la memoria del NodeMCU.

**A. Paquete LoRa (Binario):**

```cpp
struct Payload {
  float distancia; // Lectura del sensor
  byte p_in;       // Bits 0-3: Estado de botones en Uno
  byte p_out;      // Bits 0-3: Órdenes para relés en Uno
};

```

**B. Paquete API (JSON):**

* **Enviado a Web:** Distancia, `p_in`, y parámetros locales (`tank_h`, `sensor_m`, `delta_max`).
* **Recibido de Web:** Comandos `p_out` y estado del `lcd`.

#### **5. LÓGICA DE CONTROL SERIAL (NODEMCU)**

Comandos aceptados por terminal: `lcd_on`, `lcd_off`, `force_reset`.

---

### **CÓMO IMPLEMENTAR LA TRANSMISIÓN DE VUELTA (RECEPTOR -> TRANSMISOR)**

Para que el Arduino Uno reciba los comandos de salida sin colgar el circuito (evitando que ambos transmitan a la vez), utilizaremos el método de **Ventaja de Escucha (Listen Window)**.

#### **El Flujo de "Ida y Vuelta" (Half-Duplex):**

1. **El Transmisor (Uno) toma la iniciativa:**
* Lee la distancia y sus entradas.
* Envía el paquete LoRa al Receptor.
* **Inmediatamente después de enviar**, el Uno se pone en modo `LoRa.receive()` por un breve tiempo (ej. 200ms).


2. **El Receptor (NodeMCU) responde:**
* Recibe el paquete del Uno.
* Actualiza su memoria y habla con la API Web.
* **Si tiene un comando nuevo** (o simplemente para confirmar), envía de vuelta un paquete LoRa al Uno que contiene el byte `p_out`.


3. **El Transmisor (Uno) procesa la vuelta:**
* Si recibe el paquete en su "ventana de escucha", extrae el `p_out`.
* Llama a la función `escribirSalidas(p_out)` para activar/desactivar los relés.

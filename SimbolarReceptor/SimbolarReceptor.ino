#include <LittleFS.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFiClientSecure.h>

// --- Hardware ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define PIN_BOTON_MULTIFUNCION D4

const int csPin = 15;
const int resetPin = 16;
const int irqPin = 4;
struct __attribute__((packed)) Payload {
  float dist;  // 4 bytes
  byte p_in;   // 1 byte
  byte p_out;  // 1 byte
};

Payload p;
// --- Variables Globales ---
byte p_in_empaquetado = 0;
int delta_max = 15;
char tank_h[6] = "220", sensor_m[6] = "20", tank_delta[5] = "15";
//char api_url[100] = "http://45.234.117.236:54625/api/Sensores";
char api_url[100] = "https://simbolar-api-1bc5.onrender.com/api/Sensores";
int distancia = 0, porcentaje = 0, altura_agua = 0, distanciaAnterior = 0, lecturasEstables = 0;
bool sw_remotos[5] = { false };
const int LECTURAS_PARA_ARRANQUE = 5;
unsigned long tApi = 0;
int pantallaActual = 0;  // <--- AQUÍ VA LA VARIABLE
bool lcdEncendido = true;
unsigned long tScroll = 0;

int apiStatus = 0;
bool shouldSaveConfig = false;
int pantalla = 0;
bool primeramuestralcd = false;
WiFiManager wm;

WiFiManagerParameter c_h("h", "Altura", tank_h, 5);
WiFiManagerParameter c_m("m", "Margen", sensor_m, 5);
WiFiManagerParameter c_d("d", "Delta Max (cm)", tank_delta, 5);
WiFiManagerParameter c_u("u", "API URL", api_url, 100);

byte check[8] = { 0x00, 0x01, 0x03, 0x16, 0x1C, 0x08, 0x00, 0x00 };
byte cross[8] = { 0x00, 0x1B, 0x0E, 0x04, 0x0E, 0x1B, 0x00, 0x00 };
byte warn[8] = { 0b00100, 0b00100, 0b01010, 0b01010, 0b10001, 0b10101, 0b11111, 0b00000 };

void saveConfigCallback() {
  shouldSaveConfig = true;
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n--- INICIANDO RECEPTOR ---"));
  configTime(0, 0, "time.google.com", "pool.ntp.org");
  pinMode(PIN_BOTON_MULTIFUNCION, INPUT_PULLUP);

  Wire.begin(5, 0);
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, check);
  lcd.createChar(1, cross);
  lcd.createChar(2, warn);

  cargarConfiguracion();

  IPAddress _ip(10, 0, 0, 1);
  wm.setAPStaticIPConfig(_ip, _ip, IPAddress(255, 255, 255, 0));
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConfigPortalBlocking(false);

  wm.addParameter(&c_h);
  wm.addParameter(&c_m);
  wm.addParameter(&c_d);
  wm.addParameter(&c_u);

  wm.autoConnect("AguaSimbolar_AP");
  Serial.println(F("[SISTEMA] Configurando LoRa..."));
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println(F("LORA RECEPTOR: ERROR (No se encuentra el chip)"));
    // Aquí puedes prender un LED de error si tienes uno
    while (1)
      ;
  } else {
    // Si llegó acá, es que el hardware respondió 0x12 internamente
    Serial.println(F("LORA RECEPTOR: OK (Hardware detectado)"));
  }


  LoRa.setSyncWord(0xF3);
  LoRa.enableCrc();

  configTime(-3 * 3600, 0, "ar.pool.ntp.org", "time.google.com");
  Serial.println(F("\n--- SISTEMA LISTO Y CORRIENDO ---"));
  lcd.clear();
  
}


void loop() {
  wm.process();
  procesarGuardadoConfig();
  gestionarSerial();
  gestionarBotonMultifuncion();  // Estará en el archivo del Botón
  actualizarMediciones();
  if (WiFi.status() == WL_CONNECTED) {
    ejecutarAPI();
  }
  yield();
}

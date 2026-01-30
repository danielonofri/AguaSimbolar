#include <printf.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Declaremos los pines CE y el CSN de la Radio y Pines de alerta maximo nivel y Nivel Intermedio
#define CE_PIN 9
#define CSN_PIN 10
//Declaramos el Trigger y el Echo del sensor ultrasonico
#define SU_ECHO 5
#define SU_TRIGGER 6
//Resistencias Pull Down para manener el nivel bajo mientras el interruptor estÃƒÂ¡ apagado
#define PIN_MAX_NIVEL 4
#define PIN_NIVEL_MEDIO 7


//Variable con la direccion del canal por donde se va a transmitir
byte direccion[5] = {'c', 'a', 'u', 'n', 'a'};

//creamos el objeto radio (NRF24L01)
RF24 radio(CE_PIN, CSN_PIN);
//const int pinLed = 13;
const int pinLedRojo = 2;
const int pinLedVerde = 8;
//vector con los datos a enviar
float registro[7];
float promedio, acumulado;
int tamMuestra,muestrasobtenidas,demora;

void setup()
{
//  pinMode(pinLed,OUTPUT);
  pinMode(pinLedRojo,OUTPUT);
  pinMode(pinLedVerde,OUTPUT);
  
  pinMode(PIN_MAX_NIVEL, INPUT);
  pinMode(PIN_NIVEL_MEDIO, INPUT);
  pinMode(SU_TRIGGER, OUTPUT);
  pinMode(SU_ECHO, INPUT);
  
//  digitalWrite(pinLed,LOW);
  
  Serial.begin(9600);
  
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  //inicializamos el puerto serie
  Serial.begin(9600);

  //Abrimos un canal de escritura
  radio.openWritingPipe(direccion);
  demora = 1000;
  promedio = 0.00;
  tamMuestra = 10;
  muestrasobtenidas = 0;
  }

void loop()
{
  digitalWrite(pinLedRojo,LOW);
  digitalWrite(pinLedVerde,LOW);
  delay(demora);
  if (registro[0] > 255)
  {
    registro[0] = 1;
  }
  else
  {
    registro[0] = registro[0] + 1;
  }
  registro[1] =  analogRead(A1);
  acumulado = acumulado + registro[1];
  muestrasobtenidas = muestrasobtenidas + 1;
  registro[2] = (float) muestrasobtenidas;
  //Si se obtuvo la cantidad de muestras igual a tamMuestra se calcula promedio
  if(muestrasobtenidas == tamMuestra )
  {
    registro[3] = (float) (acumulado / muestrasobtenidas);
    muestrasobtenidas = 0;
    acumulado = 0.00;
  }
  registro[4] = digitalRead(PIN_MAX_NIVEL);
  registro[5] = digitalRead(PIN_NIVEL_MEDIO);
  registro[6]= devuelve_distancia();
  //enviamos los datos
  
  
  bool ok = radio.write(registro, sizeof(registro));  
 // bool ok = true;

  if (ok)
    {
    digitalWrite(pinLedVerde,HIGH);
    Serial.print("Datos enviados:");
    Serial.print(registro[0]);
    Serial.print(":");
    Serial.print(registro[1]);
    Serial.print(":");
    Serial.print(registro[2]);
    Serial.print(":");
    Serial.print(registro[3]);
    Serial.print(":");
    Serial.print(registro[4]);
    Serial.print(":");
    Serial.print(registro[5]);
    Serial.print(":");
    Serial.print(registro[6]);
    Serial.println(":");
    }
    else
    {
    digitalWrite(pinLedRojo,HIGH);
     Serial.print("XXX[");
      Serial.print(registro[0]);
      Serial.print(":");
      Serial.print(registro[1]);
      Serial.print(":");
      Serial.print(registro[2]);
      Serial.print(":");
      Serial.print(registro[3]);
      Serial.print(":");
      Serial.print(registro[4]);
      Serial.print(":");
      Serial.print(registro[5]);
      Serial.print(":");
      Serial.print(registro[6]);
      Serial.println("]");
    }
 
    delay(demora);
 }

float devuelve_distancia()
{
  int distancia;
  int tiempo;
  digitalWrite(SU_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(SU_TRIGGER, LOW);
  
  // medimos el pulso de respuesta
  tiempo = (pulseIn(SU_ECHO, HIGH)/2); // dividido por 2 por que es el 
                                       // tiempo que el sonido tarda
                                       // en ir y en volver
  // ahora calcularemos la distancia en cm
  // sabiendo que el espacio es igual a la velocidad por el tiempo
  // y que la velocidad del sonido es de 343m/s y que el tiempo lo 
  // tenemos en millonesimas de segundo
  distancia = float(tiempo * 0.0343);
  return distancia;
  }



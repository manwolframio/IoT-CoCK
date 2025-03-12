// Prueba 3 (plotter)
//
//#include <Wire.h>
//#include "MAX30105.h"
//
//MAX30105 particleSensor;
//
//const int sdaPin = 17; // Pin SDA de tu ESP32
//const int sclPin = 18; // Pin SCL de tu ESP32
//
//void setup() {
//  Serial.begin(115200); // Inicializar comunicación serie
//  Wire.begin(sdaPin, sclPin); // Inicializar I2C con pines personalizados
//
//  Serial.println("Inicializando sensor...");
//
//  // Inicializar el sensor
//  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Usar el puerto I2C predeterminado
//    Serial.println("No se detectó el MAX30105. Verifica las conexiones.");
//    while (1); // Detener ejecución si no se detecta el sensor
//  }
//
//  Serial.println("Coloca tu dedo sobre el sensor con presión constante.");
//
//  // Configurar el sensor
//  byte ledBrightness = 0x1F;  // Opciones: 0 (apagado) a 255 (50mA)
//  byte sampleAverage = 8;     // Promediar 8 muestras (opciones: 1, 2, 4, 8, 16, 32)
//  byte ledMode = 3;           // LEDs rojo + IR + verde
//  int sampleRate = 100;       // Frecuencia de muestreo (100 Hz)
//  int pulseWidth = 411;       // Ancho de pulso (411 ns)
//  int adcRange = 4096;        // Rango ADC (4096 niveles)
//
//  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
//
//  // Prellenar el gráfico del plotter para una escala inicial estable
//  const byte avgAmount = 64; // Cantidad de muestras para el promedio inicial
//  long baseValue = 0;
//  for (byte x = 0; x < avgAmount; x++) {
//    baseValue += particleSensor.getIR(); // Leer los valores IR
//  }
//  baseValue /= avgAmount;
//
//  // Enviar valores promedio iniciales al plotter
//  for (int x = 0; x < 500; x++) {
//    Serial.println(baseValue);
//  }
//}
//
//void loop() {
//  // Enviar datos IR en tiempo real al Serial Plotter
//  Serial.println(particleSensor.getIR());
//}
//


// Prueba 2 (solo imprime en los latidos)
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h" // Asegúrate de incluir la librería adecuada

MAX30105 particleSensor;

const byte RATE_SIZE = 4; // Tamaño del buffer para el promedio
byte rates[RATE_SIZE];    // Array para almacenar BPM
byte rateSpot = 0;        // Índice actual del buffer
long lastBeat = 0;        // Tiempo del último latido

float beatsPerMinute;
int beatAvg;

const int sdaPin = 17; // Pin SDA en ESP32
const int sclPin = 18; // Pin SCL en ESP32

void setup() {
  Serial.begin(115200);
  Wire.begin(sdaPin, sclPin); // Inicializar I2C personalizado

  Serial.println("Inicializando sensor...");

  // Inicializar el sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("No se detectó el MAX30105. Verifica las conexiones.");
    while (1);
  }

  Serial.println("Coloca tu dedo sobre el sensor con presión constante.");

  // Configurar el sensor
  particleSensor.setup();                    // Configuración por defecto
  particleSensor.setPulseAmplitudeRed(0x0A); // LED rojo a baja intensidad
  particleSensor.setPulseAmplitudeGreen(0);  // Apagar LED verde
}

void loop() {
  long irValue = particleSensor.getIR(); // Obtener valor IR del sensor

  // Detectar latido usando la función checkForBeat()
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat; // Tiempo entre latidos
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0); // Calcular BPM

    // Validar que BPM está en un rango normal
    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; // Almacenar en el buffer
      rateSpot %= RATE_SIZE; // Rotar el índice para sobrescribir los más antiguos

      // Calcular promedio de BPM
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;

      // Imprimir solo cuando se detecte un latido
      Serial.print("Latido detectado! BPM: ");
      Serial.print(beatsPerMinute);
      Serial.print(", Promedio BPM: ");
      Serial.println(beatAvg);
    }
  }

  delay(10); // Pequeño retraso para evitar saturar el procesador
}


// Prueba 1

//#include <Wire.h>
//#include "MAX30105.h"
//#include "heartRate.h" // Asegúrate de incluir la librería adecuada
//
//MAX30105 particleSensor;
//
//const byte RATE_SIZE = 4; // Tamaño del buffer para promediar
//byte rates[RATE_SIZE];    // Array de frecuencias cardíacas
//byte rateSpot = 0;
//long lastBeat = 0;        // Momento del último latido
//
//float beatsPerMinute;
//int beatAvg;
//
//const int sdaPin = 17; // Pin SDA del ESP32
//const int sclPin = 18; // Pin SCL del ESP32
//
//void setup() {
//  Serial.begin(115200);
//  Wire.begin(sdaPin, sclPin); // Inicializar I2C personalizado
//
//  Serial.println("Inicializando sensor...");
//
//  // Inicializar el sensor
//  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Velocidad I2C: 400 kHz
//    Serial.println("No se detectó el MAX30105. Verifica las conexiones.");
//    while (1); // Detener ejecución si no se detecta el sensor
//  }
//
//  Serial.println("Coloca tu dedo sobre el sensor con presión constante.");
//
//  // Configurar el sensor
//  particleSensor.setup();                    // Configuración por defecto
//  particleSensor.setPulseAmplitudeRed(0x0A); // LED rojo a baja intensidad
//  particleSensor.setPulseAmplitudeGreen(0);  // Apagar LED verde
//}
//
//void loop() {
//  long irValue = particleSensor.getIR(); // Obtener valor IR del sensor
//
//  // Detectar latido usando la función checkForBeat()
//  if (checkForBeat(irValue) == true) {
//    long delta = millis() - lastBeat;
//    lastBeat = millis();
//
//    beatsPerMinute = 60 / (delta / 1000.0); // Calcular BPM
//
//    if (beatsPerMinute < 255 && beatsPerMinute > 20) { // Filtro de valores válidos
//      rates[rateSpot++] = (byte)beatsPerMinute; // Almacenar en el array
//      rateSpot %= RATE_SIZE; // Rotar el índice para sobrescribir los más antiguos
//
//      // Calcular promedio de BPM
//      beatAvg = 0;
//      for (byte x = 0; x < RATE_SIZE; x++) {
//        beatAvg += rates[x];
//      }
//      beatAvg /= RATE_SIZE;
//    }
//  }
//
//  // Mostrar información en el monitor serie
//  Serial.print("IR=");
//  Serial.print(irValue);
//  Serial.print(", BPM=");
//  Serial.print(beatsPerMinute);
//  Serial.print(", Promedio BPM=");
//  Serial.print(beatAvg);
//
//  if (irValue < 50000) {
//    Serial.print(" ¿Sin dedo?");
//  }
//
//  Serial.println();
//  delay(10); // Pequeño retraso para evitar saturar el procesador
//}

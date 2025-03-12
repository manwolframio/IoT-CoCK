#include <Wire.h>
#include "MAX30105.h" // Incluye la librería MAX30105

MAX30105 particleSensor;

const int sdaPin = 17; // Pin SDA en ESP32
const int sclPin = 18; // Pin SCL en ESP32

long samplesTaken = 0;   // Contador de muestras tomadas
long unblockedValue;     // Promedio inicial de IR al iniciar
long startTime;          // Tiempo de inicio para calcular la tasa de muestreo

void setup() {
  Serial.begin(115200); // Iniciar comunicación serie
  Wire.begin(sdaPin, sclPin); // Inicializar I2C con pines personalizados

  Serial.println("Ejemplo de detección de presencia con MAX30105");

  // Inicializar el sensor MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Velocidad I2C: 400 kHz
    Serial.println("No se detectó el MAX30105. Verifica las conexiones.");
    while (1); // Detener ejecución si no se detecta el sensor
  }

  // Configurar sensor para detectar hasta ~18 pulgadas
  byte ledBrightness = 0xFF;  // Brillo máximo (0 = apagado, 255 = 50mA)
  byte sampleAverage = 4;     // Promediar 4 muestras
  byte ledMode = 2;           // Usar LEDs rojo + IR
  int sampleRate = 400;       // Tasa de muestreo: 400 Hz
  int pulseWidth = 411;       // Ancho de pulso: 411 ns
  int adcRange = 2048;        // Rango ADC: 2048

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  // Apagar los LEDs rojo y verde para usar solo el LED infrarrojo
  particleSensor.setPulseAmplitudeRed(0);
  particleSensor.setPulseAmplitudeGreen(0);

  // Tomar una lectura promedio inicial del LED infrarrojo
  unblockedValue = 0;
  for (byte x = 0; x < 32; x++) {
    unblockedValue += particleSensor.getIR(); // Leer valores IR
  }
  unblockedValue /= 32; // Calcular promedio inicial

  startTime = millis(); // Registrar el tiempo de inicio
}

void loop() {
  samplesTaken++; // Incrementar contador de muestras

  // Leer y mostrar valores actuales de IR
  long irValue = particleSensor.getIR();
  Serial.print("IR[");
  Serial.print(irValue);
  Serial.print("] Tasa Hz[");
  Serial.print((float)samplesTaken / ((millis() - startTime) / 1000.0), 2);
  Serial.print("]");

  // Calcular la diferencia con el valor promedio inicial
  long currentDelta = irValue - unblockedValue;
  Serial.print(" Delta[");
  Serial.print(currentDelta);
  Serial.print("]");

  // Detectar si algo está presente (si la diferencia supera el umbral)
  if (currentDelta > 100) { // Umbral de sensibilidad
    Serial.print(" ¡Algo está presente!");
  }

  Serial.println();

  // Pequeño retraso para evitar saturar el procesador
  delay(100);
}

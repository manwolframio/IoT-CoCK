#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

const int sdaPin = 17; // Pin SDA del ESP32
const int sclPin = 18; // Pin SCL del ESP32

void setup() {
  Serial.begin(115200); // Inicializar comunicación serie
  Wire.begin(sdaPin, sclPin); // Inicializar bus I2C con pines personalizados

  Serial.println("Inicializando sensor...");

  // Inicializar el sensor MAX30105
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) { // Configurar puerto I2C, velocidad 400 kHz
    Serial.println("No se detectó el MAX30105. Verifica las conexiones.");
    while (1); // Detener ejecución si el sensor no se detecta
  }

  // Apagar los LEDs para evitar calentamiento local que afecte la medición
  particleSensor.setup(0); // Configurar el sensor con LEDs apagados

  // Habilitar la interrupción del sensor de temperatura
  particleSensor.enableDIETEMPRDY(); // Esta línea es necesaria para medir temperatura
}

void loop() {
  // Leer temperatura en grados Celsius
  float temperature = particleSensor.readTemperature();
  Serial.print("Temperatura (Celsius): ");
  Serial.print(temperature, 4);

  // Leer temperatura en grados Fahrenheit
  float temperatureF = particleSensor.readTemperatureF();
  Serial.print(" | Temperatura (Fahrenheit): ");
  Serial.print(temperatureF, 4);

  Serial.println();

  // Esperar un segundo antes de la próxima lectura
  delay(1000);
}

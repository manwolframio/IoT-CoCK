#include "SensorTemp.h"

int tempSensorInit(MAX30105 &particleSensor, int sdaPin, int sclPin) {
    Serial.begin(115200); // Inicializar comunicación serie
    Wire.begin(sdaPin, sclPin); // Inicializar bus I2C con pines personalizados

    Serial.println("Inicializando sensor...");

    // Inicializar el sensor MAX30105
    if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) { // Configurar puerto I2C, velocidad 400 kHz
        return -1;
    }

    // Apagar los LEDs para evitar calentamiento local que afecte la medición
    particleSensor.setup(0); // Configurar el sensor con LEDs apagados

    // Habilitar la interrupción del sensor de temperatura
    particleSensor.enableDIETEMPRDY(); // Esta línea es necesaria para medir temperatura
    return 1;
}

float temperatureAcquire(MAX30105 &particleSensor) {
  // Leer temperatura en grados Celsius
  float temperature = particleSensor.readTemperature();
  return temperature;
}

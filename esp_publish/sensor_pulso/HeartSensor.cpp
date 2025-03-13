#include "HeartSensor.h"


int heartSensorInit(MAX30105 &particleSensor,int sdaPin, int sclPin) {
    Wire.begin(sdaPin, sclPin); // Inicializar I2C con pines personalizados
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("No se detectó el MAX30105. Verifica las conexiones.");
        return -1;
    }
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
    return 1;
}

float* heartbeatAcquire(MAX30105 &particleSensor) {
    float heartbeat[2] = {};
    static byte rates[4] = {};
    static long lastBeat = 0;
    static byte rateSpot = 0;

    float beatsPerMinute = 0;
    int beatAvg = 0;

    long irValue = particleSensor.getIR(); // Obtener valor IR del sensor

    if (checkForBeat(irValue)) { // Detectar latido
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= 4;

            beatAvg = 0;
            for (byte x = 0; x < 4; x++) {
                beatAvg += rates[x];
            }
            beatAvg /= 4;
        }
        heartbeat[0] = beatsPerMinute;
        heartbeat[1] = beatAvg;

        return heartbeat;
    }
    else{
        heartbeat[0] = -1;
        heartbeat[1] = -1;
        return heartbeat;
    } 
}

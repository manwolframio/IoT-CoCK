#include "HeartSensor.h"

int heartSensorInit(MAX30105 &particleSensor, int sdaPin, int sclPin) {
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

int getAverageBPM(MAX30105 &particleSensor) {
    static long lastBeat;
    static byte rates[RATE_SIZE] = {0};
    static byte rateSpot = 0;
    float beatsPerMinute;
    int beatAvg;
    if (rateSpot>=3) rateSpot = 0; else rateSpot++;
    int32_t irValue = particleSensor.getIR();
    if (checkForBeat(irValue)) {
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
            rates[rateSpot] = beatsPerMinute;
            Serial.println(rates[rateSpot]);
        }
    }
    beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
    }
    beatAvg /= RATE_SIZE;

    return beatAvg;
}

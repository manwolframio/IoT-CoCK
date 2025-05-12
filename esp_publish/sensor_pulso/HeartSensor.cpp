#include "HeartSensor.h"

const char wifi_ssid[] = "LRSS";     
const char wifi_password[] = "LRSS-uah-8342";  
const char mqtt_broker_address[] = "192.168.188.225";  
const int mqtt_port = 1883;
const char mqtt_client_id[] = "sensor:heartrate:001";  
const char patient_id[] = "001";
float hb[2];
long lastPrint = 0;

WiFiClient network;
MQTTClient mqtt(256);



void initializeWiFi() {
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
}

void initializeMQTT() {
  mqtt.begin(mqtt_broker_address, mqtt_port, network);
  while (!mqtt.connect(mqtt_client_id)) {
    delay(1000);
    Serial.println("Conectando a MQTT...");
  }
  Serial.println("Conectado a MQTT");
}


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

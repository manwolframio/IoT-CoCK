#include <WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include <Adafruit_NeoPixel.h>  // Librería para LED direccionable

// Configuración WiFi
const char WIFI_SSID[] = "LE-34";     
const char WIFI_PASSWORD[] = "mpls1234";  

// Configuración MQTT
const char MQTT_BROKER_ADDRESS[] = "192.168.205.2";  
const int MQTT_PORT = 1883;
const char MQTT_CLIENT_ID[] = "sensor:heartrate:001";  

// Topics MQTT generados dinámicamente
const char SENSOR_ID[] = "sensor:heartrate:001";  

const char TOPIC_MEDIDA[] = "sensors/sensor:heartrate:001/medida"; 
const char TOPIC_STATUS[] = "sensors/sensor:heartrate:001/status";  

const int PUBLISH_INTERVAL = 5000;  

WiFiClient network;
MQTTClient mqtt(256);
unsigned long lastPublishTime = 0;

// Configuración del LED addressable
#define LED_PIN 48  // Ajusta según tu ESP32-S3
#define NUM_LEDS 1  // Número de LEDs en la tira
Adafruit_NeoPixel led = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// NTP
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET = -18000; // UTC-5 (ajusta según zona horaria)
const int DAYLIGHT_OFFSET = 3600; 

int colors[] = {255,0,0};

void setup() {
  Serial.begin(115200);

  // Configurar LED
  led.begin();
  led.show();  // Apagar LED al inicio

  setLedColor();  // Rojo (no conectado)

  // Conexión WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("ESP32 - Conectando a Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");

  // Sincronizar hora con NTP
  configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER);
  Serial.println("Sincronizando con NTP...");
  while (!timeSynced()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada!");

  // Conectar a MQTT
  connectToMQTT();
}

void loop() {
  mqtt.loop();

  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    sendToMQTT();
    lastPublishTime = millis();
  }
}

void connectToMQTT() {
  mqtt.begin(MQTT_BROKER_ADDRESS, MQTT_PORT, network);
  Serial.print("ESP32 - Conectando a MQTT");

  while (!mqtt.connect(MQTT_CLIENT_ID)) {
    Serial.print(".");
    delay(100);
  }

  if (!mqtt.connected()) {
    Serial.println("\nESP32 - Error al conectar con MQTT!");
    return;
  }

  Serial.println("\nESP32 - Conectado a MQTT!");
  colors[0] = 0;
  colors[1] = 255;
  colors[2] = 0;
  setLedColor(); // Verde (conectado) 
}

void sendToMQTT() {
  int pre_colors[3];
  pre_colors[0] = colors[0];
  pre_colors[1] = colors[1];
  pre_colors[2] = colors[2];

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo la hora!");
    return;
  }

  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  int sensorValue = 50; 

  // Publicar estado con "retain" activado
  StaticJsonDocument<200> statusMsg;
  statusMsg["timestamp"] = timestamp;
  statusMsg["status"] = 1;
  char statusBuffer[512];
  
  // Verificar la serialización de statusMsg
  serializeJson(statusMsg, statusBuffer);
  Serial.println("Estado serializado:");
  Serial.println(statusBuffer);  // Ver el contenido serializado antes de publicar

  mqtt.publish(TOPIC_STATUS, statusBuffer);  // Con retain activado

  // Publicar medida con "retain" activado
  StaticJsonDocument<200> measureMsg;
  measureMsg["timestamp"] = timestamp;
  measureMsg["value"] = sensorValue;
  char measureBuffer[512];
  
  // Verificar la serialización de measureMsg
  serializeJson(measureMsg, measureBuffer);
  Serial.println("Medida serializada:");
  Serial.println(measureBuffer);  // Ver el contenido serializado antes de publicar

  mqtt.publish(TOPIC_MEDIDA, measureBuffer);  // Con retain activado

  Serial.println("ESP32 - Datos enviados a MQTT:");
  Serial.print("- Estado: "); Serial.println(statusBuffer);
  Serial.print("- Medida: "); Serial.println(measureBuffer);

  colors[0] = 0;
  colors[1] = 0;
  colors[2] = 255;
  setLedColor(); // Verde (conectado)
  delay(250);
  colors[0] = pre_colors[0];
  colors[1] = pre_colors[1];
  colors[2] = pre_colors[2];
  setLedColor(); // Verde (conectado)



}

// Función para cambiar color del LED RGB
void setLedColor() {
  led.setPixelColor(0, led.Color(colors[0], colors[1], colors[2]));  // Cambiar color del primer LED
  led.show();
}

// Verifica si la hora ha sido sincronizada con NTP
bool timeSynced() {
  struct tm timeinfo;
  return getLocalTime(&timeinfo);
}

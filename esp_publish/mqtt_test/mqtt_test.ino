#include <WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include <Adafruit_NeoPixel.h>  // Librería para LED direccionable

// Configuración WiFi
const char wifi_ssid[] = "LRSS";     
const char wifi_password[] = "LRSS-uah-8342";  

// Configuración MQTT
const char mqtt_broker_address[] = "192.168.190.13";  
const int mqtt_port = 1883;
const char mqtt_client_id[] = "sensor:temperature:001";  

// ID del sensor y paciente
const char sensor_id[] = "sensor:temperature:001";  
const char patient_id[] = "001"; // Se debe definir


const int publish_interval = 500;  

WiFiClient network;
MQTTClient mqtt(256);
unsigned long last_publish_time = 0;

// Configuración del LED direccionable
#define LED_PIN 48  // Ajusta según tu ESP32-S3
#define NUM_LEDS 1  // Número de LEDs en la tira
Adafruit_NeoPixel led = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int colors[] = {1, 0, 0}; // Inicialmente en rojo (no conectado)

// NTP
const char* ntp_server = "pool.ntp.org";
const long gmt_offset = -18000; // UTC-5 (ajusta según zona horaria)
const int daylight_offset = 3600; 

void setup() {
  Serial.begin(115200);
  led.begin();
  led.show();  // Apagar LED al inicio
  setLedColor();  // Rojo (no conectado)

  // Conexión WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("ESP32 - Conectando a Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");

  // Sincronizar hora con NTP
  configTime(gmt_offset, daylight_offset, ntp_server);
  Serial.println("Sincronizando con NTP...");
  while (!time_synced()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada!");

  // Conectar a MQTT
  if (connect_to_mqtt(mqtt_client_id, mqtt_broker_address, mqtt_port, network) > 0) {
    Serial.println("\nESP32 - Error al conectar con MQTT!");
  } else {
    Serial.println("\nESP32 - Conectado a MQTT!");
    colors[0] = 0; colors[1] = 1; colors[2] = 0;  // Verde (conectado)
    setLedColor();
  }
}

void loop() {
  mqtt.loop();

  if (millis() - last_publish_time > publish_interval) {
    send_to_mqtt("temperature", 50.0);
    last_publish_time = millis();
  }
}

int connect_to_mqtt(const char* client_id, const char* broker_address, int port, WiFiClient& net) {
  mqtt.begin(broker_address, port, net);
  Serial.print("ESP32 - Conectando a MQTT");

  while (!mqtt.connect(client_id)) {
    Serial.print(".");
    delay(100);
  }

  if (!mqtt.connected()) {    
    return -1;
  }
  return 0;
}

void send_to_mqtt(const char* measurement, float value) {
  int pre_colors[3] = {colors[0], colors[1], colors[2]};
  colors[0] = 0; colors[1] = 0; colors[2] = 1;  // Azul (enviando datos)
  setLedColor();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo la hora!");
    return;
  }

  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  StaticJsonDocument<200> mqtt_msg;
  mqtt_msg["timestamp"] = timestamp;
  mqtt_msg["sensor_status"] = 1;
  mqtt_msg["zone"] = 1;
  mqtt_msg["value"] = value;
  mqtt_msg["priority"] = 0;
  
  char msg_buffer[1024];
  serializeJson(mqtt_msg, msg_buffer);
  Serial.println("Estado serializado:");

  char topic[200];
  sprintf(topic, "uci/patients/patient-%s/%s/1", patient_id, measurement);
  
  mqtt.publish(topic, msg_buffer);
  Serial.println("ESP32 - Datos enviados a MQTT:");
  Serial.print("- Estado: "); Serial.println(msg_buffer);

  delay(250);
  colors[0] = pre_colors[0]; colors[1] = pre_colors[1]; colors[2] = pre_colors[2];  // Restaurar color anterior
  setLedColor();
}

// Función para cambiar color del LED RGB
void setLedColor() {
  led.setPixelColor(0, led.Color(colors[0], colors[1], colors[2]));  
  led.show();
}

// Verifica si la hora ha sido sincronizada con NTP
bool time_synced() {
  struct tm timeinfo;
  return getLocalTime(&timeinfo);
}


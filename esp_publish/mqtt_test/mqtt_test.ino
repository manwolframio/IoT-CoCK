#include <WiFi.h>
#include "mqtt.h"
#include <Adafruit_NeoPixel.h>  // Librería para LED direccionable
#include "time.h"


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

// Defunicion de objetos para el programa

WiFiClient network;
MQTTClient mqtt(256);
unsigned long last_publish_time = 0;

// Configuración del LED direccionable
#define LED_PIN 48
#define NUM_LEDS 1
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
  if (connect_to_mqtt(mqtt, mqtt_client_id, mqtt_broker_address, mqtt_port, network) > 0) {
    Serial.println("\nESP32 - Error al conectar con MQTT!");
  } else {
    Serial.println("\nESP32 - Conectado a MQTT!");
    colors[0] = 0; colors[1] = 1; colors[2] = 0;  // Verde (conectado)
    setLedColor();
  }

}

void loop() {
  static int pre_colors[3];
  mqtt.loop();
  
  if (millis() - last_publish_time > publish_interval) {

    pre_colors[0] = colors[0]; pre_colors[1] = colors[1]; pre_colors[2] = colors[2];  // Restaurar color anterior
    
    colors[0] = 0; colors[1] = 0; colors[2] = 1;  // Azul (enviando datos)
    setLedColor();

    send_to_mqtt(mqtt, patient_id,"temperature",50.0,1,0,1,1);
    
    colors[0] = pre_colors[0]; colors[1] = pre_colors[1]; colors[2] = pre_colors[2];  // Restaurar color anterior
    setLedColor();

    last_publish_time = millis();
  }
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


#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "mqtt_cock.h"
#include "rfidReader.h"

#define RST_PIN   4
#define SS_PIN    5
#define SCK_PIN   18
#define MOSI_PIN  2
#define MISO_PIN  19
#define PATIENT_ID_SIZE_VAL 20

MFRC522 mfrc522(SS_PIN, RST_PIN);

const char wifi_ssid[] = "IoTAP";
const char wifi_password[] = "iot-cock";
const char mqtt_broker_address[] = "192.168.50.28";
const int mqtt_port = 1883;
const char mqtt_client_id[] = "sensor:temperature:001";

const int publish_interval = 1000;
WiFiClient network;
MQTTClient mqtt(256);

unsigned long last_publish_time = 0;
unsigned long last_mqtt_check = 0;
const unsigned long mqtt_check_interval = 2000;

#define LED_PIN 48
#define NUM_LEDS 1
Adafruit_NeoPixel led = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int colors[] = {1, 0, 0};

const char* ntp_server = "192.168.100.1";
byte patient_id[18];
char patient_id_char[PATIENT_ID_SIZE_VAL];

// BMP180
Adafruit_BMP085 bmp;

void setLedColor() {
    led.setPixelColor(0, led.Color(colors[0], colors[1], colors[2]));  
    led.show();
}

bool time_synced() {
    struct tm timeinfo;
    return getLocalTime(&timeinfo);
}

void byte_to_char_array(byte *byteBuffer, char *charBuffer, size_t charSize) {
    if (charSize == 0) return;

    size_t src = 0;
    size_t dst = 0;
    while (src < (charSize - 1) && byteBuffer[src] != '\0') {
        if (byteBuffer[src] != ':') {
            charBuffer[dst] = (char)byteBuffer[src];
            dst++;
        }
        src++;
    }

    charBuffer[dst] = '\0';
    Serial.println(charBuffer);
}

void setup() {
    // Inicializar Serial
    Serial.begin(115200);

    // Inicializar SPI y RFID
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    mfrc522.PCD_Init();

    // Inicializar LED
    led.begin();
    led.show();
    setLedColor();

    // Inicializar I2C para BMP180 en pines fijos
    Wire.begin(8, 9);
    
    if (!bmp.begin()) {
        Serial.println("No se detectó BMP180. Reiniciando...");
        delay(1000);
        ESP.restart();
    }
    Serial.println("BMP180 inicializado correctamente.");

    // Conexión WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    Serial.print("ESP32 - Conectando a Wi-Fi");
    int failcount = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        failcount ++;
        if (failcount > 10) {
            ESP.restart();
        }
    }
    Serial.println("\nWi-Fi Conectado!");

    // Sincronizar NTP
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", ntp_server);
    Serial.print("Sincronizando con NTP");
    while (!time_synced()) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nHora sincronizada!");

    // Leer RFID
    MFRC522::MIFARE_Key key;
    Serial.println("\nEsperando tarjeta RFID!");
    int result = -99;
    byte block = 4;
    byte bufferSize = sizeof(patient_id);
    while (result < 1){
        for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
        result = process_rfid_wait_for_card(mfrc522, block, patient_id, bufferSize, key);
        if (result == 1) {
            Serial.println("Lectura correcta.");
            Serial.print("Datos leídos: ");
            Serial.write(patient_id, 16);
            Serial.println();
        } else if (result == -1) {
            Serial.println("Lectura fallida.");
        } else if (result == -2) {
            Serial.println("Error de autenticación.");
        }
    }
    byte_to_char_array(patient_id, patient_id_char, PATIENT_ID_SIZE_VAL);

    // Conexión MQTT
    if (connect_to_mqtt(mqtt, mqtt_client_id, mqtt_broker_address, mqtt_port, network) > 0) {
        Serial.println("\nESP32 - Error al conectar con MQTT!");
    } else {
        Serial.println("\nESP32 - Conectado a MQTT!");
        colors[0] = 0; colors[1] = 1; colors[2] = 0;
        setLedColor();
    }
}

void loop() {
    static int pre_colors[3];

    mqtt.loop();

    // Verificar conexión MQTT cada 2 segundos
    if (millis() - last_mqtt_check > mqtt_check_interval) {
        last_mqtt_check = millis();
        if (!mqtt.connected()) {
            Serial.println("MQTT desconectado. Intentando reconectar...");
            if (connect_to_mqtt(mqtt, mqtt_client_id, mqtt_broker_address, mqtt_port, network) == 0) {
                Serial.println("Reconectado a MQTT.");
                colors[0] = 0; colors[1] = 1; colors[2] = 0;
                setLedColor();
            } else {
                Serial.println("Fallo al reconectar a MQTT.");
            }
        }
    }

    // Transmitir datos cada segundo
    if (millis() - last_publish_time > publish_interval) {
        pre_colors[0] = colors[0]; pre_colors[1] = colors[1]; pre_colors[2] = colors[2];

        colors[0] = 0; colors[1] = 0; colors[2] = 1;
        setLedColor();

        // Publicar temperatura BMP180
        float temp = bmp.readTemperature();
        send_to_mqtt(mqtt, patient_id_char, "BMP180_Temperature_C", temp, 1, 0, 1, 1);

        // Publicar presión BMP180 en hPa
        float pressure = bmp.readPressure() / 100.0;
        send_to_mqtt(mqtt, patient_id_char, "BMP180_Pressure_hPa", pressure, 1, 0, 1, 1);

        // Publicar altitud estimada
        float altitude = bmp.readAltitude(1013.25);
        send_to_mqtt(mqtt, patient_id_char, "BMP180_Altitude_m", altitude, 1, 0, 1, 1);

        colors[0] = pre_colors[0]; colors[1] = pre_colors[1]; colors[2] = pre_colors[2];
        setLedColor();

        last_publish_time = millis();
    }
}

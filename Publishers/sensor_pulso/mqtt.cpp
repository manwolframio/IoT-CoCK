# include "mqtt.h"

int connect_to_mqtt(MQTTClient &mqtt,const char* client_id, const char* broker_address, int port, WiFiClient& net) {
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

void send_to_mqtt(MQTTClient &mqtt,const char* patient_id,const char* measurement, float value, int zone, int alarm, int priority, int status) {
      
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Error obteniendo la hora!");
      return;
    }
  
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
  
    StaticJsonDocument<200> mqtt_msg;
    mqtt_msg["timestamp"] = timestamp;
    mqtt_msg["sensor_status"] = status;
    mqtt_msg["zone"] = zone;
    mqtt_msg["value"] = value;
    mqtt_msg["priority"] = priority;
    mqtt_msg["alarm"] = alarm;
    
    char msg_buffer[1024];
    serializeJson(mqtt_msg, msg_buffer);
    Serial.println("Estado serializado:");
  
    char topic[200];
    sprintf(topic, "uci/patients/patient-%s/%s/1", patient_id, measurement);
    
    mqtt.publish(topic, msg_buffer);
    Serial.println("ESP32 - Datos enviados a MQTT:");
    Serial.print("- Estado: "); Serial.println(msg_buffer);
  
    delay(250);
}
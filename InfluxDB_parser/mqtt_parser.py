import paho.mqtt.client as mqtt
import ssl
import queue
import time
import select
import json
import os
from influxdb import InfluxDBClient
import warnings
import requests
from requests.packages.urllib3.exceptions import InsecureRequestWarning
from datetime import datetime, timedelta

warnings.simplefilter('ignore', InsecureRequestWarning)
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

MQTT_BROKER = os.getenv("MQTT_BROKER", "192.168.205.193")
MQTT_PORT = int(os.getenv("MQTT_PORT", 8883))
MQTT_USER = os.getenv("MQTT_USER", "iot")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "iot")
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "uci/patients/#")

INFLUXDB_HOST = os.getenv("INFLUXDB_HOST", "192.168.205.193")
INFLUXDB_PORT = int(os.getenv("INFLUXDB_PORT", 8086))
INFLUXDB_USER = os.getenv("INFLUXDB_USER", "iot")
INFLUXDB_PASSWORD = os.getenv("INFLUXDB_PASSWORD", "iot")
INFLUXDB_DATABASE = os.getenv("INFLUXDB_DATABASE", "mi_base")
INFLUXDB_SSL = os.getenv("INFLUXDB_SSL", "true").lower() == "true"
VERIFY_SSL = os.getenv("INFLUXDB_VERIFY_SSL", "false").lower() == "true"

influx_client = InfluxDBClient(
    host=INFLUXDB_HOST,
    port=INFLUXDB_PORT,
    username=INFLUXDB_USER,
    password=INFLUXDB_PASSWORD,
    database=INFLUXDB_DATABASE,
    ssl=INFLUXDB_SSL,
    verify_ssl=VERIFY_SSL
)

message_queue = queue.Queue()

mqtt_client = mqtt.Client(client_id="Parser-Global")

mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)

mqtt_client.tls_set(ca_certs=None, certfile=None, keyfile=None,
                    cert_reqs=ssl.CERT_NONE, tls_version=ssl.PROTOCOL_TLS_CLIENT)
mqtt_client.tls_insecure_set(True)

def on_message(client, userdata, msg):
    try:
        topic = msg.topic
        message = msg.payload.decode()
        print(f"\nRecibido en [{topic}]: {message}")
        message_queue.put((topic, message))
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

mqtt_client.on_message = on_message

def start_mqtt():
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.subscribe(MQTT_TOPIC)
    mqtt_client.socket().setblocking(False)
    print(f"Suscrito a MQTT TLS en {MQTT_BROKER}:{MQTT_PORT} como '{MQTT_USER}'")

start_mqtt()

def convert_to_influx(topic, message):
    try:
        parts = topic.split("/")
        if len(parts) < 5:
            print(f"Topic inválido: {topic}")
            return None

        raw_patient_id = parts[2]
        patient_id = raw_patient_id.replace("patient-", "")
        sensor_measure = parts[3]


        data = json.loads(message)
        zone_code = data.get("zone","NULL")

        raw_status = data.get("sensor_status", 0)
        sensor_status = "ok" if raw_status == 1 else "fail"

        raw_timestamp = data.get("timestamp")

        # Si no viene, coges UTC actual
        timestamp_dt = datetime.strptime(raw_timestamp, "%Y-%m-%d %H:%M:%S") - timedelta(hours=2) if raw_timestamp else datetime.utcnow()
       
        influx_data = [
            {
                "measurement": "sensor_data",
                "tags": {
                    "patient_id": patient_id,
                    "sensor_status": sensor_status,
                    "sensor_measure": sensor_measure,
                    "sensor_priority": int(data.get("priority", 0)),
                    "sensor_zone": zone_code
                },
                "fields": {
                    "sensor_measurement": float(data.get("value", 0.0))
                },
                "time": timestamp_dt
            }
        ]

        return influx_data

    except Exception as e:
        print(f"Error generando datos para InfluxDB: {e}")
        return None

# --- Bucle principal ---
while True:
    read_sockets, _, _ = select.select([mqtt_client.socket()], [], [], 1)

    if read_sockets:
        mqtt_client.loop_read()

    try:
        topic, message = message_queue.get_nowait()
        influx_data = convert_to_influx(topic, message)
        
        if influx_data:
            influx_client.write_points(influx_data)
            print(f"\nDatos insertados en InfluxDB: {influx_data}")

    except queue.Empty:
        pass

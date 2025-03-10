import paho.mqtt.client as mqtt
import queue
import time
import select
import json
from influxdb import InfluxDBClient
import warnings
import requests

from requests.packages.urllib3.exceptions import InsecureRequestWarning

# Deshabilitar el warning de HTTPS sin verificación
warnings.simplefilter('ignore', InsecureRequestWarning)
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)


# Configuración del broker MQTT
MQTT_BROKER = "192.168.190.13"
MQTT_PORT = 1883
MQTT_TOPIC = "uci/patients/#"

# Configuración de InfluxDB con HTTPS
INFLUXDB_HOST = "192.168.190.13"  # Cambia esto si tu servidor está en otro host
INFLUXDB_PORT = 8086  # Asegúrate de que este puerto está habilitado para HTTPS
INFLUXDB_USER = "iot"
INFLUXDB_PASSWORD = "iot"
INFLUXDB_DATABASE = "mi_base"
INFLUXDB_SSL = True  # Habilitar SSL (HTTPS)
VERIFY_SSL = False  # Cambiar a True si el certificado es válido

# Conectar con InfluxDB usando HTTPS
influx_client = InfluxDBClient(
    host=INFLUXDB_HOST,
    port=INFLUXDB_PORT,
    username=INFLUXDB_USER,
    password=INFLUXDB_PASSWORD,
    database=INFLUXDB_DATABASE,
    ssl=INFLUXDB_SSL,
    verify_ssl=VERIFY_SSL
)

# Cola para almacenar los mensajes MQTT
message_queue = queue.Queue()

# Configurar el cliente MQTT
mqtt_client = mqtt.Client(client_id="Parser-Global")

# Función que maneja los mensajes recibidos
def on_message(client, userdata, msg):
    try:
        topic = msg.topic  # Obtener el tópico del mensaje
        message = msg.payload.decode()  # Decodificar el mensaje JSON
        print(f"\nRecibido en [{topic}]: {message}")
        
        # Agregar el mensaje a la cola
        message_queue.put((topic, message))

    except Exception as e:
        print(f"Error procesando mensaje: {e}")

mqtt_client.on_message = on_message

# Conectar al broker y suscribirse
def start_mqtt():
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.subscribe(MQTT_TOPIC)
    mqtt_client.socket().setblocking(False)  # Para usar select sin bloquear
    print("Suscrito a MQTT")

start_mqtt()

# Función para convertir el mensaje en formato InfluxDB
def convert_to_influx(topic, message):
    try:
        # Descomponer el tópico: /uci/patients/<patient_id>/<sensor_measure>/<sensor_zone>/
        parts = topic.split("/")
        if len(parts) < 5:
            print(f"Topic inválido: {topic}")
            return None
        
        patient_id = parts[2]   # Extraer el patient_id
        sensor_measure = parts[3]  # Tipo de medida (ej: heartrate)
        sensor_zone = parts[4]  # Zona del cuerpo (ej: wrist, chest)

        # Convertir el mensaje JSON a un diccionario
        data = json.loads(message)

        # Extraer campos del JSON
        timestamp = data.get("timestamp", "")
        sensor_status = data.get("sensor_status", "unknown")
        sensor_value = float(data.get("value", 0.0))
        sensor_priority = int(data.get("priority", 0))

        # Crear estructura de InfluxDB
        influx_data = [
            {
                "measurement": "sensor_data",
                "tags": {
                    "patient_id": patient_id,
                    "sensor_status": sensor_status,
                    "sensor_measure": sensor_measure,
                    "sensor_priority": sensor_priority,
                    "sensor_zone": sensor_zone
                },
                "fields": {
                    "sensor_measurement": sensor_value
                },
                "time": timestamp
            }
        ]

        return influx_data

    except Exception as e:
        print(f"Error generando datos para InfluxDB: {e}")
        return None

# Bucle principal utilizando select
while True:
    # Usamos select para esperar eventos en el socket de MQTT sin bloquear
    read_sockets, _, _ = select.select([mqtt_client.socket()], [], [], 1)

    if read_sockets:
        mqtt_client.loop_read()  # Procesar mensaje MQTT si hay datos

    try:
        topic, message = message_queue.get_nowait()
        influx_data = convert_to_influx(topic, message)
        
        if influx_data:
            # Insertar en InfluxDB
            influx_client.write_points(influx_data)
            print(f"\nDatos insertados en InfluxDB: {influx_data}")

    except queue.Empty:
        pass  # La cola está vacía, no hay mensajes nuevos


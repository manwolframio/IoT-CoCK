import paho.mqtt.client as mqtt
import ssl
import json

# Configuración del broker TLS
BROKER = "127.0.0.1"  # IP de tu broker
PORT = 8883           # Puerto estándar para MQTT sobre TLS

# Credenciales de autenticación
USERNAME = "iot"
PASSWORD = "iot"

# Identificadores
PATIENT_ID = "002"
SENSOR_ID = "001"
MEASUREMENT = "heartrate"

# Construcción del topic
TOPIC = f"uci/patients/patient-{PATIENT_ID}/{MEASUREMENT}/sensor:{MEASUREMENT}:{SENSOR_ID}"

# Callback para conexión
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker MQTT con TLS (insecure)")
        client.subscribe(TOPIC)
        print(f"Suscrito al topic '{TOPIC}'")
    else:
        print(f"Fallo en la conexión: Código {rc}")

# Callback para mensajes recibidos
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en {msg.topic}: {msg.payload.decode()}")
    
    try:
        payload_json = json.loads(msg.payload.decode())
        print("Payload JSON:", payload_json)
    except json.JSONDecodeError:
        print("El payload no es un JSON válido.")

# Inicializar cliente MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Configurar autenticación
client.username_pw_set(USERNAME, PASSWORD)

# Configurar TLS sin verificar certificado (modo insecure)
client.tls_set(ca_certs=None, certfile=None, keyfile=None, cert_reqs=ssl.CERT_NONE, tls_version=ssl.PROTOCOL_TLS_CLIENT)
client.tls_insecure_set(True)

# Conectar al broker
client.connect(BROKER, PORT, 60)

# Bucle para recibir mensajes
client.loop_forever()

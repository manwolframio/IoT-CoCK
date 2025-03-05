import paho.mqtt.client as mqtt
import json  # Importar el módulo json para procesar el payload como JSON
# Dirección del broker y puerto
BROKER_HOST = "127.0.0.1"  # IP del broker
BROKER_PORT = 1883         # Puerto por defecto para MQTT

# El ID del cliente
CLIENT_ID = "CamaUCI:1"  # Cambia este ID a uno único

# Callback cuando el cliente se conecta
def on_connect(client, userdata, flags, rc):
    print("Conectado con código de resultado:", rc)
    client.subscribe("sensors/#")  # Se suscribe al tema 'sensor:heartrate:001/medida'

# Callback cuando se recibe un mensaje
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en {msg.topic}: {msg.payload.decode()}")
    
    try:
        # Intentamos cargar el payload como JSON
        payload_json = json.loads(msg.payload.decode())
        
        # Ahora imprimimos los campos específicos del JSON
        print(payload_json)
    except json.JSONDecodeError:
        # Si el mensaje no es un JSON válido, simplemente lo imprimimos como texto
        print("El payload no es un JSON válido.")


# Función principal para iniciar el nodo suscriptor
def start_subscriber():
    # Crear cliente MQTT con un ID personalizado
    client = mqtt.Client(client_id=CLIENT_ID)
    
    # Asignar los callbacks
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Conectar al broker
    client.connect(BROKER_HOST, BROKER_PORT, 60)

    # Iniciar el loop para escuchar mensajes
    print(f"Conectado al broker {BROKER_HOST}:{BROKER_PORT} con Client ID '{CLIENT_ID}' y esperando mensajes...")
    client.loop_forever()

if __name__ == "__main__":
    start_subscriber()


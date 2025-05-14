import paho.mqtt.client as mqtt
import ssl
import json
import time
import random
import argparse
from datetime import datetime
import threading

# --- Función para generar valores según la medida ---
def generate_value(measurement):
    if measurement == "heartrate":
        return random.uniform(60.0, 100.0)
    elif measurement == "temperature":
        return random.uniform(36.0, 38.5)
    elif measurement == "spo2":
        return random.uniform(90.0, 100.0)
    elif measurement == "bloodpressure":
        return random.uniform(80.0, 140.0)
    else:
        return random.uniform(0.0, 100.0)

# --- Función de publicación en un hilo ---
def publisher_thread(broker, port, username, password, patient_id, sensor_id, measurement):
    topic = f"uci/patients/patient-{patient_id}/{measurement}/sensor:{measurement}:{sensor_id}"

    client = mqtt.Client()
    client.username_pw_set(username, password)

    client.tls_set(ca_certs=None, certfile=None, keyfile=None,
                   cert_reqs=ssl.CERT_NONE, tls_version=ssl.PROTOCOL_TLS_CLIENT)
    client.tls_insecure_set(True)

    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print(f"[{measurement}] Conectado a {broker}:{port} como '{username}', topic: {topic}")
        else:
            print(f"[{measurement}] Error de conexión: {rc}")

    client.on_connect = on_connect

    client.connect(broker, port, 60)
    client.loop_start()

    try:
        while True:
            value = generate_value(measurement)
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

            payload = {
                "timestamp": timestamp,
                "sensor_status": 1,
                "zone": 2,
                "value": round(value, 3),
                "alarm": 0
            }

            payload_str = json.dumps(payload)
            result = client.publish(topic, payload_str)
            status = result[0]
            if status == 0:
                print(f"[{measurement}] Publicado en '{topic}': {payload_str}")
            else:
                print(f"[{measurement}] Error publicando")

            time.sleep(1)

    except KeyboardInterrupt:
        print(f"[{measurement}] Interrumpido")
    finally:
        client.loop_stop()
        client.disconnect()

# --- Argumentos por línea de comandos ---
parser = argparse.ArgumentParser(description="MQTT Multi-Sensor Publisher with TLS (Insecure)")
parser.add_argument("--ip", type=str, default="127.0.0.1", help="Broker IP address")
parser.add_argument("--user", type=str, default="iot", help="MQTT username")
parser.add_argument("--password", type=str, default="iot", help="MQTT password")
parser.add_argument("--id", type=str, default="001", help="Sensor ID and Patient ID")
parser.add_argument("--measurement", type=str, nargs="+", default=["heartrate"], help="Measurement types (space-separated list: heartrate spo2 temperature etc.)")

args = parser.parse_args()

# --- Lanzar un hilo por cada measurement ---
threads = []

for measurement in args.measurement:
    t = threading.Thread(target=publisher_thread, args=(args.ip, 8883, args.user, args.password, args.id, args.id, measurement))
    t.start()
    threads.append(t)

# --- Esperar a que todos los hilos terminen (Ctrl+C para interrumpir) ---
try:
    for t in threads:
        t.join()
except KeyboardInterrupt:
    print("Interrumpido por el usuario")

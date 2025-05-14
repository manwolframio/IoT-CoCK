import paho.mqtt.client as mqtt
import ssl
import queue
import time
import select
import json
import warnings
import requests
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import QTimer
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from requests.packages.urllib3.exceptions import InsecureRequestWarning

# Deshabilitar warnings HTTPS sin verificación
warnings.simplefilter('ignore', InsecureRequestWarning)
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

# Configuración del broker MQTT
MQTT_BROKER = "127.0.0.1"
MQTT_PORT = 8883  # Puerto TLS

MQTT_TOPIC = "uci/patients/#"

# Credenciales
USERNAME = "iot"
PASSWORD = "iot-user"

# Cola para almacenar los mensajes MQTT
message_queue = queue.Queue()

# Configurar el cliente MQTT
mqtt_client = mqtt.Client(client_id="Parser-Global")

# Autenticación
mqtt_client.username_pw_set(USERNAME, PASSWORD)

# TLS cifrado sin verificar certificados (insecure)
mqtt_client.tls_set(ca_certs=None, certfile=None, keyfile=None,
                    cert_reqs=ssl.CERT_NONE,
                    tls_version=ssl.PROTOCOL_TLS_CLIENT)
mqtt_client.tls_insecure_set(True)

# Función que maneja los mensajes recibidos
def on_message(client, userdata, msg):
    try:
        topic = msg.topic
        message = msg.payload.decode()
        print(f"\nRecibido en [{topic}]: {message}")
        message_queue.put((topic, message))
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

mqtt_client.on_message = on_message

# Conectar al broker y suscribirse
def start_mqtt():
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.subscribe(MQTT_TOPIC)
    mqtt_client.socket().setblocking(False)
    print(f"Suscrito a MQTT con TLS (insecure) en {MQTT_BROKER}:{MQTT_PORT}")

start_mqtt()

# Función para parsear el mensaje
def parse_message_data(topic, message):
    try:
        parts = topic.split("/")
        if len(parts) < 5:
            print(f"Topic inválido: {topic}")
            return None
        
        patient_id = parts[2]
        sensor_measure = parts[3]
        sensor_zone = parts[4]

        data = json.loads(message)

        timestamp = data.get("timestamp", "")
        sensor_status = data.get("sensor_status", "unknown")
        sensor_value = float(data.get("value", 0.0))
        sensor_priority = int(data.get("priority", 0))

        parsed_data = {
            "timestamp": timestamp,
            "sensor_status": sensor_status,
            "sensor_value": sensor_value,
            "sensor_priority": sensor_priority,
            "patient_id": patient_id,
            "sensor_measure": sensor_measure,
            "sensor_zone": sensor_zone
        }

        return parsed_data

    except Exception as e:
        print(f"Error procesando datos: {e}")
        return None

class MqttPlotter(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.graphs = {}

    def initUI(self):
        self.setWindowTitle('Real-time Sensor Data')
        self.setGeometry(100, 100, 1200, 800)

        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)
        
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(1000)

    def create_graph(self, topic):
        figure = Figure()
        canvas = FigureCanvas(figure)
        ax = figure.add_subplot(111)
        
        graph_data = {
            'figure': figure,
            'canvas': canvas,
            'ax': ax,
            'x_data': [],
            'y_data': []
        }
        
        self.graphs[topic] = graph_data
        self.layout.addWidget(canvas)

    def update_plot(self):
        try:
            topic, message = message_queue.get_nowait()

            if topic not in self.graphs:
                self.create_graph(topic)

            parsed_data = parse_message_data(topic, message)

            if parsed_data:
                graph_data = self.graphs[topic]

                timestamp = parsed_data["timestamp"]
                sensor_value = parsed_data["sensor_value"]

                graph_data['x_data'].append(timestamp)
                graph_data['y_data'].append(sensor_value)

                if len(graph_data['x_data']) > 12:
                    graph_data['x_data'] = graph_data['x_data'][-12:]
                    graph_data['y_data'] = graph_data['y_data'][-12:]

                ax = graph_data['ax']
                ax.clear()

                ax.plot(graph_data['x_data'], graph_data['y_data'], linewidth=6)

                ax.set_xlabel('Timestamp', fontsize=36)
                ax.set_ylabel('Measurement', fontsize=36)
                ax.set_title(f'Real-time Sensor Data ({topic})', fontsize=36)

                plt.setp(ax.get_xticklabels(), rotation=45, ha="right", fontsize=36)
                plt.setp(ax.get_yticklabels(), fontsize=36)

                ax.grid(True)
                graph_data['canvas'].draw()

        except queue.Empty:
            pass  # No hay mensajes nuevos

app = QtWidgets.QApplication([])
window = MqttPlotter()
window.show()

# Bucle principal no bloqueante
while True:
    read_sockets, _, _ = select.select([mqtt_client.socket()], [], [], 1)

    if read_sockets:
        mqtt_client.loop_read()

    app.processEvents()

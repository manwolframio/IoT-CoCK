import paho.mqtt.client as mqtt
import ssl
import queue
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
MQTT_BROKER = "192.168.50.28"
MQTT_PORT = 8883  # Puerto TLS
MQTT_TOPIC = "uci/patients/#"

# Credenciales
USERNAME = "iot"
PASSWORD = "iot-user"

# Cola para almacenar los mensajes MQTT
message_queue = queue.Queue()

# Estado de conexión
is_connected = False
is_subscribed = False

# Configurar el cliente MQTT
mqtt_client = mqtt.Client(client_id="Parser-Global")
mqtt_client.username_pw_set(USERNAME, PASSWORD)
mqtt_client.tls_set(ca_certs=None, certfile=None, keyfile=None,
                    cert_reqs=ssl.CERT_NONE,
                    tls_version=ssl.PROTOCOL_TLS_CLIENT)
mqtt_client.tls_insecure_set(True)

# Callback al recibir mensaje
def on_message(client, userdata, msg):
    try:
        topic = msg.topic
        message = msg.payload.decode()
        print(f"\nRecibido en [{topic}]: {message}")
        message_queue.put((topic, message))
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

# Callback al conectar
def on_connect(client, userdata, flags, rc):
    global is_connected, is_subscribed
    if rc == 0:
        print(f"Conexión exitosa a MQTT Broker: {MQTT_BROKER}")
        is_connected = True
        client.subscribe(MQTT_TOPIC)
        is_subscribed = True
        print(f"Suscrito a {MQTT_TOPIC}")
    else:
        print(f"Falló la conexión, código: {rc}")
        is_connected = False
        is_subscribed = False

# Callback al desconectar
def on_disconnect(client, userdata, rc):
    global is_connected, is_subscribed
    print("Desconectado de MQTT Broker")
    is_connected = False
    is_subscribed = False

mqtt_client.on_connect = on_connect
mqtt_client.on_disconnect = on_disconnect
mqtt_client.on_message = on_message

def start_mqtt():
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        print(f"Intentando conexión inicial a {MQTT_BROKER}:{MQTT_PORT}...")
    except Exception as e:
        print(f"Error en conexión inicial: {e}")

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

        # Etiqueta de estado
        self.status_label = QtWidgets.QLabel("Estado MQTT: Desconectado")
        self.status_label.setStyleSheet("color: red; font-size: 14pt;")
        self.layout.addWidget(self.status_label)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(1000)

        # Comprobación de conexión
        self.connection_checker = QTimer()
        self.connection_checker.timeout.connect(self.check_connection)
        self.connection_checker.start(5000)

    def check_connection(self):
        global is_connected, is_subscribed
        if not is_connected:
            print("MQTT no conectado, intentando reconectar...")
            try:
                mqtt_client.reconnect()
            except Exception as e:
                print(f"Error intentando reconectar: {e}")
        elif not is_subscribed:
            print("No suscrito, intentando re-suscripción...")
            try:
                mqtt_client.subscribe(MQTT_TOPIC)
                is_subscribed = True
            except Exception as e:
                print(f"Error en re-suscripción: {e}")

        # Actualizar etiqueta
        if is_connected:
            self.status_label.setText("Estado MQTT: Conectado")
            self.status_label.setStyleSheet("color: green; font-size: 14pt;")
        else:
            self.status_label.setText("Estado MQTT: Desconectado")
            self.status_label.setStyleSheet("color: red; font-size: 14pt;")

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
        while not message_queue.empty():
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

                    ax.plot(graph_data['x_data'], graph_data['y_data'], linewidth=2)

                    ax.set_xlabel('Timestamp', fontsize=10)
                    ax.set_ylabel('Measurement', fontsize=10)
                    ax.set_title(f'Real-time Sensor Data ({topic})', fontsize=12)

                    plt.setp(ax.get_xticklabels(), rotation=45, ha="right", fontsize=8)
                    plt.setp(ax.get_yticklabels(), fontsize=8)

                    ax.grid(True)
                    graph_data['figure'].tight_layout()
                    graph_data['canvas'].draw()

            except Exception as e:
                print(f"Error actualizando gráfico: {e}")

# Inicio
app = QtWidgets.QApplication([])
window = MqttPlotter()
window.show()

start_mqtt()
mqtt_client.loop_start()
app.exec_()

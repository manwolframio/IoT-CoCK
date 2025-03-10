import paho.mqtt.client as mqtt
import queue
import time
import select
import json
from influxdb import InfluxDBClient
import warnings
import requests
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import QTimer
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

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

class MqttPlotter(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        
        self.graphs = {}  # Diccionario para almacenar las gráficas por tópico

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
            
            influx_data = convert_to_influx(topic, message)
            
            if influx_data:
                # Insertar en InfluxDB
                influx_client.write_points(influx_data)
                print(f"\nDatos insertados en InfluxDB: {influx_data}")

                graph_data = self.graphs[topic]
                
                timestamp = influx_data[0]["time"]
                sensor_value = influx_data[0]["fields"]["sensor_measurement"]
                
                graph_data['x_data'].append(timestamp)
                graph_data['y_data'].append(sensor_value)

                # Mostrar solo los últimos 12 datos
                if len(graph_data['x_data']) > 12:
                    graph_data['x_data'] = graph_data['x_data'][-12:]
                    graph_data['y_data'] = graph_data['y_data'][-12:]
                
                ax = graph_data['ax']
                ax.clear()
                
                ax.plot(graph_data['x_data'], graph_data['y_data'], linewidth=6)  # Aumentar el ancho de línea
                
                ax.set_xlabel('Timestamp', fontsize=36)  # Aumentar tamaño de los valores del eje x
                ax.set_ylabel('Measurement', fontsize=36)  # Aumentar tamaño de los valores del eje y
                ax.set_title(f'Real-time Sensor Data ({topic})', fontsize=36)  # Aumentar tamaño del título
                
                # Orientar los ticks del eje x a 45 grados y aumentar su tamaño
                plt.setp(ax.get_xticklabels(), rotation=45, ha="right", fontsize=36)
                
                # Aumentar tamaño de los valores del eje y
                plt.setp(ax.get_yticklabels(), fontsize=36)
                
                # Agregar cuadrícula al gráfico
                ax.grid(True)
                
                graph_data['canvas'].draw()

        except queue.Empty:
            pass  # La cola está vacía, no hay mensajes nuevos

app = QtWidgets.QApplication([])
window = MqttPlotter()
window.show()

# Bucle principal utilizando select
while True:
    # Usamos select para esperar eventos en el socket de MQTT sin bloquear
    read_sockets, _, _ = select.select([mqtt_client.socket()], [], [], 1)

    if read_sockets:
        mqtt_client.loop_read()  # Procesar mensaje MQTT si hay datos

    app.processEvents()

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-dev \
    build-essential \
    libssl-dev \
    curl \
    && apt-get clean
RUN pip3 install --no-cache-dir \
    paho-mqtt \
    influxdb \
    requests

WORKDIR /app
COPY mqtt_parser.py /app/mqtt_parser.py

CMD ["python3", "mqtt_parser.py"]


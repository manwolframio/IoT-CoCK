mkdir -p ./certs && cd ./certs
openssl genrsa -out influxdb1.key 2048
openssl req -new -x509 -days 365 -key influxdb1.key -out influxdb1.crt -subj "/CN=influxdb-IOT-MUIT"

openssl genrsa -out influxdb2.key 2048
openssl req -new -x509 -days 365 -key influxdb2.key -out influxdb2.crt -subj "/CN=influxdb-IOT-MUIT"

openssl genrsa -out nginx.key 2048
openssl req -new -x509 -days 365 -key nginx.key -out nginx.crt -subj "/CN=NGNIX-IOT-MUIT"
cd ..


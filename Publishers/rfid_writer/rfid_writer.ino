#include <SPI.h>
#include <MFRC522.h>
#include "rfidReader.h"  // Incluir las funciones RFID modularizadas

#define RST_PIN   4   // Reset del MFRC522
#define SS_PIN    5   // SS (SDA) del MFRC522
#define SCK_PIN   18  // Reloj SPI
#define MOSI_PIN  23  // Datos hacia el MFRC522
#define MISO_PIN  19  // Datos hacia el ESP32

#define CAMA_START 1
#define CAMA_STOP 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("Acerca una tarjeta RFID para escribir los datos...");
}

void loop() {
    static int cama = CAMA_START;

    if (cama <= CAMA_STOP) {
        // Detectar tarjeta
        if (detect_rfid(mfrc522) == -1 || read_card_serial(mfrc522) == -1) {
            return;
        }

        Serial.println("Tarjeta detectada! Escribiendo...");

        // Generar clave predeterminada
        MFRC522::MIFARE_Key key;
        for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;  // Clave predeterminada
        
        byte block = 4;  // Bloque donde escribir los datos

        // Crear el mensaje y convertirlo a bytes con relleno de '-'
        String mensaje = ":patient-" + String(cama) + ":";
        byte dataBlock[16];
        byte bufferSize = 16;
        string_to_byte(dataBlock, bufferSize, mensaje);

        // Autenticación del sector
        if (auth_rfid(mfrc522, block, key) == -1) {
            Serial.println("Error de autenticación");
            return;
        }

        // Escritura en la tarjeta
        if (write_rfid(mfrc522, block, dataBlock) == -1) {
            Serial.println("Error al escribir");
            return;
        }

        Serial.print("Datos escritos correctamente: ");
        Serial.write(dataBlock, 16);  // Mostrar en Serial sin espacios vacíos
        Serial.println();

        // Detener la comunicación con la tarjeta
        stop_rfid(mfrc522);

        Serial.println("Retira la tarjeta y vuelve a acercarla para verificar la escritura...");
        
        // Esperar a que la tarjeta sea retirada
        while (detect_rfid(mfrc522) == 1) {
            delay(100);
        }

        Serial.println("Tarjeta retirada. Acerca nuevamente para leer los datos.");

        // Esperar a que la tarjeta sea detectada nuevamente
        while (detect_rfid(mfrc522) == -1 || read_card_serial(mfrc522) == -1) {
            delay(100);
        }

        Serial.println("Verificando escritura...");

        // Leer los datos del bloque escrito
        byte buffer[18];
        bufferSize = sizeof(buffer);

        // Autenticación para la lectura
        if (auth_rfid(mfrc522, block, key) == -1) {
            Serial.println("Error de autenticación en lectura");
            return;
        }

        // Leer los datos del bloque
        if (read_rfid_data(mfrc522, block, buffer, bufferSize) == -1) {
            Serial.println("Error al leer");
            return;
        }

        // Mostrar los datos leídos sin espacios extra
        Serial.print("Datos leídos: ");
        Serial.write(buffer, 16);
        Serial.println();

        // Finalizar comunicación con la tarjeta
        stop_rfid(mfrc522);

        // Avanzar a la siguiente cama
        cama++;

    } else {
        Serial.println("No quedan camas por escribir, código finalizado.");
        while (true) { delay(2000); }  // Detener el programa
    }
}

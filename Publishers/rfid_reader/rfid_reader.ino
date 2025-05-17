#include <SPI.h>
#include <MFRC522.h>
#include "rfidReader.h"

#define RST_PIN   4
#define SS_PIN    5
#define SCK_PIN   18
#define MOSI_PIN  2
#define MISO_PIN  19

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    mfrc522.PCD_Init();
    Serial.println("Acerca una tarjeta RFID para leerla");
}

void loop() {
    byte block = 4;
    byte buffer[18];
    byte bufferSize = sizeof(buffer);

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    // Llama a la función que espera hasta detectar una tarjeta y procesa una vez
    int result = process_rfid_wait_for_card(mfrc522, block, buffer, bufferSize, key);

    // Según el resultado, puedes decidir qué hacer (opcional)
    if (result == 1) {
        Serial.println("Lectura correcta.");
        Serial.print("Datos leídos: ");
        Serial.write(buffer, 16);
        Serial.println();
    } else if (result == -1) {
        Serial.println("Lectura fallida.");
    } else if (result == -2) {
        Serial.println("Error de autenticación.");
    }

    // Esperar un tiempo antes de volver a esperar otra tarjeta
    delay(1000);
}

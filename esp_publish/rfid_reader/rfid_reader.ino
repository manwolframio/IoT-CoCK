#include <SPI.h>
#include <MFRC522.h>
#include "rfidReader.h"  // Incluir las funciones RFID modularizadas

#define RST_PIN   4   // Reset del MFRC522
#define SS_PIN    5   // SS (SDA) del MFRC522
#define SCK_PIN   18  // Reloj SPI
#define MOSI_PIN  23  // Datos hacia el MFRC522
#define MISO_PIN  19  // Datos hacia el ESP32

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("Acerca una tarjeta RFID para leerla");
  
}

void loop() {

  byte block = 4;
  byte buffer[18];
  byte bufferSize = sizeof(buffer);
  
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;  // Clave predeterminada

  while (1){
    if(detect_rfid(mfrc522) >=1 && read_card_serial(mfrc522)>=1){    
      // Autenticación para la lectura
      if (auth_rfid(mfrc522, block, key)) {
        // Leer los datos del bloque
        if (read_rfid_data(mfrc522, block, buffer, bufferSize)) {
          Serial.print("Datos leídos: ");
          Serial.write(buffer, 16);
          Serial.println();
        }
      }
      // Finalizar comunicación con la tarjeta
      stop_rfid(mfrc522);
      delay(500);
    }
  }
}

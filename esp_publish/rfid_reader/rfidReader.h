# ifndef RFID_READER_H
# define RFID_READER_H

#include <SPI.h>
#include <MFRC522.h>

int detect_rfid(MFRC522 &mfrc522);

int read_card_serial(MFRC522 &mfrc522);

int string_to_byte(byte *buffer, byte &bufferSize, String &message);

int auth_rfid(MFRC522 &mfrc522, byte block, MFRC522::MIFARE_Key &Key);

int write_rfid(MFRC522 &mfrc522, byte &block, byte *dataBlock);

int read_rfid_data(MFRC522 &mfrc522, byte block, byte *buffer, byte &bufferSize);

int stop_rfid(MFRC522 &mfrc522);

#endif // RFID_H



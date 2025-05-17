#include "rfidReader.h"

int detect_rfid(MFRC522 &mfrc522){
    if (mfrc522.PICC_IsNewCardPresent()){
        return 1;
    }
    return -1;
}

int read_card_serial(MFRC522 &mfrc522){
    if (mfrc522.PICC_ReadCardSerial()){
        return 1;
    }
    return -1;
}

int string_to_byte(byte *buffer, byte &bufferSize, String &message){
    memset(buffer, ' ', bufferSize);  // Rellenar con '-'
    message.toCharArray((char*)buffer, bufferSize);
    return 1;
}

int auth_rfid(MFRC522 &mfrc522, byte block, MFRC522::MIFARE_Key &Key){
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &Key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
        return -1;
    }
    return 1;
}

int write_rfid(MFRC522 &mfrc522, byte &block, byte *dataBlock){
    if (mfrc522.MIFARE_Write(block, dataBlock, 16) != MFRC522::STATUS_OK) {
        return -1;
    }
    return 1;
}

int read_rfid_data(MFRC522 &mfrc522, byte block, byte *buffer, byte &bufferSize){
    if (mfrc522.MIFARE_Read(block, buffer, &bufferSize) != MFRC522::STATUS_OK) {
        return -1;
    }
    return 1;
}

int stop_rfid(MFRC522 &mfrc522){
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return 1;
}

int process_rfid_wait_for_card(MFRC522 &mfrc522, byte block, byte *buffer, byte bufferSize, MFRC522::MIFARE_Key key) {
    while (!(detect_rfid(mfrc522) >= 1 && read_card_serial(mfrc522) >= 1)) {
        delay(100);
    }

    int result = -99;  // Valor por defecto

    if (auth_rfid(mfrc522, block, key)) {
        if (read_rfid_data(mfrc522, block, buffer, bufferSize)) {
            if(check_format(buffer, bufferSize)){
                result = 1;
            }
            else{
                result = -1;
            }           

        } else {
            result = -1;
        }
    } else {
        result = -2;
    }

    // Siempre parar la comunicación después del proceso completo
    stop_rfid(mfrc522);
    return result;
}

bool check_format(byte *buffer, byte bufferSize) {
    if (bufferSize == 0) {
        return false;
    }

    return (buffer[0] == ':');
}



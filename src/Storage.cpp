#include <Storage.h>
#include "SD.h"

Storage::Storage(){
    Serial.println("Initializing SD card...");
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        mount_success = false;
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        mount_success = false;
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
        mount_success = false;
        return;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    if (cardSize < 15000){
        Serial.println("Invalid Card Capacity!");
        mount_success = false;
        return;
    }
}

Storage::write_data(String filename, String data){
    
}
Storage storage;
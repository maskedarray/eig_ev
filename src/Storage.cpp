#include <Storage.h>
#include "SD.h"

//TODO: change the curr_read_file variable from filename to filename with directory (add "/" with file name)

bool Storage::init_storage(){
    Serial.println("init_storage() -> storage.cpp -> Initializing SD card...");
    mount_success = false;
    if(!SD.begin()){
        Serial.println("init_storage() -> storage.cpp -> Card Mount Failed");
        return mount_success;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("init_storage() -> storage.cpp -> No SD card attached");
        return mount_success;
    }

    Serial.print("init_storage() -> storage.cpp -> SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
        return mount_success;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("init_storage() -> storage.cpp -> SD Card Size: %lluMB\n", cardSize);
    if (cardSize < CARD_SIZE_LIMIT_MB){
        Serial.println("init_storage() -> storage.cpp -> Invalid Card Capacity!");
        return mount_success;
    }

    if (SD.exists("/config.txt")){
        resume = true;
        Serial.println("init_storage() -> storage.cpp -> Previous data found!")
        File file = SD.open("/config.txt");
        char c = file.read();
        String temp;
        while(c != '$'){
            temp += c;
            c = file.read();
        }
        curr_read_file = temp;          //update the file name to read from
        c = file.read();
        temp = "";
        while(c != '$'){
            temp += c;
            c = file.read();
        }
        curr_read_pos = atol(temp);     //update the position to read
        file.close();
    }
    else {
        resume = false;
        curr_read_pos = 0;
        curr_read_file = "";    //TODO: handle case when read is called before write
    }
    mount_success = true;
    return mount_success;
}



bool Storage::write_data(String timenow, String data){
    bool write_success = false;
    if(mount_success){
        if((CARD_SIZE_LIMIT_MB - SD.usedBytes()/1048576) < 1024) {   //less than 1GB space left
            this->remove_oldest_file();
        }
        String path = "/" + timenow + ".txt";
        File file = SD.open(path, FILE_WRITE);
        if(!file){
            Serial.println("write_data() -> storage.cpp -> Failed to open file for writing");
            return write_success;
        }
        if(file.print(data)){
            Serial.println("write_data() -> storage.cpp -> File written");
            write_success = true;
        } else {
            Serial.println("write_data() -> storage.cpp -> Write failed");
        }
        if (!resume){
            String name = file.name();
            curr_read_file = name;
            resume = true;
        }
        file.close();
        return write_success;
    } else{
        Serial.println("write_data() -> storage.cpp -> Storage mount failed or not mounted! Try again");
        return write_success;
    }
}



void Storage::remove_oldest_file(){
    Serial.println("remove_oldest_file() -> storage.cpp -> Space is low! Removing oldest file..");
    File file = SD.open("/");
    int oldest = 99999999;          //initialized so that first file detected is oldest file
    while(file.openNextFile()){     //convert filename to number and compare to get the oldest
        String name = file.name();
        if (name != "config.txt"){  //do not check the config.txt file
            int temp = name.toInt();
            if (temp < oldest)
                oldest = temp;
        }
    }
    file.close();
    String path = "/" + String(oldest) + ".txt";    //convert number back to filename
    SD.remove(path);
    Serial.printf("remove_oldest_file() -> storage.cpp -> File removed at %s", path.c_str);
}



String Storage::read_data(){
    String path = "/" + curr_read_file;
    Serial.printf("read_data() -> storage.cpp -> Reading file: %s\n", path);
    File file = fs.open(path);
    if(!file){
        Serial.println("read_data() -> storage.cpp -> Failed to open file for reading");
        return 0;
    }
    String toread = "";
    bool readSt = 0;
    curr_chunk_size = 0;
    file.seek(curr_read_pos);
    for (int i = 0; i< 30; i++){
        if (file.available()){
            char c = file.read();
            if(c == '<'){
                readSt = 1;
                curr_chunk_size = 1;
                break;
            }
        }
    }
    if (!readSt){
        Serial.println("read_data() -> storage.cpp -> No valid data found!");
        return 0;
    }
    else{
        while(true){
            if (file.available()){
                char c = file.read();
                curr_chunk_size++;
                if (c != '>'){   
                    toread += c;
                }
                else{
                    break;
                }
            }
            else {
                Serial.println("read_data() -> storage.cpp -> Data in file corrupted!");
                curr_chunk_size = 0;
                return 0;
            }
        }
    }
    file.close();
    Serial.println("read_data() -> storage.cpp -> Parsed successfully");
    return toread;
}



void Storage::mark_data(){
    String path = "/" + curr_read_file;
    Serial.println("mark_data() -> storage.cpp -> Marking current chunk of data");
    File file = fs.open(path);
    file.seek(curr_read_pos);
    if(file.read() == '<'){
        file.seek(curr_read_pos);
        file.write('!');
        curr_read_pos += curr_chunk_size;
    }
    
    String name = file.name();
    if (file.size() - curr_read_pos < 10){      //check if this is the end of file
        int temp = name.toInt();
        temp += 1;
        name = String(temp) + ".txt";           //go to next file
        curr_read_pos = 0;
    }
    file.open("/config.txt");                   //save the filename and read position to the config.txt file
    file.write(name);
    file.println("$");
    file.write(String(curr_read_pos));
    file.println("$");
    file.close();
}



Storage storage;
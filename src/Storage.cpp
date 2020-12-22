#include <Storage.h>
#include <SPIFFS.h>
//TODO: change the curr_read_file variable from filename to filename with directory (add "/" with file name)

bool Storage::init_storage(){
    Serial.println("init_storage() -> storage.cpp -> Initializing SD card...");
    mount_success = false;

    if(!SPIFFS.begin()){
        Serial.println("init_storage() -> storage.cpp -> Card Mount Failed");
        return mount_success;
    }

    uint64_t cardSize = SPIFFS.totalBytes() / (1024 * 1024);
    Serial.printf("init_storage() -> storage.cpp -> SD Card Size: %lluMB\r\n", cardSize);
    if (cardSize < CARD_SIZE_LIMIT_MB){
        Serial.println("init_storage() -> storage.cpp -> Invalid Card Capacity!");
        return mount_success;
    }

    if (SPIFFS.exists("/config.txt")){
        resume = true;
        Serial.println("init_storage() -> storage.cpp -> Previous data found!");
        File file = SPIFFS.open("/config.txt");
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
        char temp2[20];
        temp.toCharArray(temp2, sizeof(temp2));
        curr_read_pos = atol(temp2);     //update the position to read
        file.close();
        Serial.print("File is: ");
        Serial.println(curr_read_file);
        Serial.print("Position is: ");
        Serial.println(curr_read_pos);
    }
    else {
        resume = false;
        Serial.println("init_storage() -> storage.cpp -> Previous data not found!");
        curr_read_pos = 0;
        curr_read_file = "";    //TODO: handle case when read is called before write
    }
    mount_success = true;
    return mount_success;
}



bool Storage::write_data(String timenow, String data){
    bool write_success = false;
    if(mount_success){
        if((CARD_SIZE_LIMIT_MB - SPIFFS.usedBytes()/1048576) < 1024) {   //less than 1GB space left
            Serial.println("write_data() -> storage.cpp -> Space low! Deleting oldest file");
            this->remove_oldest_file();
        }
        String path = "/" + timenow + ".txt";
        File file = SPIFFS.open(path, FILE_APPEND);
        if(!file){
            Serial.println("write_data() -> storage.cpp -> Failed to open file for writing");
            return write_success;
        }
        if(file.print("<")){
            file.print(data);
            file.println(">");
            Serial.println("write_data() -> storage.cpp -> File written");
            write_success = true;
        } else {
            Serial.println("write_data() -> storage.cpp -> Write failed");
        }
        if (!resume){                       //if this is the first time system has started, create config.txt and update variables
            String name = file.name();
            curr_read_file = name;
            File file2 = SPIFFS.open("/config.txt",FILE_APPEND);
            file2.print(curr_read_file);
            file2.println("$");
            file2.print(curr_read_pos);
            file2.println("$");
            file2.close();
            Serial.println("write_data() -> storage.cpp -> config.txt created!");
            Serial.print("File name: ");
            Serial.println(curr_read_file);
            Serial.print("Position: ");
            Serial.println(curr_read_pos);
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
    File file = SPIFFS.open("/");
    int oldest = 99999999;          //initialized so that first file detected is oldest file
    while(file.openNextFile()){     //convert filename to number and compare to get the oldest
        String name = file.name();
        if (name != "/config.txt"){  //do not check the config.txt file
            name.remove(0,1);
            int temp = name.toInt();
            if (temp < oldest)
                oldest = temp;
        }
    }
    file.close();
    String path = "/" + String(oldest) + ".txt";    //convert number back to filename
    SPIFFS.remove(path);
    Serial.printf("remove_oldest_file() -> storage.cpp -> File removed at %s", path.c_str());
}



String Storage::read_data(){
    Serial.print("read_data() -> storage.cpp -> Reading file: ");
    Serial.println(curr_read_file);
    File file = SPIFFS.open(curr_read_file);
    if(!file){
        Serial.println("read_data() -> storage.cpp -> Failed to open file for reading");
        return "";
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
                curr_read_pos += i;
                break;
            }
        }
    }
    if (!readSt){
        Serial.println("read_data() -> storage.cpp -> No valid data found!");
        return "";
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
                return "";
            }
        }
    }
    file.close();
    Serial.println("read_data() -> storage.cpp -> Parsed successfully");
    return toread;
}



void Storage::mark_data(){
    Serial.println("mark_data() -> storage.cpp -> Marking current chunk of data");
    File file = SPIFFS.open(curr_read_file, FILE_APPEND);
    if(!file){
        Serial.println("mark_data() -> storage.cpp -> Failed to open file for marking");
        return;
    }

    if(!file.seek(curr_read_pos)){
        Serial.println("not working");
    }
    char c = file.read();
    if(c == '<'){
        Serial.println("mark_data() -> storage.cpp -> Found data for marking");
        file.seek(curr_read_pos);
        file.write('!');
        curr_read_pos += curr_chunk_size;
    } else{
        Serial.print("mark_data() -> storage.cpp -> character placed at current position is: ");
        Serial.println(c);
        Serial.println("mark_data() -> storage.cpp -> Data not found for marking");
        return;
    }
    
    String name = file.name();
    if (file.size() - curr_read_pos < 10){      //check if this is the end of file
        Serial.println("mark_data() -> storage.cpp -> File completed! Moving to next file.");
        name.remove(0,1);
        int temp = name.toInt();
        temp += 1;
        name = "/" + String(temp) + ".txt";           //go to next file //TODO: what if system was off for whole day?
        curr_read_pos = 0;
    }
    file.close();
    file = SPIFFS.open("/config.txt");               //save the filename and read position to the config.txt file
    if(!file){
        Serial.println("mark_data() -> storage.cpp -> Failed to open file for saving config");
        return;
    }
    file.print(name);
    file.println("$");
    file.print(String(curr_read_pos));
    file.println("$");
    file.close();
}



Storage storage;
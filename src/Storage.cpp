#include <Storage.h>
#include <SD.h>

//TODO: while traversing to next file through increment, handle the scenario where increment takes file above 31 or 30 date

/*
 * init_storage initialises storage (SD card)
 * returns true only if the storage is initialized properly.
 */
bool Storage::init_storage(){
    Serial.println(F("init_storage() -> storage.cpp -> Initializing SD card..."));
    mount_success = false;

    if(!SD.begin()){
        Serial.println(F("init_storage() -> storage.cpp -> Card Mount Failed"));
        return mount_success;
    }

    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println(F("init_storage() -> storage.cpp -> No SD card attached"));
        return mount_success;
    }

    Serial.print(F("init_storage() -> storage.cpp -> SD Card Type: "));
    if(cardType == CARD_MMC){
        Serial.println(F("MMC"));
    } else if(cardType == CARD_SD){
        Serial.println(F("SDSC"));
    } else if(cardType == CARD_SDHC){
        Serial.println(F("SDHC"));
    } else {
        Serial.println(F("UNKNOWN"));
        return mount_success;
    }

    uint64_t cardSize = SD.totalBytes() / (1024 * 1024);
    Serial.printf("init_storage() -> storage.cpp -> SD Card Size: %lluMB\r\n", cardSize);

    if (SD.exists("/config.txt")){
        resume = true;
        Serial.println(F("init_storage() -> storage.cpp -> Previous data found!"));
        File file = SD.open("/config.txt", FILE_READ);
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
        Serial.print(F("init_storage() -> storage.cpp -> File is: "));
        Serial.println(curr_read_file);
        Serial.print(F("init_storage() -> storage.cpp -> Position is: "));
        Serial.println(curr_read_pos);
    }
    else {
        resume = false;
        Serial.println(F("init_storage() -> storage.cpp -> Previous data not found!"));
        curr_read_pos = 0;
        curr_read_file = "";    //TODO: handle case when read is called before write
    }
    mount_success = true;
    return mount_success;
}


/*
 * write data function writes data to the storage.
 * - required parameters are the current time in format YYYYMMDD and the string of data
 * - to separate a chunk of data from another, it encapsulates data in '<>'
 * - the first thing it performs is to check if card has free space greater than LOW_SPACE_LIMIT_MB 
 *   if the space is less, delete the oldest file
 * - if this is the first time of writing data, the write data function also creates the config.txt file
 * - return value is true if the write operation is successful
 */
bool Storage::write_data(String timenow, String data){
    bool write_success = false;
    if(mount_success){
        if((CARD_SIZE_LIMIT_MB - SD.usedBytes()/1048576) < LOW_SPACE_LIMIT_MB) {   //check if there is low space.
            Serial.println(F("write_data() -> storage.cpp -> Space low! Deleting oldest file"));
            this->remove_oldest_file();
        }
        String path = "/" + timenow + ".txt";
        File file = SD.open(path, FILE_APPEND);
        if(!file){
            Serial.println(F("write_data() -> storage.cpp -> Failed to open file for writing"));
            return write_success;
        }
        if(file.print("<")){
            file.print(data);
            file.println(F(">"));
            Serial.println(F("write_data() -> storage.cpp -> File written"));
            write_success = true;
        } else {
            Serial.println(F("write_data() -> storage.cpp -> Write failed"));
        }
        if (!resume){                       //if this is the first time system has started, create config.txt and update variables
            String name = file.name();
            curr_read_file = name;
            File file2 = SD.open("/config.txt",FILE_APPEND);
            file2.print(curr_read_file);
            file2.println("$");
            file2.print(curr_read_pos);
            file2.println("$");
            file2.close();
            Serial.println(F("write_data() -> storage.cpp -> config.txt created!"));
            resume = true;
        }
        file.close();
        return write_success;
    } else{
        Serial.println(F("write_data() -> storage.cpp -> Storage mount failed or not mounted! Try again"));
        return write_success;
    }
}


/*
 * remove_oldest_file() function removes the oldest file based on filename.
 * - it converts the filenames to integers and loops over all files to check the smallest.
 * - then the smallest files is removed.
 */
void Storage::remove_oldest_file(){
    Serial.println(F("remove_oldest_file() -> storage.cpp -> Space is low! Removing oldest file.."));
    File file = SD.open("/");
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
    SD.remove(path);
    Serial.print(F("remove_oldest_file() -> storage.cpp -> File removed at: "));
    Serial.println(path);
}


/*
 * read_data() function reads the data starting with < character
 * - it returns data by removing encapsulation "<>"
 * - it also updates the size of current chunk of data.
 * - to check the start of data, it checks the next 30 characters for '<'. if this is not found
 *   then it returns false (failure)
 * - if data start is not at the curr_read_pos, then it also updates the curr_read_pos variable to start of data.
 * - if the starting character is found, then it loops over the data to check the end character '>' till
 *   the end of file //TODO: add for loop here
 * - returns the string without encapsulation "<>" 
 */
String Storage::read_data(){
    Serial.print(F("read_data() -> storage.cpp -> Reading file: "));
    Serial.println(curr_read_file);
    File file = SD.open(curr_read_file, FILE_READ);
    if(!file){
        Serial.println(F("read_data() -> storage.cpp -> Failed to open file for reading"));
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
        Serial.println(F("read_data() -> storage.cpp -> No valid data found!"));
        return "";
    }
    else{
        while(true){    //TODO: infinite loop can cause problems. change with for loop and a const value for max chunk size limit
            if (file.available()){  //check if file has ended
                char c = file.read();
                curr_chunk_size++;
                if (c != '>'){   
                    toread += c;
                }
                else{
                    break;  // '>' found! terminating iterations
                }
            }
            else {  //if end character '>' not found till the end of file then data is corrupted
                Serial.println(F("read_data() -> storage.cpp -> Data in file corrupted!"));
                curr_chunk_size = 0;
                return "";
            }
        }
    }
    file.close();
    Serial.println(F("read_data() -> storage.cpp -> Parsed successfully"));
    return toread;
}


/*
 * mark_data updates the curr_read_pos in config.txt
 * - if the remaining data in file is less than 10 it also updates the filename
 */
void Storage::mark_data(){
    Serial.println(F("mark_data() -> storage.cpp -> Marking current chunk of data"));
    File file = SD.open(curr_read_file, FILE_READ);
    if(!file){
        Serial.println(F("mark_data() -> storage.cpp -> Failed to open file for marking"));
        return;
    }
    file.seek(curr_read_pos);
    char c = file.read();
    if (c == '<'){
        String name = file.name();
        if (file.size() - (curr_read_pos + curr_chunk_size) < 10){      //check if this is the end of file  //TODO: convert 10 to const
            Serial.println(F("mark_data() -> storage.cpp -> File completed! Moving to next file."));
            name.remove(0,1);                             //remove '/' from filename
            int temp = name.toInt();                      //converts the sring to integer up to the point a non-integer character is found
            temp += 1;
            name = "/" + String(temp) + ".txt";           //go to next file //TODO: what if system was off for whole day?
            curr_read_pos = 0;
            curr_read_file = name;
        }
        else{
            curr_read_pos += curr_chunk_size + 1;
        }
        file.close();
        file = SD.open("/config.txt", FILE_WRITE);               //save the filename and read position to the config.txt file
        if(!file){
            Serial.println(F("mark_data() -> storage.cpp -> Failed to open file for saving config"));
            return;
        }
        file.print(name);
        file.println("$");
        file.print(String(curr_read_pos));
        file.println("$");
        file.close();
        Serial.println(F("mark_data() -> storage.cpp -> Data marked and config updated!"));
    }   
    else {  //if the start character is not '<' then terminate
        Serial.println(F("mark_data() -> storage.cpp -> Valid data not found for marking"));
    }
}

/*
 * get_unsent_data returns the data in MBs
 */
long Storage::get_unsent_data(){ 
    String filename;
    long filepos;
    File file = SD.open("/config.txt", FILE_READ);
    {   //read filename and file position from config.txt
        char c = file.read();
        String temp;
        while(c != '$'){
            temp += c;
            c = file.read();
        }
        filename = temp;          //update the file name to read from
        c = file.read();
        temp = "";
        while(c != '$'){
            temp += c;
            c = file.read();
        }
        char temp2[20];
        temp.toCharArray(temp2, sizeof(temp2));
        filepos = atol(temp2);     //update the position to read
    }
    
    file.close();
    file = SD.open(filename);   //read the file from where curent data is being sent to cloud
    long total_bytes;
    total_bytes = file.size() - filepos;        //update total bytes

    int old_file;
    {
        String temp2;
        strcpy(temp2, filename);
        temp2.remove(0,1);
        old_file = temp2.toInt();
    }
    int curr_file;
    {
        String temp2;
        strcpy(temp2, curr_read_file);
        temp2.remove(0,1);
        curr_file = temp2.toInt();
    }

    while(old_file < curr_file){
        old_file += 1;
        filename = "/" + String(old_file) + ".txt";           //go to next file according to date
        if(SD.exists(filename)){
            file = SD.open(filename);
            total_bytes += file.size();
        }
    }
    return total_bytes/1048576;
}


Storage storage;    //storage object created for storing data
#include <Storage.h>
#include <SD.h>
#include <rtc.h>

//TODO: while traversing to next file through increment, handle the scenario where increment takes file above 31 or 30 date

/*
 * init_storage initialises storage (SD card)
 * returns true only if the storage is initialized properly.
 */
bool Storage::init_storage(){
    Serial.println(F("init_storage() -> storage.cpp -> Initializing SD card..."));
    mount_success = false;
    if(!SD.begin()){
        Serial.println(F("init_storage() -> storage.cpp -> Card Initialization Failed"));
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
    Serial.printf("init_storage() -> storage.cpp -> SD Card Size: %lluMB\r\n", cardSize);   //TODO: add a card limit check for better reliability

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
        c = file.read();                //read the \n character
        temp = "";
        while(c != '$'){
            temp += c;
            c = file.read();
        }
        curr_read_pos = atol(temp.c_str());     //update the position to read
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
        curr_read_file = "";    
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
            file.println(">");
            Serial.println(F("write_data() -> storage.cpp -> File written"));
            write_success = true;
        } else {
            Serial.println(F("write_data() -> storage.cpp -> Write failed"));
        }
        if (!resume){                       //if this is the first time system has started, create config.txt and update variables
            String name = file.name();
            curr_read_file = name;
            File file2 = SD.open("/config.txt",FILE_WRITE);
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
 * - in case the file has ended before detection of start character, that means there is no more data to be read in file.
 * - if data start is not at the curr_read_pos, then it also updates the curr_read_pos variable to start of data.
 * - if the starting character is found, then it loops over the data to check the end character '>' 
 *   till the max_chunk_size_b limit
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
        else{
            Serial.println(F("read_data() -> storage.cpp -> File has no data available to be read!"));
            return "";
        }
    }
    if (!readSt){
        Serial.println(F("read_data() -> storage.cpp -> No valid data found!"));
        return "";
    }
    else{
       while(true)
       {
            if (file.available()){  //check if file has ended
                char c = file.read();
                curr_chunk_size++;
                if(curr_chunk_size > MAX_CHUNK_SIZE_B){
                    Serial.println(F("read_data() -> storage.cpp -> Valid data not found for reading!"));
                    curr_chunk_size = 0;
                    return "";
                }
                else if (c != '>'){   
                    toread += c;
                }
                else{
                    break;  // '>' found! terminating iterations
                }
            }
            else {  //if end character '>' not found till the end of file then data is corrupted
                Serial.println(F("read_data() -> storage.cpp -> File ended before data read completed. Data Corrupt!"));
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
void Storage::mark_data(String timenow){
    String curr_write_file = "/" + timenow + ".txt";
    Serial.println(F("mark_data() -> storage.cpp -> Marking current chunk of data"));
    File file = SD.open(curr_read_file, FILE_READ);
    if(!file){
        Serial.println(F("mark_data() -> storage.cpp -> Failed to open file for marking"));
        return;
    }
    file.seek(curr_read_pos);
    char c = file.read();
    if (c == '<'){
        if (file.size() - (curr_read_pos + curr_chunk_size) < MIN_CHUNK_SIZE_B){      //check if this is the end of file 
            file.close();
            Serial.println(F("mark_data() -> storage.cpp -> File completed! Moving to next file."));
            String next_filename = next_file(curr_read_file);
            while(!SD.exists(next_filename) && (next_filename < curr_write_file)){
                next_filename = next_file(next_filename);
            }
            curr_read_pos = 0;
            curr_read_file = next_filename;
        }
        else{
            curr_read_pos += curr_chunk_size + 1;
            file.close();
        }
        file = SD.open("/config.txt", FILE_WRITE);               //save the filename and read position to the config.txt file
        if(!file){
            Serial.println(F("mark_data() -> storage.cpp -> Failed to open file for saving config"));
            return;
        }
        file.print(curr_read_file);
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

String Storage::next_file(String curr_file){
    String syear = curr_file.substring(1,4);
    String smonth = curr_file.substring(5,6);
    String sday = curr_file.substring(7,8);
    int iyear = syear.toInt();
    int imonth = smonth.toInt();
    int iday = sday.toInt();
    String next_file = getNextDay(iyear,imonth,iday);
    next_file = "/" + next_file + ".txt";
    return next_file;
}

/*
 * get_unsent_data returns the data in MBs
 */
long Storage::get_unsent_data(String timenow){ 
    String filename;
    String curr_write_file = "/" + timenow + ".txt";
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
        filepos = atol(temp.c_str());     //update the position to read
    }
    file.close();
    file = SD.open(filename);   //read the file from where curent data is being sent to cloud
    long total_bytes;
    total_bytes = file.size() - filepos;        //update total bytes

    while(filename <= curr_write_file){
        filename = next_file(filename);
        if(SD.exists(filename)){
            file = SD.open(filename);
            total_bytes += file.size();
        }
    }
    return total_bytes/1048576;
}


Storage storage;    //storage object created for storing data
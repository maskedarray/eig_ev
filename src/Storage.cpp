#include <Storage.h>
#include <rtc.h>


/*
 * init_storage initialises storage (SD card)
 * returns true only if the storage is initialized properly.
 */
bool Storage::init_storage(){
    log_i("Initializing SD card... ");
    mount_success = false;
    if(!SD.begin()){
        log_e("Card Initialization Failed ");
        return mount_success;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        log_e("No SD card attached ");
        return mount_success;
    }
    log_d("init_storage() -> storage.cpp -> SD Card Type: ");
    if(cardType == CARD_MMC){
        log_d("MMC ");
    } else if(cardType == CARD_SD){
        log_d("SDSC ");
    } else if(cardType == CARD_SDHC){
        log_d("SDHC ");
    } else {
        log_d("UNKNOWN ");
        return mount_success;
    }

    uint64_t cardSize = SD.totalBytes() / (1024 * 1024);
    log_i("SD Card Size: %lluMB", cardSize);   //TODO: add a card limit check for better reliability

    if (SD.exists("/config.txt")){
        resume = true;
        log_d("Previous data found! ");
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
        log_d("File is: ");
        log_d("%s ", curr_read_file.c_str());
        log_d("Position is: ");
        log_d("%d ", curr_read_pos);
    }
    else {
        resume = false;
        log_w("Previous data not found! ");
        curr_read_pos = FILE_START_POS;
        curr_read_file = "";    
    }

    // New addition to the init function
    if (SD.exists("/APs.txt")) // Check for APs.txt on SD card
    {
        log_i("AP storage found! ");
        APList_exists = true;
    }
    else
    {
        log_e("AP storage not found. Creating new file ");
        File APList = SD.open("/APs.txt", FILE_WRITE);
        APList.print('<');
        APList.print("EiG,12344321");   // Default entry for the APlist
        APList.print('>');
        APList.close();
        APList_exists = true;
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
            log_w("Space low! Deleting oldest file ");
            this->remove_oldest_file();
        }
        String path = "/" + timenow + ".txt";
        File file;
        if(!SD.exists(path)){
            file = SD.open(path, FILE_APPEND);
            if(!file){
                log_e("Failed to open file for writing ");
                return write_success;
            }
            create_header(file);
        }
        else {
            file = SD.open(path, FILE_APPEND); 
            if(!file){
                log_e("Failed to open file for writing ");
                return write_success;
            }
        }  
        if(file.print("<")){
            file.print(data);
            file.println(">");
            log_d("File written");
            write_success = true;
        } else {
            log_e("Write failed ");
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
            log_d("config.txt created! ");
            resume = true;
        }
        file.close();
        return write_success;
    } else{
        log_e("Storage mount failed or not mounted! Try again ");
        return write_success;
    }
}

/**
 * This function adds an access point to the SD card file APs.txt
 */
bool Storage::write_AP(String SSID, String Password) //made with small edits to the write_data function
{
    bool write_success = false;
    if(mount_success){
        String path = "/APs.txt";
        File file;
        if(!SD.exists(path)){
            file = SD.open(path, FILE_APPEND);
            if(!file){
                log_e("Failed to open file for writing ");
                return write_success;
            }
        }
        else {
            file = SD.open(path, FILE_APPEND); 
            if(!file){
                log_e("Failed to open file for writing ");
                return write_success;
            }
        }  
        if(file.print("<")){
            file.print(SSID + ",");
            file.print(Password);
            file.println(">");
            log_d("File written ");
            write_success = true;
        } else {
            log_e("Write failed ");
        }
        file.close();
        return write_success;
    } else{
        log_e("Storage mount failed or not mounted! Try again ");
        return write_success;
    }
}

/**
 * This function clears the APs.txt file and adds new credentials according to
 * the String passed to it. It is called whenever the AP list exceeds a set
 * limit or a saved SSID is being connected to with a different password.
 */
bool Storage::rewrite_storage_APs(String SSID[10], String Password[10])
{
    bool write_success = false;
    int i = 0;
    if(mount_success)
    {
        String path = "/APs.txt";
        File file;
        if(!SD.exists(path)){
            file = SD.open(path, FILE_WRITE);
            if(!file){
                log_e("Failed to open file for writing ");
                return write_success;
            }
        }
        else {
            file = SD.open(path, FILE_WRITE); 
            if(!file){
                log_e("Failed to open file for writing ");
                return write_success;
            }
        }
        while(!SSID[i].isEmpty() && i < 10)
        {
            if(file.print("<"))
            {
                file.print(SSID[i] + ",");
                file.print(Password[i]);
                file.print(">");
                log_d("File written %s and %s ", SSID[i].c_str(), Password[i].c_str());
                i++;
            } 
            else 
            {
                log_e("Write failed ");
                file.close();
                SD.remove(path);
                return false;
            }
        }
        write_success = true;
        file.close();
        return write_success;
    }
    else
    {
        log_e("Storage mount failed or not mounted! Try again ");
        return write_success;
    }
}

/*
 * places CSV header on the file
 */
void Storage::create_header(File file){
    file.println("Time,BSS_ID,Total_Slots,BSS_Voltage,BSS_Current,BSS_Power,BSS_PowerFactor,"
                "S1_B_Slot,S1_B_ID,S1_B_Auth,S1_B_Age,S1_B_Type,S1_B_M_Cylcles,S1_B_U_Cylcles,S1_B_Temp,S1_B_SoC,S1_B_SoH,S1_B_RoD,S1_B_Vol,S1_B_Curr,"
                "S2_B_Slot,S2_B_ID,S2_B_Auth,S2_B_Age,S2_B_Type,S2_B_M_Cylcles,S2_B_U_Cylcles,S2_B_Temp,S2_B_SoC,S2_B_SoH,S2_B_RoD,S2_B_Vol,S2_B_Curr,"
                "S3_B_Slot,S3_B_ID,S3_B_Auth,S3_B_Age,S3_B_Type,S3_B_M_Cylcles,S3_B_U_Cylcles,S3_B_Temp,S3_B_SoC,S3_B_SoH,S3_B_RoD,S3_B_Vol,S3_B_Curr,"
                "S4_B_Slot,S4_B_ID,S4_B_Auth,S4_B_Age,S4_B_Type,S4_B_M_Cylcles,S4_B_U_Cylcles,S4_B_Temp,S4_B_SoC,S4_B_SoH,S4_B_RoD,S4_B_Vol,S4_B_Curr,"
                "S5_B_Slot,S5_B_ID,S5_B_Auth,S5_B_Age,S5_B_Type,S5_B_M_Cylcles,S5_B_U_Cylcles,S5_B_Temp,S5_B_SoC,S5_B_SoH,S5_B_RoD,S5_B_Vol,S5_B_Curr,"
                "S6_B_Slot,S6_B_ID,S6_B_Auth,S6_B_Age,S6_B_Type,S6_B_M_Cylcles,S6_B_U_Cylcles,S6_B_Temp,S6_B_SoC,S6_B_SoH,S6_B_RoD,S6_B_Vol,S6_B_Curr,"
                "S7_B_Slot,S7_B_ID,S7_B_Auth,S7_B_Age,S7_B_Type,S7_B_M_Cylcles,S7_B_U_Cylcles,S7_B_Temp,S7_B_SoC,S7_B_SoH,S7_B_RoD,S7_B_Vol,S7_B_Curr,"
                "S8_B_Slot,S8_B_ID,S8_B_Auth,S8_B_Age,S8_B_Type,S8_B_M_Cylcles,S8_B_U_Cylcles,S8_B_Temp,S8_B_SoC,S8_B_SoH,S8_B_RoD,S8_B_Vol,S8_B_Curr,"
                "S9_B_Slot,S9_B_ID,S9_B_Auth,S9_B_Age,S9_B_Type,S9_B_M_Cylcles,S9_B_U_Cylcles,S9_B_Temp,S9_B_SoC,S9_B_SoH,S9_B_RoD,S9_B_Vol,S9_B_Curr,"
                "S10_B_Slot,S10_B_ID,S10_B_Auth,S10_B_Age,S10_B_Type,S10_B_M_Cylcles,S10_B_U_Cylcles,S10_B_Temp,S10_B_SoC,S10_B_SoH,S10_B_RoD,S10_B_Vol,S10_B_Curr,"
                "S11_B_Slot,S11_B_ID,S11_B_Auth,S11_B_Age,S11_B_Type,S11_B_M_Cylcles,S11_B_U_Cylcles,S11_B_Temp,S11_B_SoC,S11_B_SoH,S11_B_RoD,S11_B_Vol,S11_B_Curr,"
                "S12_B_Slot,S12_B_ID,S12_B_Auth,S12_B_Age,S12_B_Type,S12_B_M_Cylcles,S12_B_U_Cylcles,S12_B_Temp,S12_B_SoC,S12_B_SoH,S12_B_RoD,S12_B_Vol,S12_B_Curr,"
                "S13_B_Slot,S13_B_ID,S13_B_Auth,S13_B_Age,S13_B_Type,S13_B_M_Cylcles,S13_B_U_Cylcles,S13_B_Temp,S13_B_SoC,S13_B_SoH,S13_B_RoD,S13_B_Vol,S13_B_Curr,"
                "S14_B_Slot,S14_B_ID,S14_B_Auth,S14_B_Age,S14_B_Type,S14_B_M_Cylcles,S14_B_U_Cylcles,S14_B_Temp,S14_B_SoC,S14_B_SoH,S14_B_RoD,S14_B_Vol,S14_B_Curr,"
                "S15_B_Slot,S15_B_ID,S15_B_Auth,S15_B_Age,S15_B_Type,S15_B_M_Cylcles,S15_B_U_Cylcles,S15_B_Temp,S15_B_SoC,S15_B_SoH,S15_B_RoD,S15_B_Vol,S15_B_Curr,"
                "S16_B_Slot,S16_B_ID,S16_B_Auth,S16_B_Age,S16_B_Type,S16_B_M_Cylcles,S16_B_U_Cylcles,S16_B_Temp,S16_B_SoC,S16_B_SoH,S16_B_RoD,S16_B_Vol,S16_B_Curr");
    return;
}


/*
 * remove_oldest_file() function removes the oldest file based on filename.
 * - it converts the filenames to integers and loops over all files to check the smallest.
 * - then the smallest files is removed.
 */
void Storage::remove_oldest_file(){
    log_w("Space is low! Removing oldest file.. ");
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
    log_d("File removed at: ");
    log_d("%s ", path);
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
    log_d("Reading file: ");
    log_d("%s ", curr_read_file.c_str());
    File file = SD.open(curr_read_file, FILE_READ);
    if(!file){
        log_e("Failed to open file for reading ");
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
            log_w("File has no data available to be read! ");
            return "";
        }
    }
    if (!readSt){
        log_e("No valid data found! ");
        return "";
    }
    else{
       while(true)
       {
            if (file.available()){  //check if file has ended
                char c = file.read();
                curr_chunk_size++;
                if(curr_chunk_size > MAX_CHUNK_SIZE_B){
                    log_i("Valid data not found for reading! ");
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
                log_e("File ended before data read completed. Data corrupt! ");
                curr_chunk_size = 0;
                return "";
            }
        
        }
    }
    file.close();
    log_d("Parsed successfully ");
    return toread;
}

/**
 * This funtion cycles through the APs stored in the SD card and stores them in
 * a String array. Since the String array parameter degenerates to a pointer it
 * chenges the original referenced parameter and thus returns the list of
 * credentials in two arrays.
 */
void Storage::return_APList(String SSID_List [10], String Password_List[10])
{
    int i = 0;
    while(!SSID_List[i].isEmpty() || !Password_List[i].isEmpty())
    {
        SSID_List[i].clear();
        Password_List[i].clear();
        i++;
    }
    i = 0;
    if(mount_success && APList_exists)
    {
        char temp = '\0';
        int16_t max_iter_limit = 0;
        int16_t i = 0;
        bool read_st = false;
        bool SSID_rd = false;
        bool Password_rd = false;
        File AP = SD.open("/APs.txt", FILE_READ);

        while(1)
        {
            if(AP.available() && !read_st)
            {
                temp = AP.read();
                if(temp == '<' && max_iter_limit < 10)
                {
                    log_d("Start of frame found ");
                    temp = AP.read();
                    max_iter_limit = 0;
                    read_st = true;
                }

                if(max_iter_limit > 10) // could not find the start of frame
                {
                    log_e("Start of frame missing ");
                    return;
                }
                else
                {
                    max_iter_limit++;
                }
            }

            if(AP.available() && read_st)
            {
                while(temp != ',' && max_iter_limit < 40)
                {
                    curr_SSID += temp;
                    temp = AP.read();
                    max_iter_limit++;
                    if(max_iter_limit >= 40) //username greater than assigned limit. Maybe define global variables??
                    {
                        log_e("Invalid username ");
                        break;
                    }
                    if(temp == ',')
                    {
                        SSID_rd = true;
                        log_d("%s ", curr_SSID.c_str());
                    }
                }
                temp = AP.read();
                while(temp != '>' && max_iter_limit < 40)
                {
                    curr_Password += temp;
                    temp = AP.read();
                    max_iter_limit++;
                    if(max_iter_limit >= 40)
                    {
                        log_e("Invalid password ");
                        break;
                    }
                    if(temp == '>')
                    {
                        Password_rd = true;
                        log_d("%s ", curr_Password.c_str());
                    }
                }
            }
            if(SSID_rd && Password_rd) // One frame of credentials has been read
            {
                if(i < 10)
                {
                    SSID_List[i] = curr_SSID;
                    Password_List[i] = curr_Password;
                    i++;
                    log_d("New AP returned ");
                }
                else
                {
                    log_w("Maximum number of APs stored in SD card. Please free up space ");
                }
                curr_SSID = "";
                curr_Password = "";
                Password_rd = false;
                SSID_rd = false;
                read_st = false;
                max_iter_limit = 0;
            }
            if(!AP.available())
            {
                log_d("End of data reached ");
                break;
            }
        }
        AP.close();
        i = 0;
        return;
    }
}

/*
 * mark_data updates the curr_read_pos in config.txt
 * - if the remaining data in file is less than 10 it also updates the filename
 */
void Storage::mark_data(String timenow){
    String curr_write_file = "/" + timenow + ".txt";
    log_d("Marking current chunk of data ");
    File file = SD.open(curr_read_file, FILE_READ);
    if(!file){
        log_e("Failed to open file for marking ");
        return;
    }
    file.seek(curr_read_pos);
    char c = file.read();
    if (c == '<'){  
        if ((file.size() - (curr_read_pos + curr_chunk_size) < MIN_CHUNK_SIZE_B) && (curr_write_file != curr_read_file)){      //check if this is the end of file 
            file.close();
            log_d("File completed! Moving to next file CWF: %s CRF: %s ", curr_write_file.c_str(), curr_read_file.c_str());
            String next_filename = next_file(curr_read_file);
            while(!SD.exists(next_filename) && (next_filename < curr_write_file)){
                next_filename = next_file(next_filename);
            }
            if(SD.exists(next_filename)){
                curr_read_pos = FILE_START_POS;
                curr_read_file = next_filename;
            }
        }
        else{
            curr_read_pos += curr_chunk_size + 1;
            file.close();
        }
        file = SD.open("/config.txt", FILE_WRITE);               //save the filename and read position to the config.txt file
        if(!file){
            log_e("Failed to open file for saving config ");
            return;
        }
        file.print(curr_read_file);
        file.println("$");
        file.print(String(curr_read_pos));
        file.println("$");
        file.close();
        log_d("Data marked and config updated! ");
    }   
    else {  //if the start character is not '<' then terminate
        log_e("Valid data not found for marking ");
    }
}

String Storage::next_file(String curr_file){
    String syear = curr_file.substring(1,5);
    String smonth = curr_file.substring(5,7);
    String sday = curr_file.substring(7,9);
    int iyear = syear.toInt();
    int imonth = smonth.toInt();
    int iday = sday.toInt();
    String next_file = getNextDay(iyear,imonth,iday);
    next_file = "/" + next_file + ".txt";
    return next_file;
}

/*
 * get_unsent_data returns the data in bytes
 */
long Storage::get_unsent_data(String timenow){ 
    String filename;
    String curr_write_file = "/" + timenow + ".txt";
    long filepos;
    File file = SD.open("/config.txt", FILE_READ);
    if(!file){
        log_e("Failed to open config file ");
        return 0;
    }
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
    while(filename < curr_write_file){
        filename = next_file(filename);
        if(SD.exists(filename)){
            file = SD.open(filename);
            total_bytes += file.size() - FILE_START_POS;
            file.close();
        }
    }
    return total_bytes;
}


Storage storage;    //storage object created for storing data
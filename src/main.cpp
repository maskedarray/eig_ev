/* 
 *  make CSV file
 *
 */

#include <Arduino.h>
#include <rtc.h>
#include "SD.h"

String towrite, toread;

void addSlotsData(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,String B_U_Cycles , String B_Temp, String B_SoC, String B_SoH, String B_RoC,String B_Vol ,String B_Curr){
    towrite += B_Slot + "," + B_ID + "," + B_Auth + "," + B_Age + "," + B_Type + "," + B_M_Cycles + "," + 
            B_U_Cycles + "," + B_Temp + "," + B_SoC + "," + B_SoH + "," + B_RoC + "," + B_Vol + "," + B_Curr;
    return;
}

bool readNext(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return 0;
    }

    toread = "";
    bool readSt = 0;
    file.seek(0);

    for (int i = 0; i< 30; i++){
        if (file.available()){
            char c = file.read();
            if(c == '<'){
                readSt = 1;
                break;
            }
        }
    }
    if (!readSt){
        Serial.println("No data found!");
        return 0;
    }
    else{
        while(true){
            if (file.available()){
                char c = file.read();
                if (c != '>'){   
                    toread += c;
                }
                else{
                    break;
                }
            }
            else {
                Serial.println("Data in file not complete");
                return 0;
            }
        }
    }
    Serial.println("Parsed successfully");
    Serial.println(toread);
    file.close();
    return 1;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.println(towrite)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void initSD(){
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
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
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void setup() {

    Serial.begin(115200); //Start Serial monitor
    initSD();
    initRTC();
    {
        const char *temp = "Time,BSS_ID,Total_Slots,BSS_Voltage,BSS_Current,BSS_Power,BSS_PowerFactor,"
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
                        "S16_B_Slot,S16_B_ID,S16_B_Auth,S16_B_Age,S16_B_Type,S16_B_M_Cylcles,S16_B_U_Cylcles,S16_B_Temp,S16_B_SoC,S16_B_SoH,S16_B_RoD,S16_B_Vol,S16_B_Curr \n";
        writeFile(SD, "/ndata.txt", temp);
    }
}

void loop() {
    //we need to place a valid CSV string in towrite string
    towrite = "";
    towrite += getTime() + ",";                 //time
    towrite += String("BSS1715001") + ",";      //BSSID
    towrite += String("16") + ",";              //total slots
    towrite += String("20.273") + ",";              //BSS voltage
    towrite += String("55.781") + ",";              //BSS CURRENT
    towrite += String("7400") + ",";                  //BSS POWER
    towrite += String("0.8") + ",";                 //BSS power factor
    
    //addSlotsData(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,
    //             String B_U_Cycles , String B_Temp, String B_SoC, String B_SoH, String B_RoD,String B_Vol ,String B_Curr)
    addSlotsData("01", "1718953129", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("02", "1718953130", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("03", "1718953131", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("04", "1718953128", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "26.561");towrite += ",";
    addSlotsData("05", "1718953127", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("06", "1718953126", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("07", "1718953125", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("08", "1718953124", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "26.561");towrite += ",";
    addSlotsData("09", "1718953123", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("10", "1718953122", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("11", "1718953121", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("12", "1718953120", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "26.561");towrite += ",";
    addSlotsData("13", "1718953119", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("14", "2718953129", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("15", "1518953129", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "20.561");towrite += ",";
    addSlotsData("16", "1718253129", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12.371", "26.561");
    //Now towrite string contains one valid string of CSV data chunk

    appendFile(SD, "/ndata.txt");
    Serial.println(towrite);
    static long counter = 0;
    counter++;
    Serial.println(counter);
    Serial.println();
    delay(1000);
}
/* 
 *  make json file
 *
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <rtc.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino
BluetoothSerial ESP_BT; //Object for Bluetooth
char BT_incoming[32]; // array of characyters, to read incoming data 
String towrite, toread;

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
    if(file.print(towrite)){
        file.print("\n\n");
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

JsonObject updateSlot(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,String B_U_Cycles , String B_Temp, String B_SoC, String B_SoH, String B_RoC,String B_Vol ,String B_Curr, JsonObject Slot){
    Slot["Battery_Slot"] =String(B_Slot);
    Slot["Battery_ID"] = String(B_ID);
    Slot["Battery_Authorization_ID"] = String(B_Auth); // can be vehicle or BSS
    Slot["Battery_Age"] = String(B_Age);
    Slot["Battery_Type"] = String(B_Type);
    Slot["Battery_Max_Cycles"] = String(B_M_Cycles);
    Slot["Battery_Utilized_Cycles"] = String(B_U_Cycles);
    Slot["Battery_Temperature"] = String(B_Temp);
    Slot["Battery_SoC"] = String(B_SoC);
    Slot["Battery_SoH"] = String(B_SoH);
    Slot["Battery_RoC"] = String(B_RoC);
    Slot["Battery_Volatge"] = String(B_Vol);
    Slot["Battery_Current"] = String(B_Curr);
    return Slot;
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

  ESP_BT.begin("EiG BT"); //Name of your Bluetooth Signal
  Serial.println("Bluetooth Device is Ready to Pair");
  Serial.println("BT ready");
  //initialize sd card and place file header
  initSD();
  //deleteFile(SD, "/ndata.txt");
  //writeFile(SD, "/ndata.txt", "The data contained in this file is not sent\n\n");
  initRTC();

//   uint freeRAM = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
//   int d = 48;
//   while (1){
//     freeRAM = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
//     Serial.println(freeRAM);
//     d = d+1;
//     delay(1000);
//   }
}

void loop() {
  
  // code below will convert the raw data to Json format
  StaticJsonDocument<5200> doc; //DynamicJsonDocument doc(1200);
    doc["Time"] = String(getTime());
    doc["BSS_ID"] = String("BSS22");
    doc["Total_Slots"] = String(16);
    doc["BSS_Voltage"] = String(random(0, 14));
    doc["BSS_Current"] = String(random(0, 20));
    doc["BSS_Power"] = String(random(0, 50));
    doc["BSS_PowerFactor"] = String(random(0, 50));
    
  JsonArray Batteries = doc.createNestedArray("Slots");
  JsonObject Slot1 = Batteries.createNestedObject();  
  JsonObject Slot2 = Batteries.createNestedObject();
  JsonObject Slot3 = Batteries.createNestedObject();
  JsonObject Slot4 = Batteries.createNestedObject();
  JsonObject Slot5 = Batteries.createNestedObject();  
  JsonObject Slot6 = Batteries.createNestedObject();
  JsonObject Slot7 = Batteries.createNestedObject();
  JsonObject Slot8 = Batteries.createNestedObject();
  JsonObject Slot9 = Batteries.createNestedObject();  
  JsonObject Slot10 = Batteries.createNestedObject();
  JsonObject Slot11 = Batteries.createNestedObject();
  JsonObject Slot12 = Batteries.createNestedObject();
  JsonObject Slot13 = Batteries.createNestedObject();  
  JsonObject Slot14 = Batteries.createNestedObject();
  JsonObject Slot15 = Batteries.createNestedObject();
  JsonObject Slot16 = Batteries.createNestedObject();
    
    //updateSlot(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,String B_U_Cycles , String B_Temp, String B_SoC, String B_SoH, String B_RoD,String B_Vol ,String B_Curr, JsonObject Slot){

    Slot1 = updateSlot("1", "112", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot1);
    Slot2 = updateSlot("2", "22", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot2);
    Slot3 = updateSlot("3", "33", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot3);
    Slot4 = updateSlot("4", "44", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "26", Slot4);
    Slot5 = updateSlot("5", "5", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot5);
    Slot6 = updateSlot("6", "6", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot6);
    Slot7 = updateSlot("7", "7", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot7);
    Slot8 = updateSlot("8", "8", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "26", Slot8);
    Slot9 = updateSlot("9", "9", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot9);
    Slot10 = updateSlot("10", "10", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot10);
    Slot11 = updateSlot("11", "11", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot11);
    Slot12 = updateSlot("12", "12", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "26", Slot12);
    Slot13 = updateSlot("13", "13", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot13);
    Slot14 = updateSlot("14", "14", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot14);
    Slot15 = updateSlot("15", "15", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "20", Slot15);
    Slot16 = updateSlot("16", "16", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", "12", "26", Slot16);

  towrite = "";
  //serializeJson(doc, towrite);
  readNext(SD, "/ndata.txt");
  //appendFile(SD, "/ndata.txt");

  static long counter = 0;
  counter++;
  Serial.println(counter);
  Serial.println();
  Serial.println();
  
  //sending over BT
  ESP_BT.print("%S%"); //end byte //start byte
  serializeJson(doc,ESP_BT);
  ESP_BT.print("%S%"); //end byte
  //ESP_BT.println(); //end byte
  
  if (ESP_BT.available()){ //Check if we receive anything from Bluetooth
      for(int i=0; i<4; i++){ // reading four bytes only, need to send data to esp in a standardized form (delimiters)
         BT_incoming[i] = ESP_BT.read();
      }
      Serial.print("Received:"); Serial.println(BT_incoming);
  }
  
  delay(2000);
}
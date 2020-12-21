/* 
 *  make json file
 *
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <rtc.h>
#include <Storage.h>

String towrite, toread;


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


void setup() {
    Serial.begin(115200); //Start Serial monitor
    initRTC();

    if(storage.init_storage()){
        Serial.println("main() -> main.cpp -> storage initialization success!");
    }
    else{
        while(1){
            Serial.println("main() -> main.cpp -> storage initialization failed!");
            delay(1000);
        }
    }
    while(1){delay(1000);}
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
    //readNext(SD, "/ndata.txt");
    //appendFile(SD, "/ndata.txt");

    static long counter = 0;
    counter++;
    Serial.println(counter);
    Serial.println();
    Serial.println();
     

    
    delay(2000);
}
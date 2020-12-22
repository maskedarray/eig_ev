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
    Slot["BS"] =String(B_Slot);                             //battery slot
    Slot["ID"] = String(B_ID);                      //batery id
    Slot["AID"] = String(B_Auth);      //battery authorization id
    Slot["AG"] = String(B_Age);                    //battery age
    Slot["TY"] = String(B_Type);                  //battery type
    Slot["MC"] = String(B_M_Cycles);        //battery max cycles
    Slot["UC"] = String(B_U_Cycles);   //utilized cycles
    Slot["TP"] = String(B_Temp);           //temperature
    Slot["SC"] = String(B_SoC);                    //state of charge
    Slot["SH"] = String(B_SoH);                    //sate of health
    Slot["RH"] = String(B_RoC);                    //battery roh
    Slot["V"] = String(B_Vol);                //voltage
    Slot["C"] = String(B_Curr);               //battery current
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

}

void loop() {
  
    // code below will convert the raw data to Json format
    StaticJsonDocument<5200> doc; //DynamicJsonDocument doc(1200);
    doc["T"] = String(getTime());          //time
    doc["BID"] = String("BSS22");          //battery id
    doc["TS"] = String(16);                //total slots
    doc["BV"] = String(random(0, 14));     //BSS voltage
    doc["BC"] = String(random(0, 20));     //BSS current
    doc["BP"] = String(random(0, 50));     //BSS power
    doc["BPF"] = String(random(0, 50));    //BSS power factor
        
    JsonArray Batteries = doc.createNestedArray("S");
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
    serializeJson(doc, towrite);
    storage.write_data(getTime2(), towrite);
    String temp2 = storage.read_data();
    //storage.mark_data();
    Serial.println(temp2);

    //readNext(SD, "/ndata.txt");
    //appendFile(SD, "/ndata.txt");

    static long counter = 0;
    counter++;
    Serial.println(counter);
    Serial.println();
    Serial.println();
     

    
    delay(5000);
}
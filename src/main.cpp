
#include <Arduino.h>
#include <rtc.h>
#include <Storage.h>
#include <can.h>
#include <bluetooth.h>
#include "esp32-mqtt.h"

String towrite, toread;

void addSlotsData(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,String B_U_Cycles , 
                    String B_Temp, String B_SoC, String B_SoH, String B_RoC,String B_Vol ,String B_Curr){
    towrite += B_Slot + "," + B_ID + "," + B_Auth + "," + B_Age + "," + B_Type + "," + B_M_Cycles + "," + 
            B_U_Cycles + "," + B_Temp + "," + B_SoC + "," + B_SoH + "," + B_RoC + "," + B_Vol + "," + B_Curr;
    return;
}



void setup() {
    Serial.begin(115200); //Start Serial monitor
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, OUTPUT);
    setupCloudIoT();
    initRTC();
    /*if(storage.init_storage()){
        Serial.println("main() -> main.cpp -> storage initialization success!");
    }
    else{
        while(1){
            Serial.println("main() -> main.cpp -> storage initialization failed!");
            delay(1000);
        }
    }*/
}
unsigned long lastMillis = 0;
String CloudData = "";
void loop() {
    //we need to place a valid CSV string in towrite string
    if(WiFi.status() == WL_CONNECTED){digitalWrite(2, 1);}
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
    mqtt->loop();
    delay(10);  // <- fixes some issues with WiFi stability
    if (!mqttClient->connected()) {
        connect();
    }
    if (mqttClient->connected()) {
        Serial.println("*****");
        Serial.println(publishTelemetry(towrite));
        Serial.println(ESP.getFreeHeap());
        //Serial.println(publishTelemetry("hi"));
        Serial.println("*****");
    }
    //storage.write_data(getTime2(), towrite);
    static long counter = 0;
    counter++;
    Serial.println(counter);
    Serial.println();
    delay(1000);
}
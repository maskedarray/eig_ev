#include <Arduino.h>
#include <rtc.h>
#include <Storage.h>
#include <ESPWiFi.h>
#include <can.h>
#include <bluetooth.h>
#include <cmdlib.h>


String towrite, toread;

void addSlotsData(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,String B_U_Cycles , 
                    String B_Temp, String B_SoC, String B_SoH, String B_RoC,String B_Vol ,String B_Curr){
    towrite += B_Slot + "," + B_ID + "," + B_Auth + "," + B_Age + "," + B_Type + "," + B_M_Cycles + "," + 
            B_U_Cycles + "," + B_Temp + "," + B_SoC + "," + B_SoH + "," + B_RoC + "," + B_Vol + "," + B_Curr;
    return;
}


void test1(){
    //init
    storage.init_storage(); // initializing storage
    bt.init(); // initializing serial bluetooth
    if(!wf.init())  // initializing wifi
    {
        wf.connect_to_nearest();
    }
    while(1){
        command_bt();
        wf.check_connection(); // Check connection to WiFi
        delay(300);
    }
}
void setup() {
    Serial.begin(115200); //Start Serial monitor
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, OUTPUT);
    test1();
    //setupCloudIoT();
    //bt.init();
    initRTC();
}
unsigned long lastMillis = 0;
String CloudData = "";
void loop() {
    //we need to place a valid CSV string in towrite string
    
    //Now towrite string contains one valid string of CSV data chunk
    
    /*mqtt->loop();
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
    }*/
    static long counter = 0;
    counter++;
    Serial.println(counter);
    Serial.println();
    delay(1000);
}
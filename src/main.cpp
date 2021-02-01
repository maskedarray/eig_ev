/*
 * Copyright 2020, Energy Informatics Group, LUMS University. All rights reserved.
 * Developed by Abdur Rahman(rao.arrn@gmail.com), Wajahat Ali(s_ali@lums.edu.pk), Farasat(razi97@hotmail.com).
 * 
 * Main file for Battery Swapping Station code
 * 
 * This file is the property of Energy Informaics Group, Lums University. Its sole private
 * ownership remains with LUMS and it should not be shared with anyone without the consent 
 * of respective authority.
 */

//TODO: handle the problem when semaphore is not available within the defined time. and also define the blocking time
//TODO: OPTIMIZATION: convert String to c string.
//TODO: optimization of data that is to be saved and sent
#define DATA_ACQUISITION_TIME 1000      //perform action every 1000ms
#define DATA_MAX_LEN 1200   //bytes

#include <Arduino.h>
#include <FreeRTOS.h>
#include <rtc.h>
#include <Storage.h>
#include "esp32-mqtt.h"

String towrite, toread;
TaskHandle_t rpcTask, storageTask, wifiTask;

void vStorage( void *pvParameters );
void vWifiTransfer( void *pvParameters );
void vRpcService( void *pvParameters );
SemaphoreHandle_t semaStorage1, seamStorage2, semaWifi1;



void setup() {
    Serial.begin(115200); //Start Serial monitor
    Serial2.begin(15200);
    Serial2.setTimeout(1000);
    pinMode(LED_BUILTIN, OUTPUT);
    setupCloudIoT();
    initRTC();
    if(storage.init_storage()){
        Serial.println("main() -> main.cpp -> storage initialization success!");
    }
    else{   //TODO: handle when storage connection fails
        while(1){
            Serial.println("main() -> main.cpp -> storage initialization failed!");
            delay(1000);
        }
    }
    semaStorage1 = xSemaphoreCreateBinary();
    seamStorage2 = xSemaphoreCreateBinary();
    semaWifi1 = xSemaphoreCreateBinary();

    
    xTaskCreatePinnedToCore(vRpcService, "RPC Handler", 10000, NULL, 3, &rpcTask, 0);
    xTaskCreatePinnedToCore(vStorage, "Storage Handler", 10000, NULL, 2, &storageTask, 1);
    xTaskCreatePinnedToCore(vWifiTransfer, "Transfer data on Wifi", 1000, NULL, 1, &wifiTask, 1);
    Serial.println("created all tasks");
}
unsigned long lastMillis = 0;
String CloudData = "";
void loop() {
    
}

void vRpcService(void *pvParameters){
    for(;;){
        if(true){
            String cmd = Serial2.readStringUntil('\n');
            int cmd_num = (10 * ((uint8_t)cmd[1] - 48)) + ((uint8_t)cmd[2] - 48);
            switch(cmd_num){
                case 10:    //check if wifi is connected
                {
                    int ret = WiFi.isConnected(); 
                    String ret_msg = "<" + String(10) + "," + String(ret) + ">";
                    Serial2.println(ret_msg);
                    break;
                }
                case 11:    //create new connection
                {
                    bool ret = false;
                    //parse string and create new connection
                    String ret_msg =  "<" + String(11) + "," + String(ret) + ">";
                    Serial2.println(ret_msg);
                    break;
                }
                case 20:    //Start uploading data to cloud
                {
                    bool ret = false;
                    //store data for sending to cloud
                    long len = cmd.length();
                    towrite = cmd.substring(4, len - 1);
                    xSemaphoreGive(semaStorage1);
                    ret = true;
                    String ret_msg = "<" + String(20) + "," + String(ret) + ">";
                    Serial2.println(ret_msg);
                    break;
                }
                case 30:    //send time from rtc to master
                {
                    String ret = unixTime();
                    String ret_msg = "<" + String(30) + "," + ret + ">";
                    Serial2.println(ret_msg);
                    break;
                }
                case 40:
                {
                    
                }
                default:
                    break;
            }
        }
        else{
            vTaskDelay(10);
        }
    }
}

void vStorage( void *pvParameters ){
    for(;;){    //infinite loop
        xSemaphoreTake(semaStorage1,portMAX_DELAY);
        xSemaphoreTake(semaWifi1,portMAX_DELAY);    //wait for wifi transfer task to finish
        {
            storage.write_data(getTime2(), towrite);
        }
        xSemaphoreGive(semaWifi1);  //resume the wifi transfer task
    }   //end for
}   //end vStorage task

void vWifiTransfer( void *pvParameters ){
    for(;;){    //infinite loop
        //check unsent data and send data over wifi
        //also take semaWifi1 when starting to send one chunk of data and give semaWifi1 when sending of one chunk of data is complete
        xSemaphoreTake(semaWifi1,portMAX_DELAY);
        if(WiFi.isConnected())
        {
            int counter = 0;
            while((storage.get_unsent_data(getTime2()) > 500) && (counter < 5)){
                mqtt->loop();
                delay(10);  // <- fixes some issues with WiFi stability
                if (!mqttClient->connected()) {
                    connect();
                }
                if (mqttClient->connected()) {
                    toread = storage.read_data();
                    if (toread != ""){
                        if(publishTelemetry(toread)){
                            storage.mark_data(getTime2());
                        }
                    }
                }
                counter++;
            }
        }
        xSemaphoreGive(semaWifi1);
        vTaskDelay(10);
    }   //end for
}   //end vWifiTransfer task
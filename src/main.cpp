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
#define COMMAND_RECEIVE_TIMEOUT 1000    //millisecond
#define DEFAULT_BSS_WIFI_SSID "h7BLH=U5f+qJCeG4"
#define DEFAULT_BSS_WIFI_PASS "&_nJ}<pj&4w@3F}F"

#include <Arduino.h>
#include <FreeRTOS.h>
#include <rtc.h>
#include <Storage.h>
#include "ESPWiFi.h"
#include "esp32-mqtt.h"

String towrite, toread;
TaskHandle_t rpcTask, storageTask, wifiTask;

void vStorage( void *pvParameters );
void vWifiTransfer( void *pvParameters );
void vRpcService( void *pvParameters );
SemaphoreHandle_t semaStorage1, semaWifi1;



void setup() {
    Serial.begin(115200); //Start Serial monitor
    Serial2.begin(115200);
    Serial2.setTimeout(COMMAND_RECEIVE_TIMEOUT);   //command receive timeout in ms
    pinMode(LED_BUILTIN, OUTPUT);                   //led for status purposes
    initRTC();
    if(storage.init_storage()){
        log_d("storage initialization success!\r\n");
    }
    else{   //TODO: handle when storage connection fails
        while(1){
            log_d("storage initialization failed!\r\n");
            delay(1000);
        }
    }
    wf.init();  
    setupCloudIoT();    //TODO: change this function and add wifi initialization
    semaStorage1 = xSemaphoreCreateBinary();
    semaWifi1 = xSemaphoreCreateBinary();
    xSemaphoreGive(semaWifi1);

    
    xTaskCreatePinnedToCore(vRpcService, "RPC Handler", 10000, NULL, 3, &rpcTask, 0);
    xTaskCreatePinnedToCore(vStorage, "Storage Handler", 10000, NULL, 2, &storageTask, 1);
    xTaskCreatePinnedToCore(vWifiTransfer, "Transfer data on Wifi", 10000, NULL, 1, &wifiTask, 1);
    Serial.println("created all tasks");
}

//loop should remain empty
void loop() {
}
String parse_by_key(String message, int key)
{
    char temp = '\0';
    int index = 0;
    int comma_count = 0;
    int counter = 0;
    String key_value = "";

    while(message[index] != '>')
    {
        if(message[index] == ',')
        {
            comma_count++;
        }
        index++;
    }
    
    Serial.printf("parse_by_key() -> cmdlib.hpp -> The counter value is %d and there are %d commas in our code. \n", index, comma_count);
    index = 0;
    if(key > comma_count)
    {
        Serial.println(F("parse_by_key() -> cmdlib.hpp -> key exceeds number of entries"));
        return "";
    }
    
    while(counter != key && counter <= comma_count && message[index] != '>')
    {
        if(message[index] == ',')
        {
            counter++;
        }
        index++;
    }

    while(message[index] != '>' && message[index] != ',')
    {
        temp = message[index];
        key_value += temp;
        index++;
    }
    Serial.printf("the value of the given key is %s \n", key_value.c_str());
    return key_value;

}
void vRpcService(void *pvParameters){
    for(;;){
        if(Serial2.available()){
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
                    String tempssid = parse_by_key(cmd,1);
                    String temppass = parse_by_key(cmd,2);
                    ret = wf.create_new_connection(tempssid.c_str(),temppass.c_str());
                    String ret_msg =  "<" + String(11) + "," + String(ret) + ">";
                    Serial2.println(ret_msg);
                    break;
                }
                case 20:    //get string and upload to cloud
                {
                    bool ret = false;
                    //store data for sending to cloud
                    long len = cmd.length();
                    towrite = cmd.substring(4, len - 2);
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
                case 40:    //get bss id from bss and send it to master
                {
                    /**
                     * take semaphore of wifi so wifi task cannot run
                     * connect to new wifi
                     * send request to server
                     * get response of server
                     * send the bss id data over serial channel to master
                     */
                    xSemaphoreTake(semaWifi1,portMAX_DELAY);
                    WiFi.begin(DEFAULT_BSS_WIFI_SSID,DEFAULT_BSS_WIFI_PASS);
                    vTaskDelay(1000);
                    if(WiFi.isConnected() == WL_CONNECTED){
                        //handle here
                    }
                    WiFi.disconnect(false,true);
                    xSemaphoreGive(semaWifi1);
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
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    for(;;){    //infinite loop
        xSemaphoreTake(semaStorage1,portMAX_DELAY);
        xSemaphoreTake(semaWifi1,portMAX_DELAY);    //wait for wifi transfer task to finish
        {
            storage.write_data(getTime2(), towrite);
        }
        xSemaphoreGive(semaWifi1);  //resume the wifi transfer task
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        Serial.printf("Stack usage of Storage Task: %d\r\n",(int)uxHighWaterMark);
    }   //end for
}   //end vStorage task

void vWifiTransfer( void *pvParameters ){
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    for(;;){    //infinite loop
        //check unsent data and send data over wifi
        //also take semaWifi1 when starting to send one chunk of data and give semaWifi1 when sending of one chunk of data is complete
        xSemaphoreTake(semaWifi1,portMAX_DELAY);
        if(wf.check_connection())
        {
            int counter = 0;
            while((storage.get_unsent_data(getTime2()) > 500) && (counter < 5)){
                mqtt->loop();
                vTaskDelay(10);  // <- fixes some issues with WiFi stability
                if (!mqttClient->connected()) {
                    mqtt->mqttConnect();
                }
                if (mqttClient->connected()) {
                    toread = storage.read_data();
                    if (toread != "" && publishTelemetry(toread)){
                        storage.mark_data(getTime2());
                    }
                }
                counter++;
            }
            xSemaphoreGive(semaWifi1);
        }
        else{
            log_d("wifi cannot connect\r\n");
            xSemaphoreGive(semaWifi1);
            vTaskDelay(10000);
        }
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        Serial.printf("Stack usage of Wifi Task: %d\r\n",(int)uxHighWaterMark);
        vTaskDelay(10);
    }   //end for
}   //end vWifiTransfer task
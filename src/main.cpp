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
//TODO: cleanup rtc code
#define DATA_ACQUISITION_TIME 1000      //perform action every 1000ms
#define DATA_MAX_LEN 1200   //bytes
#define COMMAND_RECEIVE_TIMEOUT 1000    //millisecond
#define DEFAULT_BSS_WIFI_SSID "EiG"
#define DEFAULT_BSS_WIFI_PASS "12344321"

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
    log_i("initialized wifi successfully\r\n");  
    setupCloudIoT();    //TODO: change this function and add wifi initialization
    log_i("cloud iot setup complete\r\n");
    semaStorage1 = xSemaphoreCreateBinary();
    semaWifi1 = xSemaphoreCreateBinary();
    xSemaphoreGive(semaWifi1);

    
    xTaskCreatePinnedToCore(vRpcService, "RPC Handler", 10000, NULL, 3, &rpcTask, 0);
    xTaskCreatePinnedToCore(vStorage, "Storage Handler", 10000, NULL, 2, &storageTask, 1);
    xTaskCreatePinnedToCore(vWifiTransfer, "Transfer data on Wifi", 10000, NULL, 1, &wifiTask, 1);
    pinMode(25,OUTPUT);
    digitalWrite(25,HIGH);
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
/**
 * @brief this task reads a line of command from serial bus and performs some function
 * Below is the list of available commands:
 * 
 * 1. <10>\n will return the status of wifi connection. eg: <10,0>\n if disconnected.
 * 
 * 2. <11,ssid,pass>\n will add a new wifi ssid and password to the AP list if it is available
 * at that time. It will return true if operation was successful. eg: <11,1>\n if successful.
 * 
 * 3. <20,data>\n will upload data to cloud. It will return true in any case. Note that we have to
 * follow the data generating rules(eg: length > 1000 bytes) because system is not tested otherwise
 * 
 * 4. <30>\n will return the unix time.
 * 
 * 5. <40>\n will return the bss id if the connection with server is successful. The dummy code for
 * server is available in the branch dummy-server. Also note that we have to provide a bss wifi for
 * new connection. eg: successful return will be something like <40,BSS1904>
 * 
 * WARNING: we have used \n as the string terminator. Normally, println function appends \r\n to the 
 * ending of string. So when we parse string till \n, we will get one extra character (\r) at the 
 * end of command sent or received. It may pose a problem in sending or receiving, so it is proposed
 * to use print with explicit addition \n character at the end of string to be sent.
 * @param pvParameters void
 */
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
                    log_d("message sent to master: %s\r\n",ret_msg.c_str());
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
                    log_d("message sent to master: %s\r\n",ret_msg.c_str());
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
                    log_d("message sent to master: %s\r\n",ret_msg.c_str());
                    break;
                }
                case 30:    //send time from rtc to master
                {
                    String ret = unixTime();
                    String ret_msg = "<" + String(30) + "," + ret + ">";
                    Serial2.println(ret_msg);
                    log_d("message sent to master: %s\r\n",ret_msg.c_str());
                    break;
                }
                case 40:    //get bss id from bss and send it to master
                {
                    xSemaphoreTake(semaWifi1,portMAX_DELAY);
                    WiFi.begin(DEFAULT_BSS_WIFI_SSID,DEFAULT_BSS_WIFI_PASS);
                    vTaskDelay(10000);
                    String ret = "<40,";
                    if(WiFi.isConnected() == true){
                        //handle here
                        WiFiClient client;
                        if(client.connect("192.168.43.202",80)){
                            log_d("client connected\r\n");
                            client.print("client1711\n");
                            long time_start = millis();
                            long time_stop = millis();
                            while(time_stop - time_start < 5000){
                                if(client.available()){
                                    ret += client.readStringUntil('\n');
                                    log_d("response received\r\n");
                                    break;
                                }
                                time_stop = millis();
                                vTaskDelay(10);
                            }
                            client.stop();
                            log_d("client disconnected\r\n");
                        }
                    }
                    else{
                        log_d("could not connect to bss wifi\r\n");
                    }
                    WiFi.disconnect(false,true);
                    xSemaphoreGive(semaWifi1);
                    ret += ">";
                    Serial2.println(ret);
                    log_d("message sent to master: %s\r\n",ret.c_str());
                    break;
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
        if(wf.check_connection() && (storage.get_unsent_data(getTime2()) > 500))
        {
            for(int i=0; i<5; i++){
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
            }
            xSemaphoreGive(semaWifi1);
        }
        else{
            xSemaphoreGive(semaWifi1);
            vTaskDelay(10000);
        }
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        Serial.printf("Stack usage of Wifi Task: %d\r\n",(int)uxHighWaterMark);
        vTaskDelay(10);
    }   //end for
}   //end vWifiTransfer task
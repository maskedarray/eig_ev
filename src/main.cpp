/*
 * Copyright 2020, Energy Informatics Group, LUMS University. All rights reserved.
 * Developed by Abdur Rahman(rao.arrn@gmail.com), Wajahat Ali(s_ali@lums.edu.pk), Farasat(razi97@hotmail.com).
 * 
 * Main file for EV ESP32 master code
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
//#define RESET_ESP_STORAGE   //Warning: Do not uncomment this line
#define LED_1   2
#define RTC_LED 14
#define BT_LED 27
#define STORAGE_LED 26
#define CAN_LED 25
#define WIFI_LED 33
#define FIREBASE_HOST "batteryswapstation.firebaseio.com" 
#define FIREBASE_AUTH "3v7E1QgsqLjEUx5KX1mw6kaj0ONb1IrtJ5HyNxCO" 

#include <Arduino.h>
#include <FreeRTOS.h>
#include <can.h>
#include <bluetooth.h>
#include <cmdlib-master.h>
#include <rtc.h>
#include <Storage.h>
#include "ESPWiFi.h"
#include "esp32-mqtt.h"
#include "Preferences.h"
#include "defines.h"
#include "FirebaseESP32.h"

Preferences settings__;
String towrite;
TaskHandle_t dataTask1, blTask1, blTask2, storageTask, wifiTask, ledTask;
void vAcquireData( void *pvParameters );
void vBlTransfer( void *pvParameters );
void vBlCheck( void *pvParameters );
void vStorage( void *pvParameters );
void vWifiTransfer( void * pvParameters);
void vStatusLed( void * pvParameters);
int dataflag =0;
FirebaseData firebaseData;
String EV_ID;
byte flags[16];

SemaphoreHandle_t semaAqData1, semaBlTx1, semaBlRx1, semaStorage1, semaWifi1;
void addSlotsData(String B_Slot,String B_ID,String B_U_Cycles , 
                    String B_Temp, String B_SoC, String B_SoH,String B_Vol ,String B_Curr){
    towrite += B_Slot + "," + B_ID + "," +
            B_U_Cycles + "," + B_Temp + "," + B_SoC + "," + B_SoH + "," + B_Vol + "," + B_Curr;
    return;
}
void IRAM_ATTR test(){
    dataflag++;
}

void setup() {
    // cmdinit();
    Serial.begin(115200);
    #ifdef RESET_ESP_STORAGE
        settings__.begin("ev-app", false);
        settings__.clear();
        settings__.end();
        log_i("Reset ESP Preferences!");
        delay(500);
        ESP.restart();
    #endif
    pinMode(CAN_LED,OUTPUT);
    pinMode(WIFI_LED,OUTPUT);
    pinMode(STORAGE_LED,OUTPUT);
    pinMode(RTC_LED,OUTPUT);
    pinMode(BT_LED,OUTPUT);
    digitalWrite(CAN_LED, LOW);
    digitalWrite(RTC_LED, LOW);
    digitalWrite(BT_LED, LOW);
    digitalWrite(WIFI_LED, LOW);
    digitalWrite(STORAGE_LED, LOW);
    xTaskCreatePinnedToCore(vStatusLed, "Status LED", 1000, NULL, 1, &ledTask, 1);
    
    if(can.init_can()){
        digitalWrite(CAN_LED, HIGH);
        flags[can_f] = 1;
    }
    else{   //sound alarm and do nothing!
        while(1){
            flags[can_f] = 0;
            delay(100);
        }
    }
    if(initRTC()){
        digitalWrite(RTC_LED, HIGH);
        flags[rtc_f] = 1;
        _set_esp_time();
    }
    else{
        flags[rtc_f] = 0;               //the system time would be 000 from start. The data would be ? I guess 1/1/2000, or maybe 1970..
    }
    
    log_i("ESP system time: %s", esp_sys_time.getDateTime().c_str());
    if(storage.init_storage()){
        log_d("storage initialization success!");
        flags[sd_f] = 1;
        digitalWrite(STORAGE_LED, HIGH);
    }
    else{
        flags[sd_f] = 0;
        log_d("storage initialization failed!");
    }
    wf.init();
    log_i("initialized wifi successfully");  
    {
        settings__.begin("ev-app", false);
        bt.bluetooth_name = settings__.getString("bt-name", "");
        bt.bluetooth_password = settings__.getString("bt-pass","");
        EV_ID = bt.bluetooth_name;
        device_id = new char[30];
        bt.bluetooth_name.toCharArray(device_id, strlen(bt.bluetooth_name.c_str()) + 1);
        if(bt.bluetooth_name != "" && bt.bluetooth_password != ""){                             //settings saved previously
            settings__.end();
            log_i("Settings found!\r\nbluetooth name: %s\r\n bluetooth password: %s", bt.bluetooth_name.c_str(), bt.bluetooth_password.c_str());
        }
        else{                                       //saved settings not found
            log_i("Settings not found! Loading from Firebase");
            wf.check_connection();
            while(!WiFi.isConnected()){
                log_e("No wifi available, waiting for connection");
                delay(1000);
            }
            Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
            Firebase.reconnectWiFi(true);
            Firebase.setReadTimeout(firebaseData, 1000 * 60);
            Firebase.setwriteSizeLimit(firebaseData, "small");
            String mac = WiFi.macAddress();
            String tmpdevid, tmpbtname, tmpbtpass;
            if(Firebase.getString(firebaseData,"/" + mac + "/bluetoothName", tmpbtname))
                settings__.putString("bt-name", tmpbtname); 
            if(Firebase.getString(firebaseData, "/" + mac + "/bluetoothPass", tmpbtpass))
                settings__.putString("bt-pass", tmpbtpass);
            log_i("got data from firebase\r\nbluetooth name: %s\r\n bluetooth password: %s",tmpbtname.c_str(), tmpbtpass.c_str());
            settings__.end();
            log_i("restarting");
            ESP.restart();
        }
    }
    bt.init();
    delay(5000);    //wait for wifi to initialize
    setupCloudIoT();    //TODO: change this function and add wifi initialization
    log_i("cloud iot setup complete");

    
    //set_system_time();      //timeout for response has been set to 20000 so slave initializes successfully 
    attachInterrupt(0, test, FALLING);

    semaAqData1 = xSemaphoreCreateBinary();
    semaBlTx1 = xSemaphoreCreateBinary();
    semaBlRx1 = xSemaphoreCreateBinary();
    semaStorage1 = xSemaphoreCreateBinary();
    semaWifi1 = xSemaphoreCreateBinary();

    xSemaphoreGive(semaAqData1);
    xSemaphoreGive(semaBlRx1);
    xSemaphoreGive(semaWifi1);
    
    
    xTaskCreatePinnedToCore(vAcquireData, "Data Acquisition", 5000, NULL, 3, &dataTask1, 1);
    xTaskCreatePinnedToCore(vStorage, "Storage Handler", 7000, NULL, 2, &storageTask, 1);
    xTaskCreatePinnedToCore(vBlCheck, "Bluetooth Commands", 5000, NULL, 2, &blTask1, 0);
    xTaskCreatePinnedToCore(vBlTransfer, "Bluetooth Transfer", 5000, NULL, 3, &blTask2, 0);
    xTaskCreatePinnedToCore(vWifiTransfer, "Transfer data on Wifi", 10000, NULL, 1, &wifiTask, 0);
    
    log_i("created all tasks");
}

void loop() {
    
}

/**
 * @brief This function acquires data from CAN bus. This is a timed function and 
 * is hard synced by the semaAqData1 semaphore. This means that this function will
 * not run again until the the previous acquired data has been sent over bluetooth.
 * This semaphore also handles the resource sharing (sharing the global string towrite)
 * problem with the vBlTransfer task.
 * 
 * @param pvParameters void
 */
void vAcquireData( void *pvParameters ){
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;){    //infinite loop
        xSemaphoreTake(semaAqData1, portMAX_DELAY); //semaphore to check if sending of data over bluetooth and storage has returned
        {
            //Dummy acquisition of data
            float randvoltage = 11 + (random(0,2000)/1000.0);
            towrite = "";                               //empty the string
            towrite += String(getTime()) + ",";           //time
            towrite += String(EV_ID) + ",";      //vehicle id
            towrite += String("5.019") + ",";           //MCU voltage
            towrite += String("0.234") + ",";           //MCU CURRENT
            towrite += String("3000") + ",";            //vehicle rpm
            towrite += String("34.36") + ",";           //MCU Temperature
            //          S1_B_Slot, S1_B_ID, S1_B_U_Cylcles, S1_B_Temp, S1_B_SoC, S1_B_SoH, S1_B_Vol, S1_B_Curr,
            if(dataflag == 0){
                log_i("currently sending data %d",dataflag);
                addSlotsData("01", "batt1", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("02", "BATT3", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("03", "BATT5", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("04", "BATT7", "30", "80", "50", "22", String(randvoltage), "26.561");//towrite += ",";
            }
            else if (dataflag == 1){
                log_i("currently sending data %d",dataflag);
                addSlotsData("0", "0", "0", "0", "0", "0", "0", "0");towrite += ",";
                addSlotsData("0", "0", "0", "0", "0", "0", "0", "0");towrite += ",";
                addSlotsData("0", "0", "0", "0", "0", "0", "0", "0");towrite += ",";
                addSlotsData("0", "0", "0", "0", "0", "0", "0", "0");
            }
            else if (dataflag ==2){
                log_i("currently sending data %d",dataflag);
                addSlotsData("01", "BATT2", "BSS22", "30", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("02", "BATT4", "BSS22", "30", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("03", "BATT6", "BSS22", "30", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("04", "BATT8", "BSS22", "30", "50", "22", String(randvoltage), "26.561");//towrite += ",";
            }
            //Now towrite string contains one valid string of CSV data chunk
        }
        xSemaphoreGive(semaAqData1); 
        xSemaphoreGive(semaBlTx1);      //signal to call bluetooth transfer function once
        xSemaphoreGive(semaStorage1);
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        log_v("Stack usage of acquiredata Task: %d",(int)uxHighWaterMark);
        vTaskDelayUntil(&xLastWakeTime, 5*DATA_ACQUISITION_TIME);    //defines the data acquisition rate
    }   //end for
}   //end vAcquireData

/**
 * @brief This function sends the data acquired over CAN to the bluetooth. 
 * The execution of this task is controlled by the semaphore semaBlTx1. This 
 * semaphore is given by the acquire data task. Since the semaphore is given 
 * on every chunk of data acquired so, the execution frequency of this task is
 * same as that of acquire data i.e. 30 seconds
 * 
 * @param pvParameters void
 */
void vBlTransfer( void *pvParameters ){ //synced by the acquire data function
    for(;;){    //infinite loop
        xSemaphoreTake(semaBlTx1, portMAX_DELAY);   //for task sync with acquire data
        xSemaphoreTake(semaAqData1, portMAX_DELAY); //for copying towrite string
        String towrite_cpy;
        towrite_cpy = towrite;
        xSemaphoreGive(semaAqData1);
        xSemaphoreTake(semaBlRx1, portMAX_DELAY);
        log_d("sending data over bluetooth ");
        bt.send(towrite_cpy);
        xSemaphoreGive(semaBlRx1);
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        log_v("Stack usage of bltransfer Task: %d", (int)uxHighWaterMark);
    }   //end for
}   //end vBlTransfer task

/**
 * @brief This function checks and executes any available command sent 
 * by the mobile phone.
 * 
 * @param pvParameters void
 */
void vBlCheck( void *pvParameters ){
    TickType_t xLastWakeTime_2 = xTaskGetTickCount();
    for(;;){
        xSemaphoreTake(semaWifi1, portMAX_DELAY);
        xSemaphoreTake(semaBlRx1, portMAX_DELAY);
        {
            command_bt();
        }
        xSemaphoreGive(semaBlRx1);
        xSemaphoreGive(semaWifi1);
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        log_v("Stack usage of blcheck Task: %d",(int)uxHighWaterMark);
        vTaskDelayUntil(&xLastWakeTime_2, 0.5*DATA_ACQUISITION_TIME);
    }
} // end vBlCheck

void vStorage( void *pvParameters ){
    for(;;){    //infinite loop
        xSemaphoreTake(semaStorage1,portMAX_DELAY); //for syncing task with acquire
        xSemaphoreTake(semaAqData1, portMAX_DELAY); // make copy of data and stop data acquisition
        String towrite_cpy;
        towrite_cpy = towrite;
        xSemaphoreGive(semaAqData1);
        xSemaphoreTake(semaWifi1,portMAX_DELAY);    //wait for wifi transfer task to finish
        {
            storage.write_data(getTime2(), towrite_cpy);
        }
        xSemaphoreGive(semaWifi1);  //resume the wifi transfer task
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        log_v("Stack usage of Storage Task: %d",(int)uxHighWaterMark);
    }   //end for
}   //end vStorage task

void vWifiTransfer( void *pvParameters ){
    for(;;){    //infinite loop
        //check unsent data and send data over wifi
        //also take semaWifi1 when starting to send one chunk of data and give semaWifi1 when sending of one chunk of data is complete
        xSemaphoreTake(semaWifi1,portMAX_DELAY);
        log_v("entering wifi task ");
        if(wf.check_connection() && (storage.get_unsent_data(getTime2()) > 500))
        {
            for(int i=0; i<5; i++){
                mqtt->loop();
                vTaskDelay(10);  // <- fixes some issues with WiFi stability
                if (!mqttClient->connected()) {
                    mqtt->mqttConnect();
                }
                if (mqttClient->connected()) {
                    String toread; 
                    toread = storage.read_data();
                    // toread = "dummy string";
                    if (toread != "" && publishTelemetry(toread)){
                        log_d("sent data to cloud ");
                        storage.mark_data(getTime2());
                    }
                }
            }
            xSemaphoreGive(semaWifi1);
            vTaskDelay(1000);
        }
        else{
            log_d("Wifi disconnected or no data to be sent! ");
            xSemaphoreGive(semaWifi1);
            vTaskDelay(10000);
        }
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        log_v("Stack usage of Wifi Task: %d",(int)uxHighWaterMark);
        vTaskDelay(10);
    }   //end for
}   //end vWifiTransfer task

void vStatusLed( void * pvParameters){
    for(;;){    //infinite loop
        if(bt.isConnected){
            digitalWrite(BT_LED,HIGH);
        }  
        else{
            digitalWrite(BT_LED,LOW);
        }
        if(WiFi.isConnected()){
            digitalWrite(WIFI_LED, HIGH);
        }
        else{
            digitalWrite(WIFI_LED,LOW);
        }
        vTaskDelay(50);
    }
}
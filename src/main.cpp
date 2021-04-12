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
#define LED_1   2
#define EV_ID "LGA8977"
#define DEFAULT_BSS_WIFI_SSID "EiG"
#define DEFAULT_BSS_WIFI_PASS "12344321"

#include <Arduino.h>
#include <FreeRTOS.h>
#include <can.h>
#include <bluetooth.h>
#include <cmdlib-master.h>
#include <rtc.h>
#include <Storage.h>
#include "ESPWiFi.h"
#include "esp32-mqtt.h"
// #include <sys/time.h>

String towrite;
TaskHandle_t dataTask1, blTask1, blTask2, storageTask, wifiTask;
void vAcquireData( void *pvParameters );
void vBlTransfer( void *pvParameters );
void vBlCheck( void *pvParameters );
void vStorage( void *pvParameters );
void vWifiTransfer( void * pvParameters);
int flag =0;


SemaphoreHandle_t semaAqData1, semaBlTx1, semaBlRx1, semaStorage1, semaWifi1;
void addSlotsData(String B_Slot,String B_ID,String B_Auth, String B_Age,String B_Type ,String B_M_Cycles ,String B_U_Cycles , 
                    String B_Temp, String B_SoC, String B_SoH, String B_RoC,String B_Vol ,String B_Curr){
    towrite += B_Slot + "," + B_ID + "," + B_Auth + "," + B_Age + "," + B_Type + "," + B_M_Cycles + "," + 
            B_U_Cycles + "," + B_Temp + "," + B_SoC + "," + B_SoH + "," + B_RoC + "," + B_Vol + "," + B_Curr;
    return;
}
void IRAM_ATTR test(){
    flag++;
}

void setup() {
    // cmdinit();
    Serial.begin(115200);
    bt.init();
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

    delay(3000);
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
    
    xTaskCreatePinnedToCore(vAcquireData, "Data Acquisition", 10000, NULL, 3, &dataTask1, 1);
    xTaskCreatePinnedToCore(vStorage, "Storage Handler", 10000, NULL, 2, &storageTask, 1);
    xTaskCreatePinnedToCore(vBlCheck, "Bluetooth Commands", 10000, NULL, 2, &blTask1, 0);
    xTaskCreatePinnedToCore(vBlTransfer, "Bluetooth Transfer", 10000, NULL, 3, &blTask2, 0);
    xTaskCreatePinnedToCore(vWifiTransfer, "Transfer data on Wifi", 10000, NULL, 1, &wifiTask, 0);
    log_i("created all tasks\r\n");
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
            towrite += String("18:50") + ",";           //time
            towrite += String(EV_ID) + ",";      //vehicle id
            towrite += String("3000") + ",";            //vehicle rpm
            towrite += String("5.019") + ",";           //MCU voltage
            towrite += String("0.234") + ",";           //MCU CURRENT
            towrite += String("34.36") + ",";           //MCU Temperature
            //          B_Slot, B_ID, B_Auth,  B_Age, B_Type , B_M_Cycles, B_U_Cycles , B_Temp, B_SoC, B_SoH, B_RoD, B_Vol , B_Curr
            if(flag == 0){
                log_i("currently sending data %d\r\n",flag);
                addSlotsData("01", "batt1", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("02", "BATT3", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("03", "BATT5", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("04", "BATT7", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "26.561");//towrite += ",";
            }
            else if (flag == 1){
                log_i("currently sending data %d\r\n",flag);
                addSlotsData("01", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0");towrite += ",";
                addSlotsData("02", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0");towrite += ",";
                addSlotsData("03", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0");towrite += ",";
                addSlotsData("04", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0");
            }
            else if (flag ==2){
                log_i("currently sending data %d\r\n",flag);
                addSlotsData("01", "BATT2", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("02", "BATT4", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("03", "BATT6", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "20.561");towrite += ",";
                addSlotsData("04", "BATT8", "BSS22", "22", "2211", "500", "200", "30", "80", "50", "22", String(randvoltage), "26.561");//towrite += ",";
            }
            //Now towrite string contains one valid string of CSV data chunk
            //Serial.println(getTime());
        }
        xSemaphoreGive(semaAqData1); 
        xSemaphoreGive(semaBlTx1);      //signal to call bluetooth transfer function once
        xSemaphoreGive(semaStorage1);
        vTaskDelayUntil(&xLastWakeTime, DATA_ACQUISITION_TIME);    //defines the data acquisition rate
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
        xSemaphoreTake(semaBlTx1, portMAX_DELAY);
        xSemaphoreTake(semaAqData1, portMAX_DELAY);
        String towrite_cpy;
        towrite_cpy = towrite;
        xSemaphoreGive(semaAqData1);
        xSemaphoreTake(semaBlRx1, portMAX_DELAY);
        {
            bt.send(towrite_cpy);
            // log_d("sending data: %s\n\r", towrite_cpy.c_str());
        }
        xSemaphoreGive(semaBlRx1);
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
    int counter = 0;
    for(;;){
        xSemaphoreTake(semaWifi1, portMAX_DELAY);
        xSemaphoreTake(semaBlRx1, portMAX_DELAY);
        {
            //while(Serial2.read() >=0){} //flushing any data available in serial RX buffer
            command_bt();
            // log_d("commands executed");
            // send data to slave for storage and uploading to cloud
            /**
             * NOTE: Slave logic needs to be put here
             */

             /*if(counter > 600){          //counter for 60 seconds since task execution period is 100ms
                log_i("sending periodic blutooth data to slave\r\n");


                //cmdsend("<20");
                //xSemaphoreTake(semaAqData1, portMAX_DELAY);
                //cmdsend(towrite);
                //xSemaphoreGive(semaAqData1);
                //cmdsend(">\r\n");
                counter = 0;
            }
            else{
                counter++;
            }*/
        }
        xSemaphoreGive(semaBlRx1);
        xSemaphoreGive(semaWifi1);
        vTaskDelayUntil(&xLastWakeTime_2, 0.1*DATA_ACQUISITION_TIME);
    }
} // end vBlCheck

void vStorage( void *pvParameters ){
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    for(;;){    //infinite loop
        xSemaphoreTake(semaAqData1, portMAX_DELAY); // make copy of data and stop data acquisition
        String towrite_cpy;
        towrite_cpy = towrite;
        xSemaphoreGive(semaAqData1);
        xSemaphoreTake(semaStorage1,portMAX_DELAY);
        xSemaphoreTake(semaWifi1,portMAX_DELAY);    //wait for wifi transfer task to finish
        {
            storage.write_data(getTime2(), towrite_cpy);
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
        if(wf.check_connection() /*&& (storage.get_unsent_data(getTime2()) > 500)*/)
        {
            for(int i=0; i<5; i++){
                mqtt->loop();
                vTaskDelay(10);  // <- fixes some issues with WiFi stability
                if (!mqttClient->connected()) {
                    mqtt->mqttConnect();
                }
                if (mqttClient->connected()) {
                    String toread; 
                    // toread = storage.read_data();
                    toread = "dummy string";
                    if (toread != "" && publishTelemetry(toread)){
                        log_d("vWifiTransfer -> main.cpp -> sending data to cloud");
                        //storage.mark_data(getTime2());
                    }
                }
            }
            xSemaphoreGive(semaWifi1);
            vTaskDelay(1000);
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


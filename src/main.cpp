/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include <Arduino.h>
#include <esp32-mqtt.h>
#include <Storage.h>
#include <rtc.h>
#include <can.h>
#include <bluetooth.h>
#include <FreeRTOS.h>
TaskHandle_t Task1;
TaskHandle_t Task2;
void Task1code( void * pvParameters );
void Task2code( void * pvParameters );
// LED pins
const int led1 = 2;
const int led2 = 4;

void setup() {
  Serial.begin(115200); //Start Serial monitor
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, OUTPUT);
    //setupCloudIoT();
    //bt.init();
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
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    digitalWrite(led1, HIGH);
    delay(1000);
    digitalWrite(led1, LOW);
    delay(1000);
  } 
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
      Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
    digitalWrite(led2, HIGH);
    delay(700);
    digitalWrite(led2, LOW);
    delay(700);
  }
}

void loop() {
  
}
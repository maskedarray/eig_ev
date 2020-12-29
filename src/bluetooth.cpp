#include <BluetoothSerial.h>
#include <bluetooth.h>

BluetoothSerial esp_bt;

bool ESP_BT::init_bt(){
    esp_bt.begin(BLUETOOTH_NAME); //Name of your Bluetooth Signal
    Serial.println(F("init_bt() -> bluetooth.cpp -> Bluetooth Device is Ready to Pair"));
    return true;
}

ESP_BT bt;
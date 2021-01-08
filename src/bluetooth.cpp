#include <BluetoothSerial.h>
#include <bluetooth.h>

BluetoothSerial esp_bt;
char BT_incoming[32]; // array of characyters, to read incoming data

//TODO: return statements are dummy. handle the scenario when connection fails 
//or sending of data fails

/**
 * This function initializes the bluetooth. The name of bluetooth is defined in
 * macro BLUETOOTH_NAME present in header file
 * 
 * @return true if bluetooth initialization is successful. false if some error occurs
 * 
 */
bool ESP_BT::init(){
    esp_bt.begin(BLUETOOTH_NAME); //Name of your Bluetooth Signal
    Serial.println(F("init_bt() -> bluetooth.cpp -> Bluetooth Device is Ready to Pair"));
    return true;
}

/**
 * Sends string of CSV data on bluetooth
 * 
 * @param[in] tosend is the string of data to send on bluetooth
 * @return true if data is sent. false if no data is sent.
 */
bool ESP_BT::send(String tosend){
    long len = tosend.length();
    esp_bt.print("%S%");    //start byte
    esp_bt.print(String(len));
    esp_bt.print(",");
    esp_bt.print(tosend);   //data, first parameter is length of data starting from next value (after comma)
    esp_bt.print("%S%");    //end byte
    esp_bt.println();       //end byte
    
    if (esp_bt.available()){ //Check if we receive anything from Bluetooth
        for(int i=0; i<4; i++){ // reading four bytes only, need to send data to esp in a standardized form (delimiters)
            BT_incoming[i] = esp_bt.read();
        }
        Serial.print("Received:"); Serial.println(BT_incoming);
    }
}

ESP_BT bt;
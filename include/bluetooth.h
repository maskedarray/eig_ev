#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include <Arduino.h>
#include <BluetoothSerial.h>

#define BLUETOOTH_NAME "EiG BT"

/**
 * This library is made to optimize the functionality of the ESP32 bluetooth
 * functions. It allows for the reading and parsing of serial data for the
 * specific purpose of collecting new access points, which will be subsequently
 * stored for late use.
 */

class ESP_BT {
private:
    bool got_credentials;

public:
    BluetoothSerial SerialBT;
    bool init();
    bool send(String tosend);
    String bt_read();
    void wifi_parse(String text, String &Username, String &Password);
    void EV_ID_parse(String text, String &unique_identifier);
    void display(String ID, String Username, String Password);
    String check_bluetooth();
};

extern ESP_BT bt;
#endif
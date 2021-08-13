#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include <NimBLEDevice.h>
#include <cmdlib-master.h>

/**
 * @brief This library is made to optimize the functionality of the ESP32 bluetooth
 * functions. It allows for the reading and parsing of serial data for the
 * specific purpose of collecting new access points, which will be subsequently
 * stored for late use.
 */

class ESP_BT {
private:
    String bt_read();

public:
    //BluetoothSerial SerialBT;
    String bluetooth_name;
    String bluetooth_password;
    String bt_msg;
    bool init();
    bool send(String tosend);
    bool send_notification(String tosend);
    void display(String ID, String Username, String Password);
    bool check_bluetooth();
    bool isConnected;
    bool commandInQueue;
    bool commandComplete;
};

extern ESP_BT bt;
#endif
#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include <Arduino.h>
// #include <BluetoothSerial.h>


/**
 * This library is made to optimize the functionality of the ESP32 bluetooth
 * functions. It allows for the reading and parsing of serial data for the
 * specific purpose of collecting new access points, which will be subsequently
 * stored for late use.
 */

class ESP_BT {
private:
    String bt_read();
   

public:
<<<<<<< HEAD
=======
    //BluetoothSerial SerialBT;
    String bluetooth_name;
    String bluetooth_password;
>>>>>>> initsetting
    bool init();
    bool send(String tosend);
    String check_bluetooth();
    bool isConnected;
};

extern ESP_BT bt;
#endif
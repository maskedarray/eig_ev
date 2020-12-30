#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include <Arduino.h>

#define BLUETOOTH_NAME "EiG BT"

class ESP_BT {
private:

public:
    bool init();
    bool send(String tosend);
};

extern ESP_BT bt;
#endif
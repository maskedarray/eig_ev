#ifndef __CAN_H__
#define __CAN_H__
#include <Arduino.h>

#define MCP2515_CSPIN 32
class EVCan {
private:

public:
    uint16_t id;
    float soc;
    float hi_temp;
    float lo_temp;
    float voltage;
    float current;
    bool init_can();
    void send_msg(uint16_t id, float soc, float hi_temp, float lo_temp, float voltage, float current);
    bool receive_msg(void);
};

extern EVCan can;

#endif
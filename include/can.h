#ifndef __CAN_H__
#define __CAN_H__
#include <Arduino.h>

#define MCP2515_CSPIN 32
#define INIT_REQ_TIMEOUT 1000   //milliseconds

struct EvData{
    int mcu_rpm;
    int mcu_voltage;
    int mcu_cur;
    int mcu_temp;
};
struct BmsData{
    int temp;
    int soc;
    int soh;
    int ucycles;
    float vol;
    float cur;
    byte id[21];
};
class EVCan {
private:
    void mcu_message(byte data[8]);
    void ucycle_message(uint16_t id, byte data[8]);
    void cvts_message(uint16_t id,byte data[8]);
    void soh_message(uint16_t id,byte data[8]);
    void bid_message(uint16_t id, byte data[8]);
public:
    EvData evdata;
    BmsData bmsdata[16];
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
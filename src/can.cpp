#include <can.h>
#include <mcp_can.h>

MCP_CAN mcp_can(MCP2515_CSPIN);

/**
 * Initializes CAN module. The speed of CAN and other parameters are decided by 
 * the standard kept by Koreans. The intialization is tried 5 times. 
 * 
 * @return 1 if initialization is successful and 0 if it fails.
 */

bool EVCan::init_can(){ 

    for(int i = 0; i < 5; i++){
        if(CAN_OK != mcp_can.begin(CAN_500KBPS))
            log_e("init_can() -> can.cpp -> CAN bus init failed!");
        else{
            log_d("init_can() -> can.cpp -> CAN bus initialized!");
            return true;
        }
        delay(500);
    }
    return false;
}

/**
 * Sends message on CAN bus
 * 
 * It is only for emulation of CAN protocol developed by the Koreans. It is 
 * only used in hardware for emulation. This function sends data on CAN bus according to 
 * standard defined by Koreans.
 * 
 * @param[in] id: 11 bit ID in hex
 * @param[in] soc: state of charge. ranges from 0-100. resolution 1.
 * @param[in] hi_temp: BMS high temperature. ranges from -40-120. resolution 1.
 * @param[in] lo_temp: BMS low temperature. ranges from -40-120. resolution 1.
 * @param[in] voltage: BMS voltage ranges from 0.0 - 500.0. resolution 0.1.
 * @param[in] current: BMS current. ranges from -350.0 - 350.0. resolution 0.1. 
 */

void EVCan::send_msg(uint16_t id, float soc, float hi_temp, float lo_temp, float voltage, float current){
    switch (id)
    {
    case 0x610:
        byte data[8];
        data[0] = 7;
        data[1] = (byte)((int)soc);
        data[2] = (byte)((int)(hi_temp + 40));
        data[3] = (byte)((int)(lo_temp + 40));
        data[4] = (byte)((int)(voltage * 10));
        data[5] = (byte)(((int)(voltage * 10)) >> 8);
        data[6] = (byte)((int)((current + 1000) * 10));
        data[7] = (byte)(((int)((current + 1000) * 10)) >> 8);
        mcp_can.sendMsgBuf(id, 0, 8, data);
        log_d("send_msg() -> can.cpp -> Data sent!");
        break;
    
    default:
        log_e("send_msg() -> can.cpp -> Invalid ID!");
        break;
    }
}

/**
 * Reads data from CAN bus
 * 
 * First, it checks if data is available to be read. If there is data available then
 * it reads data according to prestored ids as provided by Koreans. It does not return 
 * the data rather it updates the variables in the class object itself.
 * 
 * @return 1 if data is read according to valid id or 0 if no data is found or id is invalid.
 */

bool EVCan::receive_msg(void){
    bool read_success = false;
    byte data[8];
    if(CAN_MSGAVAIL == mcp_can.checkReceive()){
        log_d("receive_msg() -> can.cpp -> Data found on CAN bus. Reading..");
        read_success = true;
        unsigned char len = 0;
        mcp_can.readMsgBuf(&len, data);
        this->id = mcp_can.getCanId();
        log_d("receive_msg() -> can.cpp -> ID: ");
        Serial.println(this->id,HEX);
        switch (this->id)
        {
            case 0x190:
                mcu_message(data);
            case 
            default:
                Serial.println(F("receive_msg() -> can.cpp -> Invalid ID!"));
                read_success = false;
                break;
        }
    }
    return read_success;
}

void EVCan::mcu_message(byte data[8]){
    switch(data[0]){
        case 0x80:
            this->evdata.mcu_rpm = (int)((data[3] << 8) | data[2]);
            break;
        case 0x08:
            this->evdata.mcu_voltage = (int)((data[5] << 8) | data[4]);
            this->evdata.mcu_cur = (int)((data[7] << 8) | data[6]);
            break;
        case 0x40:
            this->evdata.mcu_temp = (int)((data[5] << 8) | data[4]) * 0.1;
            break;
    }
}

void EVCan::bms_message(byte data[8]){

}
/**
 * Only single module of CAN will be attached to the system. So only one
 * object is created.
 */
EVCan can;
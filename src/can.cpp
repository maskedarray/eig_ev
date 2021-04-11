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
        if (this->id == 0x190){
            mcu_message(data);
        }   else if (this->id >= 0x6C0 && this->id <= 0x6CF){   //accumulated utilized cycles

        }   else if (this->id >= 0x610 && this->id <= 0x61F){   //current, voltage, temp, soc

        }   else if (this->id >= 0x620 && this->id <= 0x62F){   //soh

        }   else{
            Serial.println(F("receive_msg() -> can.cpp -> Invalid ID!"));
            read_success = false;
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

void EVCan::ucycle_message(uint16_t id, byte data[8]){
    switch(id){
        case 0x6C0:
            bmsdata[0].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C1:
            bmsdata[1].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C2:
            bmsdata[2].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C3:
            bmsdata[3].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C4:
            bmsdata[4].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C5:
            bmsdata[5].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C6:
            bmsdata[6].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C7:
            bmsdata[7].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C8:
            bmsdata[8].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6C9:
            bmsdata[9].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6CA:
            bmsdata[10].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6CB:
            bmsdata[11].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6CC:
            bmsdata[12].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6CD:
            bmsdata[13].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6CE:
            bmsdata[14].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        case 0x6CF:
            bmsdata[15].ucycles = (int)((data[1] << 8) | data[0]);
            break;
        default:
            break;
    }
}
void EVCan::cvts_message(uint16_t id, byte data[8]){
    switch(id){
        case 0x610:
            bmsdata[0].soc = (int)data[1];
            bmsdata[0].temp = (int)(data[2] - 40);
            bmsdata[0].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[0].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x611:
            bmsdata[1].soc = (int)data[1];
            bmsdata[1].temp = (int)(data[2] - 40);
            bmsdata[1].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[1].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x612:
            bmsdata[2].soc = (int)data[1];
            bmsdata[2].temp = (int)(data[2] - 40);
            bmsdata[2].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[2].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x613:
            bmsdata[3].soc = (int)data[1];
            bmsdata[3].temp = (int)(data[2] - 40);
            bmsdata[3].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[3].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x614:
            bmsdata[4].soc = (int)data[1];
            bmsdata[4].temp = (int)(data[2] - 40);
            bmsdata[4].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[4].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x615:
            bmsdata[5].soc = (int)data[1];
            bmsdata[5].temp = (int)(data[2] - 40);
            bmsdata[5].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[5].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x616:
            bmsdata[6].soc = (int)data[1];
            bmsdata[6].temp = (int)(data[2] - 40);
            bmsdata[6].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[6].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x617:
            bmsdata[7].soc = (int)data[1];
            bmsdata[7].temp = (int)(data[2] - 40);
            bmsdata[7].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[7].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x618:
            bmsdata[8].soc = (int)data[1];
            bmsdata[8].temp = (int)(data[2] - 40);
            bmsdata[8].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[8].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x619:
            bmsdata[9].soc = (int)data[1];
            bmsdata[9].temp = (int)(data[2] - 40);
            bmsdata[9].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[9].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x61A:
            bmsdata[10].soc = (int)data[1];
            bmsdata[10].temp = (int)(data[2] - 40);
            bmsdata[10].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[10].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x61B:
            bmsdata[11].soc = (int)data[1];
            bmsdata[11].temp = (int)(data[2] - 40);
            bmsdata[11].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[11].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x61C:
            bmsdata[12].soc = (int)data[1];
            bmsdata[12].temp = (int)(data[2] - 40);
            bmsdata[12].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[12].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x61D:
            bmsdata[13].soc = (int)data[1];
            bmsdata[13].temp = (int)(data[2] - 40);
            bmsdata[13].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[13].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x61E:
            bmsdata[14].soc = (int)data[1];
            bmsdata[14].temp = (int)(data[2] - 40);
            bmsdata[14].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[14].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        case 0x61F:
            bmsdata[15].soc = (int)data[1];
            bmsdata[15].temp = (int)(data[2] - 40);
            bmsdata[15].vol = (float)((data[5] << 8) | data[4]) * 0.1;
            bmsdata[15].cur = ((float)((data[7] << 8) | data[6]) * 0.1) - 1000;
            break;
        default:
            break;
    }
}
void EVCan::soh_message(uint16_t id, byte data[8]){
    switch(id){
        case 0x620:
            bmsdata[0].soh = (int)data[5];
            break;
        case 0x621:
            bmsdata[1].soh = (int)data[5];
            break;
        case 0x622:
            bmsdata[2].soh = (int)data[5];
            break;
        case 0x623:
            bmsdata[3].soh = (int)data[5];
            break;
        case 0x624:
            bmsdata[4].soh = (int)data[5];
            break;
        case 0x625:
            bmsdata[5].soh = (int)data[5];
            break;
        case 0x626:
            bmsdata[6].soh = (int)data[5];
            break;
        case 0x627:
            bmsdata[7].soh = (int)data[5];
            break;
        case 0x628:
            bmsdata[8].soh = (int)data[5];
            break;
        case 0x629:
            bmsdata[9].soh = (int)data[5];
            break;
        case 0x62A:
            bmsdata[10].soh = (int)data[5];
            break;
        case 0x62B:
            bmsdata[11].soh = (int)data[5];
            break;
        case 0x62C:
            bmsdata[12].soh = (int)data[5];
            break;
        case 0x62D:
            bmsdata[13].soh = (int)data[5];
            break;
        case 0x62E:
            bmsdata[14].soh = (int)data[5];
            break;
        case 0x62F:
            bmsdata[15].soh = (int)data[5];
            break;
        default:
            break;
    }
}
/**
 * Only single module of CAN will be attached to the system. So only one
 * object is created.
 */
EVCan can;
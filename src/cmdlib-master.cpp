#include <cmdlib-master.h>


#define AUTH_CODE "123456"
#define ESP_ID "012501202101"
#define INIT_CYCLES 4
#define FIN_CYCLES 9

// #define SEND_ACK_PIN 33
// #define ACK_WAIT 50         //equal to 1 second

bool auth_flag = false; // for authorization
int initial_cycles = 0;
int final_cycles = 0;



void cmdinit(){
    Serial2.begin(115200);
    Serial2.setTimeout(200);
}

// /**
//  * @brief sends the command over serial2 and waits 1 second for acknowledgement
//  * through the digital read of pin
//  * 
//  * @param tosend the string to send
//  * @return true when the acknowledgement is received after string is sent
//  * @return false when acknowledgement is not recieved after string is sent
//  */
// bool cmdsend_ack(String tosend){
//     Serial2.println(tosend);
//     bool ack = false;
//     int counter = 0;
//     while(!ack && counter < ACK_WAIT){
//         ack = (bool)digitalRead(SEND_ACK_PIN);
//         counter++;
//         vTaskDelay(20);
//     }
//     return ack;
// }

bool cmdsend(String tosend){
    Serial2.println(tosend);
    return true;
}

int timedReadCustom(unsigned long _timeout)
{
    unsigned long _startMillis;  // used for timeout measurement
    int c;
    _startMillis = millis();
    do {
        c = Serial2.read();
        if(c >= 0) {
            return c;
        }

    } while(millis() - _startMillis < _timeout);
    return -1;     // -1 indicates timeout
}

String readStringUntilCustom(char terminator)
{
    int _timeout = 10000;
    String ret;
    int c = timedReadCustom(_timeout);
    while(c >= 0 && c != terminator) {
        ret += (char) c;
        c = timedReadCustom(_timeout);
    }
    if(c < 0)
    {
        log_e("readStringUntilCustom() -> cmdlib-master.cpp -> timout occurred \r\n");
        return "";
    }
    return ret;
}

// /**
//  * @brief this functions checks serial port for availability of command data
//  * If there is data available it parses and returns a string
//  * 
//  * @return String is the response to be passed to the application via bluetooth
//  */
// String cmdreceive(){
//     if(Serial2.available()){
//         String received = Serial2.readStringUntil('\n');
//         int cmdnum = (10 * ((uint8_t)received[0] - 48)) + ((uint8_t)received[1] - 48);
//         switch(cmdnum){
//             case 1:
//                 break;
//             case 2:
//                 break;
//             default:
//                 break;
//         }
//     }
//     else{
//         return "";
//     }
// }

/**
 * @brief This is a generalized function used to parse the message (after the
 * ID) to return a value at a certain position (or key) in the message.
 *
 * @param message the received string
 * @param key a key for the required information. It must not exceed the total
 * number of entries in the message 
 * @return String of the parsed value at the given key. In the case an invalid
 * key is used an empty string will be returned 
 */
String parse_by_key(String message, int key)
{
    char temp = '\0';
    int index = 0;
    int comma_count = 0;
    int counter = 0;
    String key_value = "";

    while(message[index] != '>')
    {
        if(message[index] == ',')
        {
            comma_count++;
        }
        index++;
    }
    
    log_d("parse_by_key() -> cmdlib-master.cpp -> The counter value is %d and there are %d commas in our code. \n", index, comma_count);
    index = 0;
    if(key > comma_count)
    {
        log_e("parse_by_key() -> cmdlib-master.cpp -> key exceeds number of entries");
        return "";
    }
    
    while(counter != key && counter <= comma_count && message[index] != '>')
    {
        if(message[index] == ',')
        {
            counter++;
        }
        index++;
    }

    while(message[index] != '>' && message[index] != ',')
    {
        temp = message[index];
        key_value += temp;
        index++;
    }
    log_d("the value of the given key is %s \n", key_value.c_str());
    return key_value;

};

bool send_battery_info(String bat_1_ID, String bat_2_ID, String bat_3_ID, String bat_4_ID, String bat_1_SOC, String bat_2_SOC, String bat_3_SOC, String bat_4_SOC)
{
    String temp = bat_1_ID + ", " + bat_1_SOC + " - "  + bat_2_ID + ", " + bat_2_SOC + " - " + bat_3_ID + ", " + bat_3_SOC + " - " + bat_4_ID + ", " + bat_4_SOC; 
    bt.send(temp);
    return true;
};

/**
 * @brief This command is used to create a new connection for wifi and store it
 * in the list of APs. It requires authentication first (command 4)
 *
 * @param message the received string
 * @return true if connection instruction is sent
 * @return false otherwise
 */
bool command_3_newConn(String message)
{
    Serial.println("sommand_3_newConn() -> cmdlib-master.cpp -> the initial message is: " + message);
    message = "<11" + message.substring(message.indexOf(','), message.indexOf('>') + 1);
    Serial.println("sommand_3_newConn() -> cmdlib-master.cpp -> the final message is: " + message);
    cmdsend(message);

    String ret = readStringUntilCustom('\n');
    return ret.isEmpty()? bt.send("error") : bt.send(ret);
};

/**
 * @brief This command compares the recieved string to the authorization code of
 * this unit to allow or restrict usage of commands. Must be called before any
 * other command.
 *
 * @param message the received string
 * @param auth_code the authorizatrion code of this unit 
 * @return true if authorization is successful
 * @return false otherwise
 */
bool command_4_auth(String message, String auth_code)
{
    String entered_code = parse_by_key(message, 1);
    
    if(entered_code == auth_code)
    {
        auth_flag = true;
        log_d("command_4_auth() -> cmdlib-master.cpp -> Authentication successful");
        return true;
    }
    else
    {
        log_e("command_4_auth() -> cmdlib-master.cpp -> Authentication unsuccessfful");
        return false;
    }
};

/**
 * @brief This command initiates battery swap mode and saves the initial cycles
 * for subtraction when the battery swap mode is exited
 *
 * @return true if initialization is successful
 * @return false otherwise
 */
bool command_5_enterSwap()
{
    cmdsend("<40>");
    initial_cycles = 3;
    log_d("command_5_enterSwap() -> cmdlib-master.cpp -> entered battery swap mode");
    String ret = readStringUntilCustom('\n');
    return ret.isEmpty()? bt.send("error") : bt.send(ret);
};

/**
 * @brief This command exits battery swap mode and takes the final cycles of the
 * swapped batteries and sends the difference via bluetooth.  
 *
 * @return true if sending is successful
 * @return false otherwise
 */
bool command_6_exitSwap(String towrite)
{
    final_cycles = 8;
    int difference = final_cycles - initial_cycles;
    String diff_str = (String)difference;
    //initial_cycles = 0;
    //final_cycles = 0;
    log_d("command_6_exitSwap() -> cmdlib-master.cpp -> successful typecast %S \n", diff_str);
    log_d("command_6_exitSwap() -> cmdlib-master.cpp -> exited battery swap mode");
    return bt.send(towrite);
};

/**
 * @brief this command tells the slave to check the wifi connection and send the status back to the master
 * 
 * @return true if command has been sent
 * @return false if command could not be sent
 */
bool command_7_checkWifi()
{
    cmdsend("<10>");
    log_d("command_7_checkWifi() -> cmmdlib-master.cpp -> wifi check request sent");
    String ret = readStringUntilCustom('\n');
    return ret.isEmpty()? bt.send("error") : bt.send(ret);
}

bool command_8_getTime()
{
    cmdsend("<30>");
    log_d("command_8_getTime() -> cmdlib-master.cpp -> time request sent");
    String ret = readStringUntilCustom('\n');
    return ret.isEmpty()? bt.send("error") : bt.send(ret);
}

/**
 * @brief This is the main wrapper function that is called in a loop and checks
 * for commands on bluetooth. It also checks if authorization has been granted
 * for different functions and resets authorization after every command call.
 *
 * @return true if command returns true
 * @return false otherwise
 */
bool command_bt(String towrite)
{
    String message = "";
    message = bt.check_bluetooth();
    
    if(message.length() > 0)
    {
        log_d("command_bt() -> cmdlib-master.cpp -> message received: %s \n", message);
        int ID = (10 * ((uint8_t)message[1] - 48)) + ((uint8_t)message[2] - 48);
        log_d("command_bt() -> cmdlib-master.cpp -> the authorization status is: %d \n", auth_flag);
        log_d("command_bt() -> cmdlib-master.cpp -> the the ID sent is: %d \n", ID);
        if(ID == 4)
        {
            return command_4_auth(message, AUTH_CODE);
        }
        else if(ID > 0 && ID < 100 && auth_flag)
        {
            switch(ID)
            {
                case 3: // connect to new credentials
                    auth_flag = false;
                    return command_3_newConn(message);
                case 5: // enter battery swapping mode
                    auth_flag = false;
                    return command_5_enterSwap();
                case 6: // exit battery swapping mode
                    auth_flag = false;
                    return command_6_exitSwap(towrite);
                case 7: // check wifi
                    auth_flag = false;
                    return command_7_checkWifi();
                case 8: // check time
                    auth_flag = false;
                    return command_8_getTime();
                default:
                    auth_flag = false;
                    log_e("command_bt() -> cmdlib-master.cpp -> invalid ID");
                    return false;
            }
        }
        else
        {
            log_e("command_bt() -> cmdlib-master.cpp -> entered invalid ID or authorization not met");
            return false;
        }

    }
    else
    {
        return false;
    }
}
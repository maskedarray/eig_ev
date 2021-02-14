#include <cmdlib-master.h>

#define AUTH_CODE "123456"
#define ESP_ID "012501202101"

bool auth_flag = false; // for authorization

void cmdinit(){
    Serial2.begin(115200);
}

bool cmdsend(String tosend){
    Serial2.print(tosend);
    log_d("the message sent to slave is: %s \r\n", tosend.c_str());
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
        vTaskDelay(10);
    } while(millis() - _startMillis < _timeout);
    log_d("timed out!\r\n");
    return -1;     // -1 indicates timeout
}

String readStringUntilCustom(char terminator, int _timeout)
{
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

ESP32Time rtc;
bool set_system_time(){
    cmdsend("<30>\r\n");

    String ret = readStringUntilCustom('\n', 20000);
    if(ret.length() > 0){
        String unixtime = parse_by_key(ret, 1);
        rtc.setTime(unixtime.toDouble(), 0);
        return true;
    }
    else{
        return false;
    }
}

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
    cmdsend("<11");
    cmdsend(message.substring(message.indexOf(','), message.indexOf('>') + 1));
    cmdsend("\r\n");

    String ret = readStringUntilCustom('\n', 15000);
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
        log_d("Authentication successful\r\n");
        return true;
    }
    else
    {
        log_e("Authentication unsuccessfful\r\n");
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
    cmdsend("<40>\r\n");
    log_d("entered battery swap mode. getting bss id from slave...\r\n");
    String ret = readStringUntilCustom('\n', 20000);
    return ret.isEmpty()? bt.send("error") : bt.send(ret);
};

/**
 * @brief This command exits battery swap mode and takes the final cycles of the
 * swapped batteries and sends the difference via bluetooth.  
 *
 * @return true if sending is successful
 * @return false otherwise
 */
bool command_6_exitSwap()
{
    log_d("exited battery swap mode\r\n");
    return true;
};

/**
 * @brief this command tells the slave to check the wifi connection and send the status back to the master
 * 
 * @return true if command has been sent
 * @return false if command could not be sent
 */
bool command_7_checkWifi()
{
    cmdsend("<10>\r\n");
    log_d("wifi check request sent. waiting for response...\r\n");
    String ret = readStringUntilCustom('\n', 5000);
    return ret.isEmpty()? bt.send("error") : bt.send(ret);
}

bool command_8_getTime()
{
    cmdsend("<30>\r\n");
    log_d("time request sent\r\n");
    String ret = readStringUntilCustom('\n', 5000);
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
bool command_bt()
{
    String message = "";
    message = bt.check_bluetooth();
    
    if(message.length() > 0)
    {
        log_d("message received: %s \r\n", message);
        int ID = (10 * ((uint8_t)message[1] - 48)) + ((uint8_t)message[2] - 48);
        log_d("the authorization status is: %d \r\n", auth_flag);
        log_d("the the ID sent is: %d \r\n", ID);
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
                    return command_6_exitSwap();
                case 7: // check wifi
                    auth_flag = false;
                    return command_7_checkWifi();
                case 8: // check time
                    auth_flag = false;
                    return command_8_getTime();
                default:
                    auth_flag = false;
                    log_e("invalid ID\r\n");
                    return false;
            }
        }
        else
        {
            log_e("entered invalid ID or authorization not met\r\n");
            return false;
        }

    }
    else
    {
        return false;
    }
}
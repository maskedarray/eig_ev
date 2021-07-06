#include <cmdlib-master.h>

#define PAIRING_ID "abcdef"



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
    
    index = 0;
    if(key > comma_count)
    {
        log_e("key exceeds number of entries ");
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
    return key_value;

};


/**
 * @brief This command is used to create a new connection for wifi and store it
 * in the list of APs.
 *
 * @param message the received string
 * @return empty string if connection instruction is sent
 * @return error string otherwise
 */
String command_3_newConn(String message)
{
    bool ret = false;
    String tempssid = parse_by_key(message, 1);
    String temppass = parse_by_key(message, 2);
    ret = wf.create_new_connection(tempssid.c_str(),temppass.c_str());
    return ret ? "" : "error";
};

/**
 * @brief This command initiates battery swap mode and saves the initial cycles
 * for subtraction when the battery swap mode is exited
 *
 * @return empty string if initialization is successful
 * @return error string otherwi
 */
String command_5_enterSwap()
{
/*     WiFi.begin(DEFAULT_BSS_WIFI_SSID,DEFAULT_BSS_WIFI_PASS);
    vTaskDelay(10000);
    String ret = "<40,";
    if(WiFi.isConnected() == true){
        //handle here
        WiFiClient client;
        if(client.connect("192.168.0.107",80)){
            log_d("client connected ");
            client.print("client1711\n");
            long time_start = millis();
            long time_stop = millis();
            while(time_stop - time_start < 5000){
                if(client.available()){
                    ret += client.readStringUntil('\n');
                    log_d("response received ");
                    break;
                }
                time_stop = millis();
                vTaskDelay(10);
            }
            client.stop();
            log_d("client disconnected ");
        }
    }
    else{
        log_e("could not connect to bss wifi ");
    }
    WiFi.disconnect(false,true);
    ret += ">";
    bt.send(ret);
    log_i("message sent to master: %s",ret.c_str()); */
    log_i("entered battery swap mode");
    return "";
};

/**
 * @brief This command exits battery swap mode and takes the final cycles of the
 * swapped batteries and sends the difference via bluetooth.  
 *
 * @return empty string if initialization is successful
 * @return error string otherwise
 */
String command_6_exitSwap()
{
    log_i("exited battery swap mode ");
    return "";
};

/**
 * @brief this command tells the slave to check the wifi connection and send the
 * status back to the master
 *
 * @return true if command has been sent
 * @return false if command could not be sent
 */
String command_7_checkWifi()
{
    bool ret = WiFi.isConnected();
    log_i("the connection status is: %d\n\r", ret);
    return ret ? "connected" : "disconnected";
}

/**
 * @brief This command returns the unix time
 * 
 * @return as string for the current unix time
 */
String command_8_getTime()
{
    String ret = unixTime();
    log_d("The returned time is: %s", ret);
    // bt.send(ret);
    return ret;
}

/**
 * @brief This is the main wrapper function that is called in a loop and checks
 * for commands on bluetooth.
 *
 * @return the command response string
 */
String command_bt(String message)
{

    if(message.length() > 0)
    {
        log_i("message received: %s", message.c_str());
        int ID = (10 * ((uint8_t)message[1] - 48)) + ((uint8_t)message[2] - 48);
        log_d("the the ID sent is: %d", ID);
        if(ID > 0 && ID < 100)
        {
            switch(ID)
            {
                case 3: // connect to new credentials
                    return command_3_newConn(message);
                case 5: // enter battery swapping mode
                    return command_5_enterSwap();
                case 6: // exit battery swapping mode
                    return command_6_exitSwap();
                case 7: // check wifi
                    return command_7_checkWifi();
                case 8: // check time
                    return command_8_getTime();
                default:
                    log_e("invalid ID ");
                    return "error";
            }
        }
        else
        {
            log_e("entered invalid ID ");
            return "error";
        }

    }
    else
    {
        return "error";
    }
}
#include <cmdlib-master.h>


#define AUTH_CODE "123456"
#define ESP_ID "012501202101"
// #define SEND_ACK_PIN 33
// #define ACK_WAIT 50         //equal to 1 second

bool auth_flag = false;
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
}

/**
 * @brief this functions checks serial port for availability of command data
 * If there is data available it parses and returns a number based on the command parsed
 * 
 * @return int is the number of commnad that is parsed
 */
int cmdreceive(){
    if(Serial2.available()){
        String received = Serial2.readStringUntil('\n');
        int cmdnum = (10 * ((uint8_t)received[0] - 48)) + ((uint8_t)received[1] - 48);
        switch(cmdnum){
            case 1:
                break;
            case 2:
                break;
            default:
                break;
        }
    }
    else{
        return 0;
    }
}

/**
 * @brief This function takes the received message as a string and parses it to
 * retrieve the ID as an integer. The ID signifies the command to be carried out
 * and the following data is parsed accordingly. The ID can take a value between
 * 1 and 99.
 *
 * @param message the received string
 * @return int the ID converted to int 
 */
int ID_parse(String message)
{
    message = message.substring(message.indexOf('<') + 1, message.indexOf('>'));
    String ID = message.substring(0, message.indexOf(','));
    if(ID.length() < 3)
    {
        Serial.println("ID: " + ID);
        return ID.toInt();
    }
    else
    {
        Serial.println("ID exceeds set limit");
        ID = "";
        return 0;
    }
};

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
    
    log_d("parse_by_key() -> cmdlib.hpp -> The counter value is %d and there are %d commas in our code. \n", index, comma_count);
    index = 0;
    if(key > comma_count)
    {
        log_e("parse_by_key() -> cmdlib.hpp -> key exceeds number of entries");
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

/**
 * @brief This command is used to create a new connection for wifi and store it
 * in the list of APs. It requires authentication first (command 4)
 *
 * @param message the received string
 * @return true if connection instruction is sent
 * @return false otherwise
 */
bool command_3(String message)
{
    cmdsend(message);
    return true;
    //return wf.create_new_connection(SSID.c_str(), Password.c_str());
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
bool command_4(String message, String auth_code)
{
    String entered_code = parse_by_key(message, 1);
    
    if(entered_code == auth_code)
    {
        auth_flag = true;
        log_d("command_4() -> cmdlib.hpp -> Authentication successful");
        return true;
    }
    else
    {
        log_e("command_4() -> cmdlib.hpp -> Authentication unsuccessfful");
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
bool command_5()
{
    cmdsend("<40>");
    initial_cycles = 3; 
    log_d("command_5() -> cmdlib.hpp -> entered battery swap mode");
    return true;
};

/**
 * @brief This command exits battery swap mode and takes the final cycles of the
 * swapped batteries and sends the difference via bluetooth.  
 *
 * @return true if sending is successful
 * @return false otherwise
 */
bool command_6()
{
    final_cycles = 8;
    int difference = final_cycles - initial_cycles;
    String diff_str = (String)difference;
    initial_cycles = 0;
    final_cycles = 0;
    log_d("command_6() -> cmdlib.hpp -> successful typecast %S \n", diff_str);
    log_d("command_6() -> cmdlib.hpp -> exited battery swap mode");
    return bt.send(diff_str);
};

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
    log_d("message received: %s \n", message);
    if(message.length() > 0)
    {
        int ID = (10 * ((uint8_t)message[1] - 48)) + ((uint8_t)message[2] - 48);
        log_d("the authorization status is: %d \n", auth_flag);
        if(ID == 4)
        {
            return command_4(message, AUTH_CODE);
        }
        else if(ID > 0 && ID < 100 && auth_flag)
        {
            switch(ID)
            {
                case 3:
                    auth_flag = false;
                    return command_3(message);
                case 5:
                    auth_flag = false;
                    return command_5();
                case 6:
                    auth_flag = false;
                    return command_6();
                default:
                    auth_flag = false;
                    log_e("invalid ID");
                    return false;
            }
        }
        else
        {
            log_e("entered invalid ID or authorization not met");
            return false;
        }

    }
    else
    {
        log_e("invalid message");
        return false;
    }
}
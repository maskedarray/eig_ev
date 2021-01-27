#include <cmdlib-master.h>

#define AUTH_CODE "123456"
#define ESP_ID "012501202101"

bool auth_flag = false;
int initial_cycles = 0;
int final_cycles = 0;

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
    
    Serial.printf("parse_by_key() -> cmdlib.hpp -> The counter value is %d and there are %d commas in our code. \n", index, comma_count);
    index = 0;
    if(key > comma_count)
    {
        Serial.println(F("parse_by_key() -> cmdlib.hpp -> key exceeds number of entries"));
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
    Serial.printf("the value of the given key is %s \n", key_value.c_str());
    return key_value;

};

/**
 * @brief This command is used to create a new connection for wifi and store it
 * in the list of APs. It requires authentication first (command 4)
 *
 * @param message the received string
 * @return true if connection is successful
 * @return false otherwise
 */
bool command_3(String message)
{
    String SSID = parse_by_key(message, 1);
    String Password = parse_by_key(message, 2);
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
        Serial.println(F("command_4() -> cmdlib.hpp -> Authentication successfful"));
        return true;
    }
    else
    {
        Serial.println(F("command_4() -> cmdlib.hpp -> Authentication unsuccessfful"));
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
    initial_cycles = 3;   
    Serial.println(F("command_5() -> cmdlib.hpp -> entered battery swap mode"));
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
    Serial.println("command_6() -> cmdlib.hpp -> successful typecast " + diff_str);
    Serial.println(F("command_6() -> cmdlib.hpp -> exited battery swap mode"));
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
    int ID = 0;
    message = bt.check_bluetooth();
    Serial.println("command_bt() -> cmdlib.hpp -> message received: " + message);
    if(message.length() > 0)
    {
        ID = ID_parse(message);
        Serial.printf("command_bt() -> cmdlib.hpp -> the authorization status is: %d \n", auth_flag);
        if(ID == 4 && !auth_flag)
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
                    Serial.println("command_bt() -> cmdlib.hpp -> invalid ID");
                    return false;
            }
        }
        else
        {
            Serial.println(F("command_bt() -> cmdlib.hpp -> entered invalid ID or authorization not met"));
            return false;
        }

    }
    else
    {
        Serial.println(F("command_bt() -> cmdlib.hpp -> invalid message"));
        return false;
    }
}
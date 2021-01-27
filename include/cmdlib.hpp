#include <bluetooth.h>
#include <ESPWiFi.h>
#include <arduino.h>

#define AUTH_CODE "123456"
#define ESP_ID "012501202101"

bool auth_flag = false;
int initial_cycles = 0;
int final_cycles = 0;

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

bool command_3(String message)
{
    String SSID = parse_by_key(message, 1);
    String Password = parse_by_key(message, 2);
    return wf.create_new_connection(SSID.c_str(), Password.c_str());
};

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

bool command_5()
{
    initial_cycles = 3;   
    Serial.println(F("command_5() -> cmdlib.hpp -> entered battery swap mode"));
    return true;
};

bool command_6()
{
    final_cycles = 8;
    int difference = final_cycles - initial_cycles;
    String diff_str = (String)difference;
    Serial.println("command_6() -> cmdlib.hpp -> successful typecast " + diff_str);
    Serial.println(F("command_6() -> cmdlib.hpp -> exited battery swap mode"));
    return bt.send(diff_str);
};

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
};

/*             switch(ID.toInt())
            {
                case 3: // An ID of 3 means data is SSID and Password
                    this->wifi_parse(message, entry_1, entry_2);
                    auth_flag1 = false;
                    auth_flag = auth_flag1;
                    break;
                default:
                    
                    break;
            } */
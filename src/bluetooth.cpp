#include <bluetooth.h>


char BT_incoming[32]; // array of characyters, to read incoming data
bool auth_flag1 = false;
//TODO: return statements are dummy. handle the scenario when connection fails 
//or sending of data fails

/**
 * This function initializes the bluetooth. The name of bluetooth is defined in
 * macro BLUETOOTH_NAME present in header file
 * 
 * @return true if bluetooth initialization is successful. false if some error occurs
 * 
 */
bool ESP_BT::init(){
    SerialBT.begin(BLUETOOTH_NAME); //Name of your Bluetooth Signal
    log_d("Bluetooth Device is Ready to Pair");
    return true;
}

/**
 * Sends string of CSV data on bluetooth
 * 
 * @param[in] tosend is the string of data to send on bluetooth
 * @return true if data is sent. false if no data is sent.
 */
bool ESP_BT::send(String tosend){
    long len = tosend.length();
    SerialBT.print("%S%");    //start byte
    SerialBT.print(String(len));
    SerialBT.print(",");
    SerialBT.print(tosend);   //data, first parameter is length of data starting from next value (after comma)
    SerialBT.print("%S%");    //end byte
    SerialBT.println();       //end byte
    return true;
}

/**
 * @brief This function reads and returns a serially transmitted message from
 * the controller (phone application). It is built with generalizability in mind
 * and records everything in <>.
 *
 * @return String 
 */
String ESP_BT::bt_read() 
{
    int32_t size = 0;
    char temp = '\0';
    String BTread = "";
    temp = SerialBT.read();
    if(temp == '<')
    {
        while(temp != '>' && size <= 90) // ID size will be fixed, needs to be addressed
        {
            BTread += temp;
            temp = SerialBT.read();
            size++;
        }
        if(size >= 90)
        {
            log_d("Credentials exceed set limit");
            return "";
        }
        BTread += temp;
    }
    return BTread;
}


/**
 * @brief This is a wrapper function which checks for incoming bluetooth
 * messages and stores them accordingly. It uses some other functions defined
 * previously in this library. 
 *
 * @return String message
 */
String ESP_BT::check_bluetooth()
{
    if (this->SerialBT.available())
    {
        return this->bt_read();
    }
    else
    {
        return "";
    }
}

ESP_BT bt;
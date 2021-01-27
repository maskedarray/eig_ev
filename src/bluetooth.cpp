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
    Serial.println(F("init_bt() -> bluetooth.cpp -> Bluetooth Device is Ready to Pair"));
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
    
    if (SerialBT.available()){ //Check if we receive anything from Bluetooth
        for(int i=0; i<4; i++){ // reading four bytes only, need to send data to esp in a standardized form (delimiters)
            BT_incoming[i] = SerialBT.read();
        }
        Serial.print("Received:"); Serial.println(BT_incoming);
    }
}

/**
 * @brief This function reads and returns a serially transmitted message from
 * the controller (phone application). It is built with generalizability in mind
 * and records everything in <>.
 *
 * @return String 
 */
String ESP_BT::bt_read() // TODO: take ID first and define various reads according to it
{
    int32_t size = 0;
    char start = '<';
    char end = '>';
    char temp = '\0';
    String BTread = "";
    temp = SerialBT.read();
    if(temp == start)
    {
        while(temp != end && size <= 90) // ID size will be fixed, needs to be addressed
        {
            BTread += temp;
            temp = SerialBT.read();
            size++;
        }
        if(size >= 90)
        {
            Serial.println("Credentials exceed set limit");
            return "";
        }
        BTread += temp;
    }
    return BTread;
}

/**
 * This parses the text provided to return an SSID and Password.
 */
void ESP_BT::wifi_parse(String text, String &Username, String &Password)
{
    text = text.substring(text.indexOf(',') + 1, text.indexOf('>') + 1);
    Username = text.substring(0, text.indexOf(','));
    text = text.substring(text.indexOf(',') + 1, text.indexOf('>') + 1);
    Password = text.substring(0, text.indexOf('>'));
    return;
}

/**
 * @brief This parses the given data to get a unique identifier for the specific EV.
 * 
 * @param text the received text to be read
 * @param unique_identifier the unique identifer parsed
 */
void ESP_BT::EV_ID_parse(String text, String &unique_identifier)
{
    text = text.substring(text.indexOf(',') + 1, text.indexOf('>') + 1);
    unique_identifier = text.substring(0, text.indexOf('>'));
    return;
}

/**
 * This is a simple diagnostic function for debugging purposes.
 */
void ESP_BT::display(String ID, String Username, String Password)
{
    Serial.print("display() -> bluetooth.cpp -> ID: " + ID + "\n");
    Serial.print("display() -> bluetooth.cpp -> Username: " + Username +  "\n");
    Serial.print("display() -> bluetooth.cpp -> Password: " + Password + "\n");
}

/**
 * @brief This is a wrapper function which checks for incoming bluetooth messages and stores them accordingly. It uses some other functions defined previously in this library. 
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
#include <bluetooth.h>


char BT_incoming[32]; // array of characyters, to read incoming data

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
    tosend = String(len) + "," + tosend;
    SerialBT.print("%S%");    //start byte
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
 * This function reads an incoming serial message and parses it. The output is
 * passed to the parameters, which are referenced. The value of the ID will
 * change the output and the parse function in use.
 */ 
bool ESP_BT::bt_read(String &ID, String &Username, String &Password) // TODO: take ID first and define various reads according to it
{
    int32_t size = 0;
    char start = '<';
    char end = '>';
    char temp = '\0';
    String BTread = "";
    ID = "";
    Username = "";
    Password = "";
    temp = SerialBT.read();
    if(temp == start)
    {
        while(temp != end && size <= 90) // ID size will be fixed, needs to be addressed
        {
            BTread += temp;
            temp = SerialBT.read();
            size++;
        }
        if(size > 90)
        {
            Serial.println("Credentials exceed set limit");
            ID = "";
            Username = "";
            Password = "";
            return false;
        }
        BTread += temp;
        ID = BTread.substring(BTread.indexOf('<') + 1, BTread.indexOf(','));
        if(ID.length() < 3)
        {
            Serial.println("ID: " + ID);
        }
        else
        {
            Serial.println("ID exceeds set limit");
            ID = "";
            Username = "";
            Password = "";
            return false;
        }
        switch(ID.toInt())
        {
            case 3: // An ID of 3 means data is SSID and Password
                this->wifi_parse(BTread, Username, Password);
                break;
            default:
                break;
        }
    }
    return true;
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
 * @param ID refers to the parsed ID. This dictates the purpose of the message as well as the function needed to be performed 
 * @param Username refers to the first String entry parsed
 * @param Password refers to the second String entry parsed
 * @return true if bluetooth is available and read is successful
 * @return false if otherwise
 */
bool ESP_BT::check_bluetooth(String &ID, String &Username, String &Password)
{
    if (this->SerialBT.available() && !got_credentials)
    {
        got_credentials = this->bt_read(ID, Username, Password);
        got_credentials = !got_credentials;
        return !got_credentials;
    }
    else
    {
        return false;
    }
    
}

ESP_BT bt;
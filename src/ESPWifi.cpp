#include <ESPWiFi.h>
//TODO: optimize wifi reconnection time. 
/**
 * This function initializes the ESP wifi module with the requisite
 * configurations. It attempts to connect to a default SSID and password and
 * also updates the vector for APs in the WiFiMulti class.
 */
bool ESP_WiFi::init()
{
    Serial.println("init() -> ESP_WiFi.cpp -> updating AP list from SD card");
    this -> update_APs();
    return true;
}

/**
 * This function adds new APs to the WiFiMulti class. This should only be called
 * on the first initialization as it may result in multiple instances of the
 * same credentials in the vector.
 */
void ESP_WiFi::update_APs()
{
    int i = 0;
    //String SSIDs[10]; String Passwords[10];
    storage.return_APList(SSID_List, Password_List);
    access_points = new WiFiMulti();
    while(!SSID_List[i].isEmpty() && i < 10)
    {
        access_points->addAP(SSID_List[i].c_str(), Password_List[i].c_str());
        Serial.println(F("update_APs() -> ESP_WiFi.cpp -> Added AP"));
        Serial.println("SSID: " + SSID_List[i]);
        Serial.println("Password: " + Password_List[i]);
        i++;
    }
    credential_length = i;
    Serial.printf("update_APs() -> ESP_WiFi.cpp -> credential length: %u \n", credential_length);
}

/**
 * This function adds a new connection to the vector in the WiFiMulti class and
 * the text file for APs in the Sd card. In the situation that the SSID sent is
 * the same as one that already exists, the function will either fail if the
 * password is the same or update the specific entry if not.
 */
bool ESP_WiFi::create_new_connection(const char *SSID, const char *Password)
{
    Serial.println(F("create_new_connection() -> ESP_WiFi.cpp -> connecting to new AP"));
    WiFi.begin(SSID, Password);

    int32_t timer = 0;
    while(!WiFi.isConnected()) // Checking if credentials conect with an AP
    {
        if(timer > 15) // Timeout
        {
            Serial.println(F("create_new_connection() -> ESP_WiFi.cpp -> Failed to connect. Please check SSID and Password"));
            return false;
        }
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(500);
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(500);
        timer += 1;
    }

    // Connection has been established
    Serial.printf("create_new_connection() -> ESP_WiFi.cpp -> credential_length = %u \n", credential_length);
    if(credential_length >= 10) // Check if AP data limit reached
    {
        // clear all data and write single AP along with default
        int i = 0;
        Serial.println(F("create_new_connection() -> ESP_WiFi.cpp -> recreating AP data"));
        while(!SSID_List[i].isEmpty() || !Password_List[i].isEmpty())
        {
            SSID_List[i].clear();
            Password_List[i].clear();
            i++;
        }
        i = 0;
        SSID_List[0] = DEFAULT_SSID;
        Password_List[0] = DEFAULT_PASSWORD;
        SSID_List[1] = (String) SSID;
        Password_List[1] = (String) Password;
        credential_length = 2;
        if(remake_access_points() && storage.rewrite_storage_APs(SSID_List, Password_List))
        {
            return true;
        }
        else
        {
            Serial.println(F("create_new_connection() -> ESP_WiFi.cpp -> Error"));
            return false;
        }
    }

    for(int i = 0; i < 10; i++) // Check if the SSID matches with a pre-existing entry 
    {
        if(SSID_List[i].isEmpty())
        {
            break;
        }
        Serial.println(SSID_List[i]);
        if((String) SSID == SSID_List[i])
        {
            if((String) Password == Password_List[i]) // Same credentials were entered
            {
                Serial.println(F("create_new_connection() -> ESP_WiFi.cpp -> Credentials already exist"));
                return false;
            }
            else // Password was different
            {
                Serial.println(F("create_new_connection -> ESP_WiFi.cpp -> Updating Password"));
                // add code to update all lists within if condition (must be
                // type bool)
                Password_List[i] = (String) Password;
                if(remake_access_points() && storage.rewrite_storage_APs(SSID_List, Password_List))
                {
                    return true;
                }
            }
        }
    }
    storage.write_AP(SSID, Password); // Add to storage
    access_points->addAP(SSID, Password); // Add to WiFiMulti Class
    credential_length++; // Add to credential length
    return true;
}


/**
 * This function serves to destroy and recreate a WiFiMulti class in order to
 * change the list of APs. This is done as the AP list (vector) is a private
 * variable
 */
bool ESP_WiFi::remake_access_points()
{
    delete access_points;
    access_points = new WiFiMulti();
    int i = 0;
    while(!SSID_List[i].isEmpty() && i < 10)
    {
        access_points->addAP(SSID_List[i].c_str(), Password_List[i].c_str());
        Serial.printf("remake_access_points() -> ESP_WiFi.cpp -> %s and %s added \n", SSID_List[i].c_str(), Password_List[i].c_str());
        i++;
    }
    return true;
}


/**
 * This fucntion is used to connect to the nearest available access point.
 */
bool ESP_WiFi::connect_to_nearest()
{
    if(access_points->run() == WL_CONNECTED){
        Serial.println(F("connect_to_nearest() -> ESP_WiFi.cpp -> Connection established"));
        return true;
    }
    else{
        Serial.println(F("connect_to_nearest() -> ESP_WiFi.cpp -> Connection timed out"));
        return false;
    }
}

/**
 * @brief This function is a wrapper function which checks the status of the
 * wifi connection. It is designed to be used in a loop. Can be altered
 * according to requirements.
 *
 * @return previous connection exists 
 * @return previous connection doesn't exist
 */
bool ESP_WiFi::check_connection()
{
    if (WiFi.isConnected()) // Check WiFi connection
    {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("Connected to: " + WiFi.SSID());
        return true;
    }
    else
    {
        Serial.println("WiFi not connected. Establishing connection");
        if(this->connect_to_nearest()){
            digitalWrite(LED_BUILTIN,HIGH);
            return true;
        } else {
            digitalWrite(LED_BUILTIN,LOW);
            return false;
        }
    }
}

ESP_WiFi wf;
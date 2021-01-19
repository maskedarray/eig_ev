/* This library contains all the wrapper funcions for main.cpp the relevant
library inclusions are also added here to clean up the main code.*/

#include <bluetooth.h>
#include <ESPWifi.h>
#include <Storage.h>

#define LED 2

String CSVText;
String ID;
String Username;
String Password;
bool got_credentials = false;

bool check_connection()
{
    if (WiFi.isConnected()) // Check WiFi connection
    {
        digitalWrite(LED, HIGH);
        Serial.println("Connected to: " + WiFi.SSID());
        delay(1000);
        return true;
    }
    else
    {
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);
        delay(50);
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);

        Serial.println("WiFi not connected. Establishing connection");
        wf.connect_to_nearest();
        delay(100);
        return false;
    }
};

void check_bluetooth()
{
    if (bt.SerialBT.available() && !got_credentials)
    {
        got_credentials = bt.bt_read(ID, Username, Password);
    }
};

void connect_to_new_credentials()
{
    if (got_credentials)
    {
        if (ID == "3") // Check ID for correct function to run
        {
            if (wf.create_new_connection(Username.c_str(), Password.c_str()))
            {
                digitalWrite(LED, HIGH);
                Serial.printf("Connection Status: %d\n", WiFi.status());
                Serial.println("Connected to: " + WiFi.SSID());
            }
        }
        else // Turn to switch statementsin the event that we use more functions according to ID
        {
            Serial.println("entered the wrong ID");
        }
        got_credentials = false;
    }
};
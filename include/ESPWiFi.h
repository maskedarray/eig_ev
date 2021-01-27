#ifndef __ESPWIFI_H__
#define __ESPWIFI_H__
#include <WiFi.h>
#include <WiFiMulti.h> // for multiple APs and connecting to the closest
#include <Arduino.h>
#include <Storage.h> // to access APs.txt

#define LED 2
#define DEFAULT_SSID "EiG"
#define DEFAULT_PASSWORD "12344321"

/**
 * This library adds functions to connect to the closest available AP for WiFi
 * or create a new AP for use. The maximum limit for the number of credentials
 * is set to limit the number of APs to cycle through
 */
class ESP_WiFi
{
    private:
        int32_t timer;
        int32_t credential_length;
        String temp_ID;
        String temp_entry_1;
        String temp_entry_2;

    public:
        String SSID_List[10];
        String Password_List[10];
        WiFiMulti *access_points;
        bool init();
        bool create_new_connection(const char *SSID, const char *Password);
        bool remake_access_points();
        void update_APs();
        bool connect_to_nearest();
        bool check_connection();
};

extern ESP_WiFi wf;

#endif
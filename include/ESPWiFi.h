#ifndef __ESPWIFI_H__
#define __ESPWIFI_H__
#include <WiFi.h>
#include <WiFiMulti.h> // for multiple APs and connecting to the closest
#include <Arduino.h>
#include <Storage.h> // to access APs.txt
#include <defines.h>

#define DEFAULT_SSID "EiG"
#define DEFAULT_PASSWORD "12344321"

/**
 * This library adds functions to connect to the closest available AP for WiFi
 * or create a new AP for use. The maximum limit for the number of credentials
 * is set to limit the number of APs to cycle through
 * NOTE: This library includes other libraries. Any changes in those libraries
 * will affect this library. 
 */
class ESP_WiFi
{
    private:
        int32_t credential_length;
        bool remake_access_points();
        void update_APs();

    public:
        String SSID_List[10];
        String Password_List[10];
        WiFiMulti *access_points;
        bool init();
        bool create_new_connection(const char *SSID, const char *Password);
        bool connect_to_nearest();
        bool check_connection();
};

extern ESP_WiFi wf;

#endif
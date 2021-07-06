#ifndef __CMDLIB_H__
#define __CMDLIB_H__
#include <Arduino.h>
#include <FreeRTOS.h>
#include <ESP32Time.h>
#include <ESPWiFi.h>
#include <rtc.h>

#define DEFAULT_BSS_WIFI_SSID "EiG"
#define DEFAULT_BSS_WIFI_PASS "12344321"

String command_bt(String message);


#endif
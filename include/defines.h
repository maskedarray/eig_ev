#ifndef __DEFINES_H__
#define __DEFINES_H__
#include <Arduino.h>
#include <ESP32Time.h>

extern ESP32Time __esptime;
extern const char *registry_id;
extern char *device_id;
extern String EV_ID;
enum flags_ {rtc_f, sd_f, bt_f, can_f, cloud_blink_f, cloud_f, bt_blink_f, can_blink_f, sd_blink_f};
extern byte flags[16];

#endif
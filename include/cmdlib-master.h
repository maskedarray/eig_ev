#ifndef __CMDLIB_H__
#define __CMDLIB_H__
#include <bluetooth.h>
#include <Arduino.h>
#include <FreeRTOS.h>
#include <ESP32Time.h>

bool command_bt();
bool cmdsend(String tosend);
void cmdinit();
bool set_system_time();


#endif
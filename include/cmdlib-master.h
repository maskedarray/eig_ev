#ifndef __CMDLIB_H__
#define __CMDLIB_H__
#include <bluetooth.h>
#include <Arduino.h>
#include <FreeRTOS.h>

bool command_bt(String towrite);
bool cmdsend(String tosend);
void cmdinit();


#endif
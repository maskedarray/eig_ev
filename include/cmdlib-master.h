#ifndef __CMDLIB_H__
#define __CMDLIB_H__
#include <bluetooth.h>
#include <arduino.h>
#include <FreeRTOS.h>

bool command_bt();
// bool cmdsend_ack(String tosend);
bool cmdsend(String tosend);
void cmdinit();
int cmdreceive();

#endif
#ifndef _RTC_H_
#define _RTC_H_
#include <Arduino.h>    //for using arduino String

bool initRTC();
String getTime();
String getTime2();
String unixTime();
String getNextDay(int,int,int);

#endif
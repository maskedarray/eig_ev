#include <rtc.h>
#include <RTClib.h>

RTC_DS3231 rtc;
//TODO: Update time from internet after a while
/*
 * Function initRTC initializes RTC and adjusts date and time in case of power loss
 * TODO: Handle error in case RTC initialization fails
 */
bool initRTC(){
    if(rtc.begin()){
        if (rtc.lostPower()) {
            log_d("Readjusting RTC date and time \r\n");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // sets the clock to time when code was burned 
        }
        log_d("RTC initialization successful\r\n");
        return true;
    }
    else{
        log_d("RTC initialization failed\r\n");
        return false;
    }
}


/*
 * Function getTime returns time from RTC hardware in form of string. 
 * The format is : "YYYY-MM-DD HH:MM:SS"
 * TODO: Optimize the function by replacing individual strings in the return statement.
 *       This will reduce the temporary ram usage
 */
String getTime(){
    DateTime now = rtc.now();
    String YYYY = String(now.year(), DEC);
    String mm = String(now.month(), DEC);
    if (mm.length() == 1){mm = "0" + mm; }
    String dd = String(now.day(), DEC);
    if (dd.length() == 1){dd = "0" + dd; }
    String HH = String(now.hour(), DEC);
    if (HH.length() == 1){HH = "0" + HH; }
    String MM = String(now.minute(), DEC);
    if (MM.length() == 1){MM = "0" + MM; }
    String SS = String(now.second(), DEC);
    if (SS.length() == 1){SS = "0" + SS; }
    return  (YYYY + "-" + mm + "-" + dd + " " + HH + ":" + MM + ":" + SS) ; 
}

/*
 * Function getTime2 returns time from RTC hardware in form of string.
 * The format is: "YYYYMMDD"
 * Difference from getTime() function is the format. It is required for writing data in file
 */

  String getTime2(){
    DateTime now = rtc.now();
    String YYYY = String(now.year(), DEC);
    String mm = String(now.month(), DEC);
    if (mm.length() == 1){mm = "0" + mm; }
    String dd = String(now.day(), DEC);
    if (dd.length() == 1){dd = "0" + dd; }
    return YYYY + mm + dd;
  }

String getNextDay(int iyear, int imonth, int iday){
    DateTime temp1(iyear,imonth,iday,0,0,0);
    TimeSpan temp2(1,0,0,0);
    temp1 = temp1 + temp2;
    String YYYY = String(temp1.year(), DEC);
    String mm = String(temp1.month(), DEC);
    if (mm.length() == 1){mm = "0" + mm; }
    String dd = String(temp1.day(), DEC);
    if (dd.length() == 1){dd = "0" + dd; }
    return (YYYY + mm + dd);
}

String unixTime(){
    DateTime now = rtc.now();
    return String(now.unixtime());
}
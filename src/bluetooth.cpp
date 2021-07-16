#include <bluetooth.h>


char BT_incoming[32]; // array of characters, to read incoming data
bool auth_flag1 = false;
//TODO: return statements are dummy. handle the scenario when connection fails 
//or sending of data fails

/**
 * This function initializes the bluetooth. The name of bluetooth is defined in
 * macro BLUETOOTH_NAME present in header file
 * 
 * @return true if bluetooth initialization is successful. false if some error occurs
 * 
 */
bool ESP_BT::init(){
    
    // The following part until the next comment consists of initializations
    // that need to be carried out every time the device starts
    Serial2.begin(115200);
    String samp;
    isConnected = false; // Connection not established at initialization
    delay(50);
    // TODO: Add a check for if the device has set up the module in order to run
    // the following part only once

    //Check if AT Commands are working
    Serial2.write("AT");
    delay(50);
    log_d("%d", Serial2.baudRate());

    // add a while instead of an if and also include a timeout
    if(Serial2.available())
    {
        samp = Serial2.readStringUntil('\n');
    }
    else{
        log_e("error in bluetooth");
        return false;
    }
    if(samp == "OK")
    {
        log_d("AT Commands work");
    }
    else{
        log_e("error in bluetooth");
        return false;
    }
    log_i("Bluetooth Device is Ready to Pair");
    return true;
}

/**
 * Sends string of CSV data on bluetooth
 * 
 * @param[in] tosend is the string of data to send on bluetooth
 * @return true if data is sent. false if no data is sent.
 */
bool ESP_BT::send(String tosend){
    if(isConnected)
    {
        long len = tosend.length();
        tosend = "%S%" + String(len) + "," + tosend + "%S%\r\n";
        Serial2.write(tosend.substring(0,(int)(len/3)).c_str());
        vTaskDelay(200);
        Serial2.write(tosend.substring((int)(len/3),(int)((2*len)/3)).c_str());
        vTaskDelay(200);
        Serial2.write(tosend.substring((int)((2*len)/3),len+13).c_str());
        return true;
    }
    else
    {
        log_w("Device isn't connected, sending aborted");
        return false;
    }
}

/**
 * @brief This function reads and returns a serially transmitted message from
 * the controller (phone application). It is built with generalizability in mind
 * and records everything in <>.
 *
 * @return String 
 */
String ESP_BT::bt_read() 
{
    // Read the message of a connected device and check if it conforms to the
    // set conventions
    log_d("The device connection status is: %d", isConnected);
    if(isConnected) 
    {
        int32_t size = 0;
        char temp = '\0';
        String BTread = "";
        temp = Serial2.read();
        if(temp == '<')
        {
            while(temp != '>' && size <= 90) // ID size will be fixed, needs to be addressed
            {
                BTread += temp;
                temp = Serial2.read();
                size++;
            }
            if(size >= 90)
            {
                log_e("Credentials exceed set limit");
                return "";
            }
            BTread += temp;
        }
        else // we got some other message in AT format
        {
            BTread = temp + Serial2.readString();
            log_d("%s", BTread.c_str());
            return BTread;
        }
        log_d("%s", BTread.c_str());
        return BTread;
    }

    /**
     * TODO: add the custom timed read and read string functions
     */
    else
    {
        // Read message without checking for message conventions
        String BTread = Serial2.readString();
        log_d("%s", BTread.c_str());
        return BTread;
    }
}


/**
 * @brief This is a wrapper function which checks for incoming bluetooth
 * messages and processes them accordingly. It first checks if the device has
 * bonded to another device by checking AT responses and then recieves messages
 * from the central device
 *
 * @return String message
 */
String ESP_BT::check_bluetooth()
{
    String msg;
    if (Serial2.available())
    {
        msg = this->bt_read();
        log_d("The message recieved is %s", msg.c_str());
        // Check for connection
        if(msg == "OK+CONN")
        {
            isConnected = true;
            return "";
        }
        else if(msg == "OK+LOST")
        {
            isConnected = false;
            return "";
        }

        if(isConnected)
        {
            return msg;
        }

    }
    else
    {
        return "";
    }
}

ESP_BT bt;
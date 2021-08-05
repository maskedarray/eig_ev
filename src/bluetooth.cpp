#include <bluetooth.h>


// char BT_incoming[32]; // array of characters, to read incoming data
// bool auth_flag1 = false;
//TODO: return statements are dummy. handle the scenario when connection fails 
//or sending of data fails
static NimBLEServer* pServer;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();

        // set connection status to true
        bt.isConnected = true;
    };
    /** Alternative onConnect() method to extract details of the connection. 
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */  
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 5x interval time for best results.  
         */
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);

        // Start Security
        NimBLEDevice::startSecurity(desc->conn_handle);

        // set connection status to true
        bt.isConnected = true;
    };
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
        bt.isConnected = false;
    };
    
/********************* Security handled here **********************
****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
        Serial.println("Server Passkey Request");
        /** This should return a random 6 digit number for security 
         *  or make your own static passkey as done here.
         */
        return 123456; 
    };

    bool onConfirmPIN(uint32_t pass_key){
        Serial.print("The passkey YES/NO number: ");Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true; 
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        /** Check that encryption was successful, if not we disconnect the client */  
        if(!desc->sec_state.encrypted) {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
        /*String msg = command_bt(pCharacteristic->getValue().c_str());
        Serial.println(msg);
        bt.send(msg);*/
        String temp = pCharacteristic->getValue().c_str();
        Serial.println("the last char of the captured string: " + *(temp.end() - 1));
        if(temp[0] == '<')
        {
            bt.bt_msg = temp;
            bt.commandInQueue = true;
        }
        else if(bt.commandInQueue && *(temp.end() - 1) != '>')
        {
            bt.bt_msg += temp;
        }
        else if(bt.commandInQueue && *(temp.end() - 1) == '>')
        {
            bt.bt_msg += temp;
            bt.commandInQueue = false;
            bt.commandComplete = true;
        }
        else
        {
            bt.bt_msg = "";
        }
    };

    /** Called before notification or indication is sent, 
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", "; 
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};
    
/** Handler class for descriptor actions */    
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
        std::string dscVal((char*)pDescriptor->getValue(), pDescriptor->getLength());
        Serial.print("Descriptor witten value:");
        Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
        Serial.print(pDescriptor->getUUID().toString().c_str());
        Serial.println(" Descriptor read");
    };
};

/** Define callback instances globally to use for multiple Charateristics \ Descriptors */ 
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;

/**
 * This function initializes the bluetooth. The name of bluetooth is defined in
 * macro BLUETOOTH_NAME present in header file
 * 
 * @return true if bluetooth initialization is successful. false if some error occurs
 * 
 */
bool ESP_BT::init()
{
    Serial.println("Starting NimBLE Server");

    /** set connection status as false */
    this->isConnected = false;
    this->commandInQueue = false;

    /** sets device name */
    NimBLEDevice::init("NimBLE-Arduino");

    /** Optional: set the transmit power, default is 3db */
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    
    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /** 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, secure connections.
     *   
     *  These are the default values, only shown here for demonstration.   
     */ 
    //NimBLEDevice::setSecurityAuth(true, true, true); 
    NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM | BLE_SM_PAIR_AUTHREQ_SC);
    NimBLEDevice::setSecurityPasskey(123456);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pDeadService = pServer->createService("DEAD");
    NimBLECharacteristic* pBeefCharacteristic = pDeadService->createCharacteristic(
                                               "BEEF",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                               /** Require a secure connection for read and write access */
                                               NIMBLE_PROPERTY::READ_ENC |  // only allow reading if paired / encrypted
                                               NIMBLE_PROPERTY::WRITE_ENC   // only allow writing if paired / encrypted
                                              );
  
    /**
     * TODO: change this
     */
    pBeefCharacteristic->setValue("Burger");
    pBeefCharacteristic->setCallbacks(&chrCallbacks);

    /** 2904 descriptors are a special case, when createDescriptor is called with
     *  0x2904 a NimBLE2904 class is created with the correct properties and sizes.
     *  However we must cast the returned reference to the correct type as the method
     *  only returns a pointer to the base NimBLEDescriptor class.
     */
    NimBLE2904* pBeef2904 = (NimBLE2904*)pBeefCharacteristic->createDescriptor("2904"); 
    pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
    pBeef2904->setCallbacks(&dscCallbacks);
  

    NimBLEService* pBaadService = pServer->createService("BAAD");
    NimBLECharacteristic* pFoodCharacteristic = pBaadService->createCharacteristic(
                                               "F00D",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::READ_ENC |
                                               //NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::NOTIFY
                                              );

    pFoodCharacteristic->setValue("Fries");
    pFoodCharacteristic->setCallbacks(&chrCallbacks);

    /** Note a 0x2902 descriptor MUST NOT be created as NimBLE will create one automatically
     *  if notification or indication properties are assigned to a characteristic.
     */

    /** Custom descriptor: Arguments are UUID, Properties, max length in bytes of the value */
    NimBLEDescriptor* pC01Ddsc = pFoodCharacteristic->createDescriptor(
                                               "C01D",
                                               NIMBLE_PROPERTY::READ, 
                                               //NIMBLE_PROPERTY::WRITE|
                                               //NIMBLE_PROPERTY::WRITE_ENC, // only allow writing if paired / encrypted
                                               20
                                              );
    pC01Ddsc->setValue("Send it back!");
    pC01Ddsc->setCallbacks(&dscCallbacks);

    /** Start the services when finished creating all Characteristics and Descriptors */  
    pDeadService->start();
    pBaadService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pDeadService->getUUID());
    pAdvertising->addServiceUUID(pBaadService->getUUID());
    /** If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");
    return true;
}

/**
 * Sends string of response data on bluetooth upon receiving instructions
 * 
 * @param[in] tosend is the string of data to send on bluetooth
 * @return true if data is sent. false if no data is sent.
 */
bool ESP_BT::send(String tosend)
{
    long len = tosend.length();
    tosend = "%S%" + String(len) + "," + tosend + "%S%\r\n";
    NimBLEService* pSvc = pServer->getServiceByUUID("DEAD");
    if(pSvc)
    {
        NimBLECharacteristic* pChr = pSvc->getCharacteristic("BEEF");
        if(pChr)
        {
            Serial.println("we got into the send function");
            Serial.println("the message is:" + tosend);
            // NOTE: this will probably cause problems on the phone side since
            // the data isn't changing (we'll see it when we cross that bridge)
            pChr->setValue((uint8_t *)tosend.c_str(), tosend.length()+1);
            return true;
        }
    }
    return false;
}

bool ESP_BT::send_notification(String tosend)
{
    long len = tosend.length();
    tosend = "%S%" + String(len) + "," + tosend + "%S%\n\r";
    NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
    if(pSvc)
    {
        NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
        if(pChr)
        {
            // NOTE: this will probably cause problems on the phone side since
            // the data isn't changing (we'll see it when we cross that bridge)
            pChr->setValue((uint8_t *)tosend.c_str(), tosend.length()+1);
            pChr->notify(true);
            return true;
        }
    }
    return false;
}

/**
 * @brief This function reads and returns a serially transmitted message from
 * the controller (phone application). It is built with generalizability in mind
 * and records everything in <>.
 *
 * @return String 
 * 
 * NOTE: removed in new iteration
 */
// String ESP_BT::bt_read() 
// {
//     // Read the message of a connected device and check if it conforms to the
//     // set conventions
//     log_d("The device connection status is: %d", isConnected);
//     if(isConnected) 
//     {
//         int32_t size = 0;
//         char temp = '\0';
//         String BTread = "";
//         temp = Serial2.read();
//         if(temp == '<')
//         {
//             while(temp != '>' && size <= 90) // ID size will be fixed, needs to be addressed
//             {
//                 BTread += temp;
//                 temp = Serial2.read();
//                 size++;
//             }
//             if(size >= 90)
//             {
//                 log_e("Credentials exceed set limit");
//                 return "";
//             }
//             BTread += temp;
//         }
//         else // we got some other message in AT format
//         {
//             BTread = temp + Serial2.readString();
//             log_d("%s", BTread.c_str());
//             return BTread;
//         }
//         log_d("%s", BTread.c_str());
//         return BTread;
//     }

//     /**
//      * TODO: add the custom timed read and read string functions
//      */
//     else
//     {
//         // Read message without checking for message conventions
//         String BTread = Serial2.readString();
//         log_d("%s", BTread.c_str());
//         return BTread;
//     }
// }

/**
 * @brief This is a simple diagnostic function for debugging purposes.
 */
void ESP_BT::display(String ID, String Username, String Password)
{
    Serial.print("display() -> bluetooth.cpp -> ID: " + ID + "\n");
    Serial.print("display() -> bluetooth.cpp -> Username: " + Username +  "\n");
    Serial.print("display() -> bluetooth.cpp -> Password: " + Password + "\n");
}

/**
 * @brief This is a wrapper function which checks for incoming bluetooth
 * messages and processes them accordingly. It first checks if the device has
 * bonded to another device by checking AT responses and then recieves messages
 * from the central device
 *
 * @return String message
 */
bool ESP_BT::check_bluetooth()
{
    if(!this->commandInQueue && this->commandComplete)
    {
        NimBLEService* pSvc = pServer->getServiceByUUID("DEAD");
        if(pSvc)
        {
            NimBLECharacteristic* pChr = pSvc->getCharacteristic("BEEF");
            if(pChr)
            {
                delay(10);
                //String toreceive = pChr->getValue().c_str();
                String toreceive = this->bt_msg;
                Serial.println("we got into the send function");
                Serial.println("the message received is:" + toreceive);
                String tosend = command_bt(toreceive);
                // NOTE: this will probably cause problems on the phone side since
                // the data isn't changing (we'll see it when we cross that bridge)
                pChr->setValue((uint8_t *)tosend.c_str(), tosend.length()+1);
                this->commandInQueue = false;
                this->commandComplete = false;
                return true;
            }
        }
    }
    return false;
}

ESP_BT bt;
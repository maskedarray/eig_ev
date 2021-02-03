
#include <WiFi.h>

const char* ssid     = "h7BLH=U5f+qJCeG4";
const char* password = "&_nJ}<pj&4w@3F}F";

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(5, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

int value = 0;

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        String cl = client.readStringUntil('\n');             // read a byte, then
        Serial.println(cl);                    // print it out the serial monitor
        if(cl == "client1711"){
          client.print("BSS9014\n");
          client.stop();
        }
      }
    }
    // close the connection:
    Serial.println("Client Disconnected.");
  }
}
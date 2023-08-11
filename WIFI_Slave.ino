/*
 *  This sketch sends a message to a TCP server
 *
 */

#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;
WiFiClient client;
const char * host = "192.168.4.1"; 
const uint16_t port = 80;
void connectMaster() {
  WiFiMulti.addAP("ESP32-WiFi");
  while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
  }
  Serial.println("Erişim noktası eklendi");
  if (!client.connect(host, port)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 1 seconds before retrying...");
        delay(1000);
        return;
  }
  Serial.println("Hosta bağlanıldı");
}
void setup()
{
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);

    // We start by connecting to a WiFi network
    

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");
    connectMaster();
    
}


void loop()
{
  if (client.connected()) {
    client.write('T');
    delay(50);
  }
  else {
    connectMaster();
  }
/*
  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 1000)
  {
    maxloops++;
    delay(1); //delay 1 msec
  }
  if (client.available() > 0)
  {
    //read back one line from the server
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  else
  {
    Serial.println("client.available() timed out ");
  }

    Serial.println("Closing connection.");
    client.stop();

    Serial.println("Waiting 5 seconds before restarting...");
    delay(5000); */
}

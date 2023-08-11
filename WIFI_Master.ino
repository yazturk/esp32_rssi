/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <esp_wifi.h>

// Set these to your desired credentials.
const char *ssid = "ESP32-WiFi";
const char *password = "sifre12345";

WiFiServer server(80);

WiFiClient slave;

int rssi;
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {

    // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
    //if (type != WIFI_PKT_MGMT)
     //   return;

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    
    rssi = ppkt->rx_ctrl.rssi;
}

void findSlave() {
  //Bir istemci bulunana kadar döngüye al
  while (1) {
    slave = server.available();
    if (slave) {
      Serial.println("BASLA");
      break;
    }
    else {
      Serial.println("Mesaj: Slave bulunamadi.");
      delay(1000);
    }
  }
}
void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid)) {
    Serial.println("Erişim noktası oluşturulamadı. 1s sonra tekrar denenecek.");
    delay(1000);
    ESP.restart();
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  //Slave bul
  findSlave();
}

void loop() {
  int timeout = 5000;
  char c;
  if (slave) {              
    while (slave.connected()) {            // loop while the client's connected
      if (slave.available()) {             // if there's bytes to read from the client,
        c = slave.read();
        if (c == 'T') {       
            Serial.println(rssi);
        }
        timeout = 5000;
      }
      else {
        delay(1);
        timeout -=1;
      }
      if (timeout == 0) {
        break;
      }
    }
    
    // close the connection:
    slave.stop();
    Serial.println("Mesaj: Slave baglantisi kesildi.");
    findSlave();
  }
  else {
    Serial.println("Mesaj: Slave yok");
    findSlave();
    }
}

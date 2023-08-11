/**
   ESPNOW - Basic communication - Master
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Master module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Master >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()


#define CHANNEL 1
#define DELETEBEFOREPAIR 0

typedef struct {
  char mesaj[100];
} data_t;
data_t incoming_data;

// global slave değişkeni
esp_now_peer_info_t slave;
int connected = 0;
int rssi;
int slave_number;
// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// Scan for slaves in AP mode
int ScanForSlave() {
  String SSID;
  //Makine adresi, String olarak
  String BSSIDstr;
  //Makine adresi, int dizisi
  int mac[6];
  int i;

  int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, CHANNEL); // Scan only on one channel
  // reset on each scan
  bool slaveFound = 0;
  memset(&slave, 0, sizeof(slave));

  Serial.println("");
  if (scanResults == 0) {
    
  } 
  else {
    Serial.print("Mesaj: "); Serial.print(scanResults); Serial.println("cihaz bulundu.");
    for (i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      SSID = WiFi.SSID(i);
      BSSIDstr = WiFi.BSSIDstr(i);


      delay(10);
      // Check if the current device starts with `Slave`
      if (SSID.startsWith("Slave")) {
        // SSID of interest
        Serial.println("Mesaj: Slave bulundu");
        //Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
        // Get BSSID => Mac Address of the Slave
        
        //Makine adresi stringten int dizisine dönüştürülür
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            slave.peer_addr[ii] = (uint8_t) mac[ii];
          }
        }

        slave.channel = CHANNEL; // pick a channel
        slave.encrypt = 0; // no encryption

        slaveFound += 1;
        // we are planning to have only one slave in this example;
        // Hence, break after we find one, to be a bit efficient
        break;
      }
    }
  }

  if (slaveFound) {
    return i;
  } else {
    // Tarama sonuçlarını temizle
    WiFi.scanDelete();
    Serial.println("Mesaj: Slave bulunamadi, tekrar denenecek");
    return -1;
  }

  
}

// Slave-master eşleşmesini kontrol eder
bool manageSlave() {
  if (slave.channel == CHANNEL) {
    if (DELETEBEFOREPAIR) {
      deletePeer();
    }

    Serial.print("Mesaj: Slave durumu: ");
    // eşleşme var mı?
    bool exists = esp_now_is_peer_exist(slave.peer_addr);
    if ( exists) {
      // Eşleşme varsa bir şey yapmadan çık
      Serial.println("Zaten eslesmis.");
      return true;
    } else {
      // Eşleşme yoksa eşleştirmeye çalış
      esp_err_t addStatus = esp_now_add_peer(&slave);
      if (addStatus == ESP_OK) {
        // Eşleşme başarılı
        Serial.println("Basariyla eslesti");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW baslamamis.");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Yanlis arguman.");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        // Sadece 1 peer olacağı için bunun olmaması beklenir
        Serial.println("Peer listesi dolu.");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Hafiza yetersiz.");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer mevcut");
        return true;
      } else {
        Serial.println("Bilinmeyen bir hata oldu.");
        return false;
      }
    }
  } else {
    // Slave'in kanalı farklı ise
    Serial.println("Mesaj: Slave'in kanali farkli");
    return false;
  }
}

void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
  Serial.print("Mesaj: Slave'in silinme durumu: ");
  if (delStatus == ESP_OK) {
    Serial.println("Basarili");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW baslamamis.");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Gecersiz arguman.");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer bulunamadi.");
  } else {
    Serial.println("Bilinmeyen hata.");
  }
}

uint8_t data = 255;

void sendData() {

  esp_err_t result = esp_now_send(slave.peer_addr, &data, sizeof(data));
  Serial.print("Mesaj: Slave'e veri gonderme: ");
  if (result == ESP_OK) {
    Serial.println("Basarili");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW baslamamis.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Gecersiz arguman");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Dahili hata.");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("Hafiza dolu.");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer bulunamadi.");
  } else {
    Serial.println("Bilinmeyen hata");
  }
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Mesaj: Slave'e mesaj ulasti.");
    //connected = 100;
  }
  else {
    
  }
}


void setup() {
  Serial.begin(115200);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.println("ESPNow/Basic/Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
}


void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  
  //if (memcmp(slave.peer_addr, mac_addr, 6) == 0) {
  
    //memcpy(&incoming_data, data, sizeof(incoming_data));
    if (connected == 0){
      Serial.println("BASLA");
    }
    connected = 100;
    Serial.println(rssi);
  //}
}
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {

    // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
    if (type != WIFI_PKT_MGMT)
        return;

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    
    rssi = ppkt->rx_ctrl.rssi;
  
}
void loop() {
  // Bağlantı yok ise
  if (connected == 0) {
    // In the loop we scan for slave
    slave_number = ScanForSlave();
    // If Slave is found, it would be populate in `slave` variable
    // We will check if `slave` is defined and then we proceed further
    if (slave.channel == CHANNEL) { // check if slave channel is defined
      // `slave` is defined
      // Add slave as peer if it has not been added already
      bool isPaired = manageSlave();
      if (isPaired) {
        // pair success or already paired
        // Send data to device
        sendData();
      } else {
        // slave pair failed
        Serial.println("Mesaj: Slave ile eslesme basarisiz");
      }
    }
    else {
    // No slave found to process
    }
    delay(100);
  }
  // Bağlantı var ise
  else {
    //25ms'de bir bağlantı zayıflar
    delay(25);
    connected -= 1;
  }
  // wait for 3seconds to run the logic again
  //delay(3000);
}

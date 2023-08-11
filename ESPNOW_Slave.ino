/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

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

#define CHANNEL 1

const char *SSID = "Slave_1";
// Master'ın makine adresi
uint8_t master_mac[6];
int connected = 0;

typedef struct {
  char mesaj[50];
} data_t;
data_t data;

esp_now_peer_info_t peerInfo;

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

// config AP SSID
void configDeviceAP() {
  bool result = WiFi.softAP(SSID, "", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}


int sendData() {

  esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t *) &data, sizeof(data));
  Serial.print("Mesaj: Master'a veri gonderme: ");
  if (result == ESP_OK) {
    Serial.println("Basarili");
    return 0;
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
  } else if (result == ESP_ERR_ESPNOW_IF) {
    Serial.println("Interface hatası");
  } else {
    Serial.println("Bilinmeyen hata");
  }
  return -1;
}

// Slave-master eşleşmesini kontrol eder
bool manageSlave() {
  if (peerInfo.channel == CHANNEL) {
    deletePeer();


    Serial.print("Mesaj: Peer durumu: ");
    // eşleşme var mı?
    bool exists = esp_now_is_peer_exist(peerInfo.peer_addr);
    if ( exists) {
      // Eşleşme varsa bir şey yapmadan çık
      Serial.println("Zaten eslesmis.");
      return true;
    } else {
      // Eşleşme yoksa eşleştirmeye çalış
      esp_err_t addStatus = esp_now_add_peer(&peerInfo);
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
    // Peer kanalı farklı ise
    Serial.println("Mesaj: Peer kanali farkli");
    return false;
  }
}

void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(peerInfo.peer_addr);
  Serial.print("Mesaj: Peer silinme durumu: ");
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

void setup() {
  strcpy(data.mesaj, "TEST MESAJI");
  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP_STA);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSend);

}

void OnDataSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Ulaştı");

  }
  else {
    Serial.println("Mesaj ulaşmadı.");
  }
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  // Bağlı master yoksa teklifler değerlendirilir
  if (connected == 0) {
    for (int i=0; i<6; i++) {
      peerInfo.peer_addr[i] = mac_addr[i];

      Serial.println(peerInfo.peer_addr[i]);
    }
    Serial.println("Master'in makine adresi alindi.");
    connected = 50;
    //memcpy(peerInfo.peer_addr, mac_addr, 6);
    peerInfo.channel = 1;  
    peerInfo.encrypt = false;
  
    // Peer ekle        
    manageSlave();
  }
  
}


void loop() {
  if (connected != 0) {
    int result = sendData();
    if (result == -1) {
      connected -= 1;
      //Serial.println(connected);
    }
    else {
      connected = 50;
    }

    delay(50);
  }
  else {
    // Bağlantı kopmuşsa tekrar bağlanmaya çalış
    Serial.println("Master aranıyor");
    delay(3000);
  }
}

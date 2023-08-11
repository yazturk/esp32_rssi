/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara

 Arduino içinde sunulan örneklerden faydalanılmıştır
 RSSI okuma için kaynak: https://community.appinventor.mit.edu/t/arduino-distance-meassuring-through-bluetooth-low-energy-signal-strength-rssi-project/24830
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// Bu servisi sunan cihaza bağlanılacak
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// Bağlanılan cihazla bu karakteristik üzerinden veri alışverişi sağlanacak
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

// Tarama tamamlandığında true yapılarak bağlanılmaya çalışılacak
static boolean doConnect = false;

static boolean connected = false;
static boolean doScan = false;

// BLE protokolünde karakteristikler alınıp verilecek veri kümelerini ifade eder
static BLERemoteCharacteristic* pRemoteCharacteristic;

// Bağlanılacak Slave
static BLEAdvertisedDevice* myDevice;

// Slave'in makine adresi
esp_bd_addr_t remote_addr;

// Bildirim geldiğinde çalışcak fonksiyon
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    // Bildirim alındığında RSSI okumayı tetikler
    esp_ble_gap_read_rssi(remote_addr); 
}

class MyClientCallback : public BLEClientCallbacks {
  // Bir cihazla bağlantı sağlandığında
  void onConnect(BLEClient* pclient) {
    // RSSI okuyan fonksiyon register edilir
    BLEDevice::setCustomGapHandler(rssi_handler);
  }
  // Bir bağlantı sonlandığında
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};
// GAP event'larını kontrol eder
void rssi_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  int rssi;
  // RSSI okuma tetiklenmişse oku ve porta yaz
  if (event == ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT) {
    rssi = param->read_rssi_cmpl.rssi;
    Serial.println(rssi);
  }
}

// Slave'e bağlan
bool connectToServer() {
    // Cihazın makine adresi
    BLEAddress addr = myDevice->getAddress();
    // Makine adresi 6 byte halinde ilgili değişkene yazılır
    memcpy(remote_addr, addr.getNative(), 6);
    Serial.print("Forming a connection to ");
    Serial.println(addr.toString().c_str());
    // Bir istemci oluştur
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    // onConnect ve onDisconnect callback'leri register edilir
    pClient->setClientCallbacks(new MyClientCallback());

    // Slave'in server'ına bağlan.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // İlgilendiğimiz servisin referansı alınır
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    // Refrans null ise bağlantı sonlanır
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Servisten karakteristik alınır
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    // Null alınmışsa bağlantı sonlanır
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Karakteristik değeri okunur
    // Bizim için gereksiz 
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }
    
    // Bildirim özelliği açık ise (ki açık olmalı) callback register edilir
    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);
    
    // Buraya kadar gelinmişse bağlantı sağlanmıştır
    connected = true;
    return true;
}
// Çevre server'ları tara ve ilgilendiğimiz servisi sunan cihaza bağlan
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 // Kendini tanıtan bütün server'lar için bu fonksiyon çağrılır
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Doğru servise sahip ise
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      // Aranan cihaz bulunduğuna göre tarama durdurulur
      BLEDevice::getScan()->stop();
      // Slave cihaz belirlenir
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      // bu değişkenler loop içerisinde denetlenerek gerekli eylemler yapılır
      doConnect = true;
      // Bağlantı varken tarama yapılmayacağı için (bkz loop)
      // şimdiden ayarlanabilir
      doScan = true;

    } 
  } 
}; 


void setup() {
  // Seri port açılır
  Serial.begin(115200);
  
  // BLE istemci olarak açılır
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  // Cihaz bulunduğunda çalışacak callback'leri ayarla
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  // 5 saniye boyunca tara
  pBLEScan->start(5, false);
} 


void loop() {

  // Uygun bir cihaz bulunmuş ve bağlanılması gerekiyorsa
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
      Serial.println("BASLA");
    } 
    else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // Bağlantı varsa
  if (connected) {
    // Verileri bildirim geldiğinde okuyacağız (notifyCallback)
    // Burada bir şey yapmaya gerek yok
  }
  // Bağlı değilse ve tarama gerekiyorsa
  else if(doScan){
    BLEDevice::getScan()->start(0);  
  }
  // 1 saniye bekle
  delay(1000); 
} 

//Kaynaklar: 
// https://community.appinventor.mit.edu/t/arduino-distance-meassuring-through-bluetooth-classic-signal-strength-rssi-project/21175
// https://deepbluembedded.com/esp32-bluetooth-classic-with-arduino-complete-guide/


#include <BluetoothSerial.h>
#include "esp_gap_bt_api.h"

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

//Bu cihazın adı
const char *myName = "ESP32-BT-Master";

// Slave'in mac adresi
uint8_t mac[6];

int rssi;
// gap event'larını kontrol eder ve rssi değişkenini günceller
// GAP: Generic Access Profile
void gap_callback (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
  // loop() icinde esp_bt_gap_read_rssi_delta çağrıldığında bu event gerçekleşir
  if (event == ESP_BT_GAP_READ_RSSI_DELTA_EVT)
  {
    rssi = param->read_rssi_delta.rssi_delta; 
    // Sinyal gücünü porta yazdır
    Serial.println(rssi);
  }
}


// Başlangıçta bir defa çalıştırılır
void setup() {
  // Bilgisayarın seri portuna bağlan
  Serial.begin(115200);

  // bluetooth portu master modunda açılır
  SerialBT.begin(myName, true); //true=master
  Serial.printf("Mesaj: Cihaz \"%s\" master modunda basladi\r\n", myName);
  

}

// Sürekli tekrarlanır
void loop() {
  char c;
  // Bağlantı var ise okuma yap
  if (SerialBT.connected()) {
    // Okunacak veri var ise
    if (SerialBT.available()) {
      // Bir karakter oku
      c = SerialBT.read();
      // RSSI okuma event'ı oluştur
      esp_bt_gap_read_rssi_delta (mac);
      //RSSI callback fonksiyonunun içinde okunacak ve porta yazılacak
      //Buradan yazdırılırsa güncel olmayan değer yazılabilir
      //Serial.println(rssi);
    }
  }
  //Bağlı değilse
  else
  {
    // Cihazları tara
    BTScanResults *results = SerialBT.discover(10000);
    // Bulunan cihaz sayısı
    int count = results->getCount();
    // Bulunan sonuçları yazdır
    if (results)
      results->dump(&Serial);
    else
      Serial.println("Error on BT Scan, no result!");

    // Sonuçları incele
    for (int i=0; i<count; i++) {
      // i. cihaz
      BTAdvertisedDevice *device = results->getDevice(i);
      // Cihazın ismi Slave ile başlıyorsa bağlanılacak
      String name = device->getName().c_str();
      if (name.startsWith("Slave")) {
        Serial.print("Mesaj: Slave bulundu: ");
        Serial.println(name);
        // Cihazın makine adresini mac değişkenine kopyala
        BTAddress addr = device->getAddress();
        memcpy(mac, addr.getNative(), 6);
        // İsim veya makine adresi kullanılarak bağlanılabilir
        SerialBT.connect(name);
        delay(500);
        // gap event'ları için callback fonksiyonun register edilmesi
        // Bu, bağlantı kurulmadan önce yapılırsa bağlantının kurulmasını
        // ve cihazların görünmesini engelliyor ?
        esp_bt_gap_register_callback(gap_callback);
        delay(500);
        // Sinyal okuma programına okumanın başladığını bildirmek için
        Serial.println("BASLA");
        // Sadece tek cihaza bağlanılacağı için fonksiyon sonlandırılır
        return;
      }
    }
    // Buraya geldiğine göre sonuç bulunamadı
    Serial.println("Mesaj: Hic cihaz bulunamadi");
    // Tekrar tarama yapılması için bir müddet bekle
    delay(1000);
      
  }

    
}


  

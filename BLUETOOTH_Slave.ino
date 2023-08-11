//Kaynak: https://deepbluembedded.com/esp32-bluetooth-classic-with-arduino-complete-guide/

#include "BluetoothSerial.h"
 
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

// Cihazın kendi adı
const char *device_name = "Slave_1";
//Bluetooth bağlantısı
BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  // Bluetooth'u başlat
  SerialBT.begin(device_name);
  Serial.println("Başladı");
  delay(1000);

}

void loop() {
  // 50ms'de bir mesaj gönder
  // T test anlamında
     SerialBT.write('T');
     delay(50);
}

/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara

Arduino IDE içindeki örneklerden faydalanıldı
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Orijinal UUID oluşturmak için
// https://www.uuidgenerator.net/

// Master cihazla aynı olmalı
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Veri iletişimi karakteristik üzerinden yapılır
BLECharacteristic *pCharacteristic;


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  // BLE başlat
  BLEDevice::init("Long name works now");
  // Server oluştur
  BLEServer *pServer = BLEDevice::createServer();
  // Servis oluştur
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Karakteristik oluştur
  // Okuma, yazma ve bildirim özelliklerini ekle
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  // Karakteristiğin değerini ayarla
  pCharacteristic->setValue("Hello World says Neil");
  // servisi başlat
  pService->start();

  // Servise tanıtım ekle
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true); 
  /* Bunlar olmadan da çalıştı
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12); */
  
  // Tanıtıma başla
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

}

void loop() {
  // Karakteristiğe değer atayıp bildirim gönder
  pCharacteristic->setValue((uint8_t *) "TEST",4);
  pCharacteristic->notify();
  // 50ms bekle
  delay(50);
}
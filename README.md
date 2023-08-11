# esp32_rssi
ESP32 cihazlarında farklı protokollerde gönderilen sinyallerin RSSI değerlerini ölçme ve grafik haline getirme


1 - Slave cihazlar sinyal gönderir, Master'lar sinyal gücünü ölçerek seri porttan bilgisayara iletir.

2 - Komut satırı programı basit tutulmuştur. Sadece belirtilen sayı kadar örnek okur ve her satıra bir veri olacak şekilde dosyaya kaydeder.

Şu şekilde kullanılabilir:

    VeriOku_CLI.py -p <protokol_ismi> -n <ornek sayisi>
    
Seri portu okuyan başka program varsa (örneğin Arduino) çalışmaz.

3 - GUI programı 
![GUI_Ekran_Görüntüsü](https://github.com/yazturk/esp32_rssi/assets/22481884/2103dcbf-129b-4351-8469-ec1199d33e6b)

Farklı mesafelerden okunan verileri CSV formatında kaydeder. Aynı zamanda iki grafik oluşturur. 

Büyük oranda hatasız çalışır. Yalnız iptal butonuna basıldıktan sonra bir verinin daha okunması beklenir. Okunacak veri yoksa beklemede kalır.

Arayüz programının çalışması için de seri portu okuyan başka program varsa kapatılmalıdır.

CSV dosyasında ilk sütun mesafelerden oluşur. Diğer sütunlar o mesafeye karşılık gelen RSSI değerleridir.

Sonlandır'a tıklandığında aynı isimli dosya varsa silmek yerine ekleme yapar. Bu durumda örnek sayısının aynı olmasına dikkat edilmelidir.

4 - Grafikler
Yapılan deneylerde aynı mesafeden 500 veri alınmıştır

Örnek:

![BLE_boxplot](https://github.com/yazturk/esp32_rssi/assets/22481884/12c5ecf9-c86d-442b-93e8-b473e80dcca2)


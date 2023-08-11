# esp32_rssi
ESP32 cihazlarında farklı protokollerde gönderilen sinyallerin RSSI değerlerini ölçme ve grafik haline getirme


1 - Slave cihazlar sinyal gönderir, Master'lar sinyal gücünü ölçerek seri 
porttan bilgisayara iletir.

2 - Komut satırı programı basit tutulmuştur. Sadece belirtilen sayı kadar örnek
okur ve her satıra bir veri olacak şekilde dosyaya kaydeder
Şu şekilde kullanılabilir:
    GrafikCiz_CLI.py -p <protokol_ismi> -n <ornek sayisi>
    
Seri portu okuyan başka program varsa (örneğin Arduino) çalışmaz.

3 - GUI programı farklı mesafelerden okunan verileri CSV formatında kaydeder
Aynı zamanda iki grafik oluşturur. Büyük oranda hatasız çalışır. Yalnız iptal 
butonuna basıldıktan sonra bir verinin daha okunması bekleniyor. Okunacak veri 
yoksa beklemede kalır.

Arayüz programının çalışması için de seri portu okuyan başka program
varsa (Arduino gibi) kapatılmalıdır.

CSV dosyasında ilk sütun mesafelerden oluşur. Diğer sütunlar o mesafeye
karşılık gelen RSSI değerleridir.

Sonlandır'a tıklandığında aynı isimli dosya varsa silmek yerine ekleme 
yapar. Bu durumda örnek sayısının aynı olmasına dikkat edilmelidir.

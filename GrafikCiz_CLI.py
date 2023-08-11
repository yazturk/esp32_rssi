#!/usr/bin/python

import serial
from serial.tools import list_ports
import sys

# Portun veri hızı, bağlı cihazlarla aynı olmalıdır
baudrate = 115200

# main fonksiyonu dosyanın en sonundaki komutla başlatılır
def main():
    # Komut satırından girilen argümanları ayıkla
    sonuc = arguman_kontrol();
    # sonuc 0 ise hatalı veya eksik argüman girilmiştir
    # program sonlandırılır
    if sonuc == 0:
        print("Kullanım: ./get_rssi.py -p <protokol> -n <örnek sayısı>")
        return
    # Hata yoksa protokol ismi ve okunacak örnek sayısı alınır
    else:
        protokol, ornek_s = sonuc
    
    # Mevcut portlardan birisi seçilir ve seçilen port açılır
    ser = port_ac()
    # Sonuç sıfır dönmüşse hata vardır, program sonlandırılır
    if ser == 0:
        print("Uygun port bulunamadı");
        print("Cihazın doğru porta bağlı olduğunu ve porta erişim yetkinizin olduğunu kontrol edin")
        return
    
    # Öncelikle portta birikmiş eski veriler silinir
    ser.flush()
    # Sonuçların kaydedileceği dosya yazma modunda açılır
    dosya = open(protokol + ".txt", "w")
    i = 1
    while (i<=ornek_s):
        # Bir satır okunur, string'e çevrilip son iki karakter atılır
        line = ser.read_until(b'\r\n').decode()[:-2]
        # Okunan satır int'e çevrilmeye çalışılır
        # Başarılı olursa tekrar string'e çevrilip dosyaya yazılır
        try:
            rssi = int(line)
            dosya.write(str(rssi) + "\n")
            # Her 10 satırda bir geri bildirim verilir
            if i % 10 == 0:
                print(i, "veri okundu")
            i += 1
        except ValueError:
            # int'e çevirmede hata olmuşsa bu satır yazdırılır
            # okuma devam eder
            print(line)
        except:
            # Başka bir hata olmuşsa program sonlandırılır
            dosya.close()
            ser.close()
            raise
    # Okuma bitti, dosya ve seri port kapatılır
    dosya.close()
    ser.close()

# Komutsatırının argümanlarını denetler
def arguman_kontrol():
    # 4 argüman girilmesi beklenir
    # Program ismiyle beraber 5 argüman
    if len(sys.argv) != 5:
        print("Yanlış sayıda argüman")
        print(sys.argv)
        return 0
    
    # 1. argüman -p
    if sys.argv[1] != "-p":
        print("İlk argüman -p olmalı")
        print("Girilen:", sys.argv[1])
        return 0
    # 2. argüman protokol ismi
    
    # 3. argüman -n
    if sys.argv[3] != "-n":
        print("3. argüman -n olmalı")
        print("Girilen:", sys.argv[3])
        return 0
    
    # Son argüman örnek sayısı, bu yüzden int'e çevrilir
    try:
        o_s_ = int(sys.argv[4])
    except:
        print("Son argüman bir tam sayı olmalı")
        print("Girilen:", sys.argv[4])
        return 0
    # Protokol ismini ve örnek sayısını döndür
    return (sys.argv[2], o_s_)

# Mevcut portlardan birini kullanıcıya seçtirir
def port_ac():
    # Port listesini al
    portlar = list_ports.comports()
    if len(portlar) == 0:
        print("Seri port bulunamadı")
        return 0
    # Yalnızca 1 port varsa kullanıcıya sormaya gerek yok
    if len(portlar) == 1:
        i = 1

    else:
        # Bulunan portları başında numaralarla beraber yazdırır
        for i,port in enumerate(portlar, start=1):
            print("[{}] {}".format(i,port.device))

        print("Seçtiğiniz portun başındaki numarayı girip entere basın")
        i = input()
        i = int(i)
    # Seçilen port alınır
    port = portlar[i-1].device
    print("Seçilen port:",port)
    # Portla seri bağlantı kurulur
    try:
        ser = serial.Serial(port, baudrate)
    except:
        print("Porta bağlanılamadı")
        return 0
    # Başarılı olduysa seri bağlantı döner
    return ser

# Fonksiyonların tanımlanması tamam, program başlar
main()

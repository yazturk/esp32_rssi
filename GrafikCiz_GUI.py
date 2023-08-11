#!/usr/bin/python
import tkinter as tk
import serial
from serial.tools import list_ports
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import threading

baudrate = 115200

# Uygulamanın 3 modu bulunmaktadır

# 1. Başlangıç modu: Daha önce kayıt edilmiş bir adım yoktur
#                    Sonlandır butonu hariç tüm öğeler aktiftir

# 2. Yeni adım modu: Yeni mesafe girilip başla butonuna tıklanılması beklenir.
#                    Sonlandır'a tıklanması halinde verileri dosyaya ve grafiğe 
#                    kaydedip başlangıç moduna döner

# 3. Veri okuma modu: Daha önce belirlenen sayıda verinin okunması beklenir.
#                    İptal'e tıklanması halinde sadece o adımda okunan veriler 
#                    kaybolur. 

# Seri porttan okunan veriler:
# Mesaj: ile başlıyorsa arayüze yazılır ama okuma devam eder
# Hata: ile başlıyorsa arayüze yazılır ve okuma sonlanır (o adımdaki veriler kaybolur)
# Diğer durumlarda int'e dönüştürülür ve RSSI değeri olarak kaydedilir
# int'e dönüşmüyorsa yok sayılır ve okumaya devam edilir


# tkinter'ın Tk sınıfı miras alınır
class App(tk.Tk):    
    # __init__ metodunda önce arayüz oluşturulur 
    # son satırda ise portlari_al metoduyla kullanılabilir portlar alınır
    def __init__(self):
        #Tk sınıfının init metodu
        super().__init__()
        
        # Bu listenin her bir öğesi bir mesafeden alınan verileri tutacak
        # Verilerin başında cm cinsinden mesafe yazacak
        self.veriler = []
        
        # Arayüz yapılandırması
        self.title('Seri Porttan Sinyal Gücü Okuma')
        frame1 = tk.Frame()
        frame1.pack(padx=10, pady=10)
        tk.Label(
            frame1, 
            text="Port seçin:"
            ).pack(side=tk.LEFT)
        self.port_sec = tk.StringVar()
        self.port_menusu = tk.OptionMenu(
            frame1,
            self.port_sec,
            "")
        self.port_menusu.pack(side=tk.LEFT)
        
        self.yenile_butonu = tk.Button(
            frame1,
            text='YENİLE',
            command=self.portlari_al)
        self.yenile_butonu.pack()
        
        frame2 = tk.Frame()
        frame2.pack(padx=10, pady=10)
        
        tk.Label(
            frame2, 
            text="Deney adı (örneğin bluetooth1)"
            ).pack(side=tk.LEFT)
        self.deney_adi_alani = tk.Entry(frame2)
        self.deney_adi_alani.pack(side=tk.LEFT)
        
        tk.Label(
            frame2, 
            text="\tAynı mesafeden örnek veri sayısı: "
            ).pack(side=tk.LEFT)
        self.ornek_sayisi_sec = tk.Spinbox(frame2)
        self.ornek_sayisi_sec.pack(side=tk.LEFT)
        
        tk.Label(
            frame2, 
            text="\tMesafe (cm): "
            ).pack(side=tk.LEFT)

        self.mesafe_sec = tk.Spinbox(frame2)
        self.mesafe_sec.pack(side=tk.LEFT)

        frame3 = tk.Frame()
        frame3.pack(padx=10, pady=10)
        
        #Uygulamada mesajlar ve hatalar bu etiket üzerinden gösterilecek
        self.mesaj_etiketi = tk.Label(
            frame3, 
            text="Yukarıdaki değerleri girdikten sonra Başla butonuna basın")
        self.mesaj_etiketi.pack()
        self.oku_butonu = tk.Button(
            frame3, 
            text="Başla", 
            command=self.basla)
        self.oku_butonu.pack(side=tk.LEFT)

        self.sonlandir_butonu = tk.Button(
            frame3, 
            text="Sonlandır ve Dosyaya Kaydet",
            command=self.sonlandir,
            state=tk.DISABLED)
        self.sonlandir_butonu.pack()
        
        # Açılışta uygun portlar alınır
        self.portlari_al()
    
    # Kullanılabilir port listesi alınarak port_menusu ögesine yüklenir
    # Başlangıçta ve YENİLE butonuna tıklayınca çalışır
    def portlari_al(self):
        print("Portlar alınıyor")
        portlar = list_ports.comports()

        if len(portlar) == 0:
            portlar = ["Kullanılabilir port bulunamadı"]
        else:
            portlar = [p.device for p in portlar]
        self.port_sec.set(portlar[0])
        menu = self.port_menusu["menu"]
        menu.delete(0, "end")
        for port in portlar:
            menu.add_command(
                label=port,
                command=lambda v=port: self.port_sec.set(v))
    
    
    # Başla butonuna tıklanması halinde çalışır
    def basla(self):
        # Butonların devredışı bırakılması için
        self.veri_okuma_modu()
        # Arayüz öğeleri güncellenir
        self.update_idletasks()
        
        # Arayüzde girilen değerler ilgili değişkenlere yüklenir
        self.degiskenleri_al()
        
        # Okunan değişkenlerde sorun olması halinde veri okuma iptal edilir
        if self.iptal:
            self.iptal = False
            return
        
        # Bu adımda okunacak veriler, en başa mesafe yazılır
        # Veri okuma sonunda bu değişken 'veriler' listesine eklenecektir
        self.guncel_veriler = np.zeros(self.ornek_sayisi + 1, dtype=np.int)
        self.guncel_veriler[0] = self.mesafe
        
        self.mesaj_yaz("Veri okuma başlıyor")
        
        # Portun açılması
        # port = seçilen seri portun ismi
        # ser  = seri bağlantı
        try:
            self.ser = serial.Serial(self.port, baudrate)
            print("Port {} açıldı".format(self.port))
        except Exception as e:
            print(e)
            self.hata_yaz("Port açılamadı:{}".format(self.port))
            return
        
        # Bu değişken hata mesajını veya kaç veri okunduğu mesajını içerebilir
        self.okuma_durumu = ""
        
        #Arayüzün donmaması için okuma ayrı bir thread'de yapılır
        t = threading.Thread(target=self.okuyucu)
        t.start()
        
        # Oluşturulan thread sonlanan kadar beklenir
        self.after(100, self.thread_kontrol, t)  
    
    # Sonlandır butonuna tıklanması halinde çalışır
    def sonlandir(self):
        # Toplanan veriler DataFrame'e dönüştürülür
        df = pd.DataFrame(self.veriler)
        self.veriler = []
        print(df)
        
        # CSV dosyasına yazılır
        dosya = self.deney_adi
        if not dosya.endswith(".csv"):
            dosya += ".csv"
        df.to_csv(dosya, index=False, header=False, mode="a")
        
        # Grafik çizmek istemiyorsanız aşağıdaki satırı devre dışı bırakın
        self.grafik_ciz(dosya)
        
        # Uygulama ilklendirilir
        self.basla_modu()        
    
    # Butonların aktifleştirilmesi için
    def basla_modu(self):
        self.mesafe_sec.config(state=tk.NORMAL)
        # Daha önce kayıtlı veri yoksa Başlangıç Modu
        if len(self.veriler) == 0:
            self.port_menusu.config(state=tk.NORMAL)
            self.deney_adi_alani.config(state=tk.NORMAL)
            self.ornek_sayisi_sec.config(state=tk.NORMAL)
            self.yenile_butonu.config(state=tk.NORMAL)
            
            #Kaydedilecek veri olmadığı için sonlandır butonu çalışmaz
            self.sonlandir_butonu.config(state=tk.DISABLED)
            
            self.oku_butonu.config(
                text="Başla",
                command=self.basla)
        # Kayıtlı veri varsa Yeni Adım Modu
        else:
            self.sonlandir_butonu.config(state=tk.NORMAL)
            self.oku_butonu.config(
                text="Yeni Adıma Başla",
                command=self.basla)
        


        
    def grafik_ciz(self, dosya_adi):
        # Dosyadan verileri oku
        # Eski verilerin üzerine ekleme yapılmış olabileceği için 
        # dosyadan okunuyor
        df = pd.read_csv(dosya_adi, index_col=False, header=None)
        #print(df)
        
        # Mesafe verileri DataFrame'in ilk sütunundadır
        mesafeler  = df[0]
        
        # self.veriler değişkeniyle karıştırmayın
        # mesafe verilerinden oluşan ilk sütun silinir
        veriler = df.drop(0, axis=1)
        
        # Plot çizme
        ort = [np.mean(v) for v in veriler.values]
        
        plt.figure()
        plt.title("Mesafeye göre sinyal gücü", fontsize=16)
        plt.xlabel("Mesafe (cm)")
        plt.ylabel("Sinyal gücü (RSSI)")
        plt.plot(mesafeler, ort)
        
        # Dosyaya kaydetme
        dosya = self.deney_adi
        if dosya.endswith(".csv"):
            dosya = dosya[:-4]
        dosya += "_plot.png"
        plt.savefig(dosya)
        #plt.show()


        # Boxplot çizme
        plt.figure()
        plt.title("Mesafeye göre sinyal gücü kutu grafiği", fontsize=16)
        plt.xlabel("Mesafe (cm)")
        plt.ylabel("Sinyal gücü (RSSI)")
        plt.boxplot(
            veriler.transpose(),
            labels=mesafeler)
        plt.grid(linestyle="--")
        
        # Dosyaya kaydedilir
        dosya = self.deney_adi
        if dosya.endswith(".csv"):
            dosya = dosya[:-4]
        dosya += "_boxplot.png"
        plt.savefig(dosya)
        
        # Derhal göstermek için bu satır kullanılabilir
        # Grafik kapatılıncaya kadar arayüz donar
        #plt.show()

            
    # Veri okuma modu için arayüzdeki alanlar kilitlenir
    def veri_okuma_modu(self):
        self.port_menusu.config(state=tk.DISABLED)
        self.deney_adi_alani.config(state=tk.DISABLED)
        self.ornek_sayisi_sec.config(state=tk.DISABLED)
        self.mesafe_sec.config(state=tk.DISABLED)
        self.yenile_butonu.config(state=tk.DISABLED)
        self.sonlandir_butonu.config(state=tk.DISABLED)
        
        # Her ihitmale karşı iptal değişkeni ilklendirilir
        self.iptal = False
        
        # Başla butonu İptal butonuna dönüşür
        self.oku_butonu.config(
            text="İptal",
            command=self.iptal_et)
    
    # Arayüzdeki alanlardan alınan değerler değişkenlere aktarılır
    def degiskenleri_al(self):
        # Port ismi
        self.port = self.port_sec.get()
        
        # Uygun port yoksa başlangıç veya yeni adım moduna dönülür
        if self.port == "Kullanılabilir port bulunamadı":
            self.basla_modu()
            self.mesaj_yaz("Kullanılabilir port yok")
            self.iptal_et()
            return
        
        # Dosya ismi olarak kullanılacak deney adı
        self.deney_adi = self.deney_adi_alani.get().strip()
        if self.deney_adi == "":
            self.basla_modu()
            self.mesaj_yaz("Geçerli bir deney adı girin, dosya ismi olarak kullanılacak")
            self.iptal_et()
            return
        
        # Örnek sayısı
        o_s = self.ornek_sayisi_sec.get()
        try:
            # Tam sayıya dönüşmüyorsa iptal et
            self.ornek_sayisi = int(o_s)
            
            # 0'dan küçükse iptal et
            if self.ornek_sayisi <= 0:
                raise Exception
                
        except:
            self.basla_modu()
            self.mesaj_yaz("Geçerli örnek sayısı girin")
            self.iptal_et()
            return
        
        # Mesafe
        m = self.mesafe_sec.get()
        try:
            # Tam sayıya dönüşmüyorsa iptal et
            self.mesafe = int(m)
            
            # 0'dan küçükse iptal et
            if self.mesafe < 0:
                raise Exception
                
        except:
            self.mesafe_sec.config(state=tk.NORMAL)
            self.mesaj_yaz("Geçerli mesafe girin")
            self.iptal_et()
            return
        
        print("Seçilen port:", self.port)
        print("Deney:", self.deney_adi)
        print("Mesafe:", self.mesafe)
        print("Alınacak örnek sayısı:", self.ornek_sayisi)
    
    # Uygulama içi iptal sinyali gönderme
    def iptal_et(self):
        self.iptal=True
    
    # Etiketin üzerine mesajı yazar
    def mesaj_yaz(self, msj):
        self.mesaj_etiketi.config(text=msj, fg="black")
    
    # Mesajı kırmızı renkte yazar
    def hata_yaz(self, ht):
        self.mesaj_etiketi.config(text=ht, fg="red")
    
    # Arayüzün donmaması için bu fonksiyon ayrı bir thread olarak çalışacaktır
    # Bu metodun içinde arayüz öğelerinde değişiklik yapılmamalıdır
    # Bu yüzden mesaj_yaz ve hata_yaz metodları yerine okuma_durumu
    # değişkeni kullanılır.
    def okuyucu(self):
        # Seri portta daha önce birikmiş varsa silinir
        self.ser.flush()
        # Belirlenen örnek sayısı kadar veri okunacak
        i = 1
        while(i <= self.ornek_sayisi):
            print(i, ". veri okuma")
            if self.iptal:
                return
            # Arduino referanslarına göre Serial.println fonksiyonu
            # satırları '\r\n' ikilisiyle sonlandırmaktadır.
            line = self.ser.read_until(b'\r\n').decode()[:-2]
            
            # Okunan satır hata mesajı ise
            if line.startswith("Hata:"):
                self.okuma_durumu = line
                self.ser.close()
                return
            if line.startswith("Mesaj:"):
                self.okuma_durumu = line
            else:
                try:
                    rssi = int(line)
                    self.guncel_veriler[i] = rssi
                    print("{}/{} veri okundu".format(i, self.ornek_sayisi))
                    i += 1
                except:
                    #Veri okunamıyorsa yok say
                    print("Okunamayan veri:", line)
        
        # Yeterli sayıda veri okunduktan sonra 
        self.okuma_durumu = """Toplam {} veri okundu
            \nOrtalama: {}\nStandart Sapma: {}
            """.format(
                self.ornek_sayisi,
                np.mean(self.guncel_veriler[1:]),
                np.std(self.guncel_veriler[1:]))
        self.ser.close()
        # İşletim thread_kontrol metoduna geçer
        
    # Veri okuma threadi sonlanana kadar beklemek için kullanılır
    def thread_kontrol(self, t):
        # Okuma sırasında gelen mesajları yazar
        if self.okuma_durumu.startswith("Mesaj:"):
            self.mesaj_yaz(self.okuma_durumu)
        self.update_idletasks()
        
        # Okuma devam ediyorsa 100 ms bekleyip döngü devam eder
        if t.is_alive():
            self.after(100, self.thread_kontrol, t)
        
        # Okuma sonlandığında   
        else:
            # İptal butonuyla sonlanmışsa
            if self.iptal:
                self.iptal = False
                self.mesaj_yaz("İptal edildi")
            # Hata ile sonlanmışsa
            elif self.okuma_durumu.startswith("Hata:"):
                self.hata_yaz(self.okuma_durumu)
            # Başarıyla sonlanmışsa
            else:
                # Okunan verileri tüm veriye ekle
                self.veriler.append(self.guncel_veriler)
                self.mesaj_yaz(self.okuma_durumu)
            # Arayüzü hazırla
            self.basla_modu()


# Uygulama başlatılır
App().mainloop()

# DMA_in_STM32F446RE_NUCLEO

DMA (Direct Memory Access - Doğrudan Bellek Erişimi), STM32 mikrodenetleyiciler ve diğer mikrodenetleyicilerde kullanılan, işlemciyi meşgul etmeden veri transferini hızlı bir şekilde gerçekleştiren bir birimdir.
STM32F446RE gibi mikrodenetleyicilerde DMA birimi, çevresek birimler (örneğin UART, SPI, DAC, ADC) ile bellek arasında veya bellekten belleğe veri transferi yapabilir.


## DMA Nedir ve Nasıl Çalışır?

DMA, mikrodenetleyici çekirdeği (CPU) yerine doğrudan veri aktarımını gerçekleştiren bir donanım birimidir. Normalde, veri transferi sırasında CPU, veri aktarımı işlemini kontrol eder ve bu da CPU'nun zamanını harcar. 

**DMA kullanıldığında :**
1. CPU, DMA'ya hangi veriyi, hangi kaynaktan alacağını, nereye göndereceğini ve ne kadar veri transfer edeceğini belirtir.
2. DMA, belirtilen veri transferini CPU'dan bağımsız bir şekilde gerçekleştirir.
3. Veri transferi tamamlandığında, DMA birimi CPU'ya bir kesme (interrupt) göndererek işlemin tamamlandığını bildirir.

DMA isteği için çevresel birim tarafından (UART, ADC, DAC, I2C vs) DMA kontrolcüsüne istek gönderilir, kontrolcü bu isteğin sırası gelince ilgili çevresel birime geri bildirimde bulunur. İşlem kaynak adresten hedef adrese doğru gerçekleşir.


## DMA'nın Avantajları

1. **CPU Yükünü Azaltır :** CPU, veri aktarımı işlemiyle uğraşmak yerine diğer görevler için serbest kalır.
2. **Daha Hızlı Veri Transferi :** DMA birimi doğrudan donanım üzerinden çalıştığı için veri transferi daha hızlı yapılır.
3. **Enerji Verimliliği :** CPU'nın gereksiz yere çalışmasını önlediği için enerji tasarrufu sağlar.
4. **Kesintisiz Çalışma :** DMA, veri aktarımı sırasında CPU'ya ihtiyaç duymadığı için gerçek zamanlı görevlerin aksamasını önler. 


## DMA'nın Kullanım Alanları

1. **Veri Transferi :** UART, SPI, I2C gibi çevresel birimlerden gelen veya bu birimlere giden büyük veri bloklarını transfer etmek.
2. **ADC ve DAC ile Çalışma :** ADC' den gelen sürekli veri akışını belleğe yazmak veya DAC ile sürekli veri üretmek.
3. **Bellekten Belleğe Transfer :** RAM içinde veri kopyalama işlemlerinde.
4. **Grafik ve Ses İşleme :** LCD ekran veya ses verilerinin hızlı bir şekilde taşınması.


## DMA'nın STM32F446RE' deki Yapısı

STM32F446RE mikrodenetleyicisinde iki adet DMA kontrolcüsü bulunur: DMA1 ve DMA2. Her bir kontrolcünün kanalları vardır (DMA1: 8 kanal, DMA2: 8 kanal). Her bir kanal bir çevresel birime veya bir veri transferine atanabilir.

DMA1'in DMA2'den, kanal 1'in kanal 2'den yüksek olduğu bilinmektedir.
* Öncelik sırası belirtmek için dört seviye vardır. Low, Medium, High, Very High.

DMA'lar paralel olarak çalışmazlar, seri olarak çalışırlar. Bu nedenle hangisinin sırası geldi ise o anda o çalışır.


**DMA Transfer Modları**

1. **Peripheral-to-Memory :**
   * Örnek : ADC'den gelen veriyi RAM'e yazmak.

2. **Memory-to-Peripheral :**
   * Örnek : RAM'den alınan veriyi UART ile göndermek.

3. **Memory-to-Memory :**
   * Örnek : RAM içindeki iki blok arasında veri kopyalama.

**DMA'da Transfer Tipleri**

1. **Normal Mode :** Transfer tamamlandıktan sonra işlem durur.
2. **Circular Mode :** Transfer bittiğinde baştan başlar. Örnek : Sürekli ADC okuma.
3. **Double Buffer Mode :** Bir tampon (buffer) dolarken diğeri boşaltılır, böylece kesintisiz veri akışı sağlanır.


## DMA Kullanım Adımları

DMA kullanımı için aşağıdaki adımlar izlenir.

1. **DMA Kanallarının Konfigürasyonu :**
   * Kaynak ve hedef adreslerin ayarlanması.
   * Veri uzunuluğunun belirlenmesi.
   * Transfer modu ve yönünün seçilmesi.

2. **Çevresel Birim Konfigürasyonu :**
   * DMA'nın kullanılacağı çevresel birimin (UART, ADC, vb.) DMA desteği açılır.

3. **Kesme (Interrupt) Ayarları :**
   * Transfer tamamlandığında veya hata oluştuğunda kesme kullanılır.

4. **DMA Aktifleştirilmesi :**
  * DMA kanalı başlatılır ve transfer işlemi otomatik olarak yapılır.



## DMA1 Request Mapping

![image](https://github.com/user-attachments/assets/7151ced7-2e50-4ed3-b1df-06b63b02620d)


## DMA Request Mapping

![image](https://github.com/user-attachments/assets/46872080-692d-4721-9379-e439d26668ad)





























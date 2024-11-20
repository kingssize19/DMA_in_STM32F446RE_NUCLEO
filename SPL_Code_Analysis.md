## Bu kodda STM32F4 mikrodenetleyicisi kullanılarak ADC'den okunan veriler DMA (Direct Memory Access) aracılığıyla hafızaya yazılıyor. Ardından bu veriler UART (USART2) üzerinden terminal ekranında görüntüleniyor.

### 1. GPIO_Config Fonksiyonu

**Bu fonksiyon, ADC giriş pini olan PA0'ı analog giriş olarak yapılandırır.**

```c
void GPIO_Config()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;          // Analog mod seçildi
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;        // Çıkış tipi push-pull, analog modda etkisiz
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;             // PA0 pinini seçiyoruz
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;      // Pull-up/down kullanılmıyor
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;     // GPIO hızını 50 MHz olarak belirtiyoruz

    GPIO_Init(GPIOA, &GPIO_InitStruct);
}
```
* **RCC_AHB1PeriphClockCmd :** GPIOA'nın saat sinyalini etkinleştirir.
* **GPIO_Mode_AN :** Analog mod seçilir; ADC'nin çalışabilmesi için gereklidir.
* **GPIO_PuPd_NOPULL :** Analog modda çekme direnci (pull-up/down) kullanılmaz
* **Sonuç :** PA0 pini ADC'nin analog giriş sinyali alabilmesi için hazırlanır.


### 2. ADC_Config Fonksiyonu

**Bu fonksiyon, ADC1 çevresel biriminin analog sinyali dijitale çevirebilmesi için yapılandırır.**

```c
void ADC_Config()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_CommonStruct.ADC_Mode = ADC_Mode_Independent;                        // Tek bir ADC kanalı kullanıldığı için bağımsız mod
    ADC_CommonStruct.ADC_Prescaler = ADC_Prescaler_Div4;                     // ADC'yi 4'e bölerek saat frekansını düşür
    ADC_CommonStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;         // Tek ADC kullanımı için DMA erişimi kapalı
    ADC_CommonStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;   // İki ADC için örnekleme gecikmesi

    ADC_CommonInit(&ADC_CommonStruct);

    ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;                      // Çözünürlük 12-bit
    ADC_InitStruct.ADC_ScanConvMode = ENABLE;                                // Tarama modu etkin (birden fazla kanal için)
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;                      // Veriler sağa hizalanır
    ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // Harici tetikleme yok
    ADC_InitStruct.ADC_ExternalTrigConv = 0;                                 // Harici tetikleyici kullanılmaz
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;                          // Sürekli dönüşüm modu
    ADC_InitStruct.ADC_NbrOfConversion = 1;                                  // Tek kanal kullanılacak

    ADC_Init(ADC1, &ADC_InitStruct);

    ADC_Cmd(ADC1, ENABLE);                                                    // ADC'yi etkinleştir

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles); // Kanal 0 (PA0) seçilir, örnekleme süresi 3 döngü
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);                         // DMA'nın her dönüşümden sonra veri alması sağlanır
    ADC_DMACmd(ADC1, ENABLE);                                                 // ADC'nin DMA desteği etkinleştirilir
}
```

* **ADC_Mode_Independent :** ADC1 tek başına çalışır; diğer ADC'lerle bağlı çalışmaz.
* **ADC_Resolution :** 12-bit çözünürlük; 0-4095 arası dijital değer.
* **ADC_ContinuousConvMode :** Sürekli mod; ADC veri dönüştürmeye sürekli devam eder.
* **ADC_Channel_0 :** Analog giriş kanalı PA0 seçilir.
* **Sonuç :** ADC1, analog veriyi sürekli olarak dijitale çevirir.


### 3. DMA_Config Fonksiyonu

**DMA'yı yapılandırır. ADC'den gelen dijital veriyi CPU müdahalesi olmadan doğrudan adc_value adlı RAM'deki dizisine aktarır.**

```c
void DMA_Config()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_InitStruct.DMA_Channel = DMA_Channel_0;                                 // ADC1 ile ilişkili DMA kanalı
    DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;                        // Öncelik seviyesi çok yüksek
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;                // ADC veri register adresi
    DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&adc_value;                  // ADC verisinin RAM'e yazılacağı adres
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;                        // Çevresel birimden belleğe veri aktarımı
    DMA_InitStruct.DMA_BufferSize = BUFFER_LENGTH;                              // ADC verisi için tampon boyutu
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;                          // FIFO modu etkin
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;              // FIFO doluluk eşiği (16 bit yarım dolu)
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;                    // Hafıza verisi tek tek gönderilir
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;            // Çevresel verisi tek tek gönderilir
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                                // Sürekli veri transferi için dairesel mod
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;    // ADC verisi 16-bit
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;            // RAM'e yazılan veri boyutu 16-bit
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                        // Bellekte artış sağlanır (Aynı adres üzerine değil ardışık gelen                                                                                  //                          adreslere veriler yazılır)

    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;               // Çevresel adres sabit

    DMA_Init(DMA2_Stream0, &DMA_InitStruct);
    DMA_Cmd(DMA2_Stream0, ENABLE);                                              // DMA'yı çalıştır
}

```
* **DMA_DIR_PeripheralToMemory :** Veri ADC'nin data register'ından (çevresel birim) RAM'e aktarılır.
* **DMA_Mode_Circular :** Tampon dolduğunda baştan yazmaya devam eder; sürekli okuma sağlar.
* **Sonuç :** ADC verisi doğrudan RAM'e aktarılır, CPU yükü azaltılır.


### 4. USART2_Config Fonksiyonu

**UART yapılandırması yapılarak ADC verisinin seri port üzerinden gönderilmesini sağlar.**

```c
void USART2_Config()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;                      // Alternatif fonksiyon modu
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;            // USART2 TX (PA2) ve RX (PA3)
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;                      // Pull-up etkin
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                    // Push-pull çıkış tipi
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;                 // GPIO hızını 50 MHz olarak ayarla

    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART2, &USART_InitStruct);
    USART_Cmd(USART2, ENABLE);
}

```


### 5. USART2_SendString Fonksiyonu

**Bu fonksiyon, UART üzerinden bir karakter dizisi göndermek için yazılmıştır. Gönderilecek dizi, karakter karakter gönderilir ve her bir karakterin aktarımı tamamlanana kadar beklenir.**

```c
void USART2_SendString(char *str)
{
    while (*str)  // Gönderilecek karakter dizisinin sonuna kadar devam eder
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET); 
        // TXE bayrağı (Transmit Data Register Empty) sıfırsa bekler.
        // Bu bayrak, veri göndermek için USART'ın hazır olup olmadığını belirtir.

        USART_SendData(USART2, *str++); 
        // Veri gönderme işlemi: karakter USART'ın veri registerine yazılır
        // ve seri hat üzerinden iletilir.
    }
}
```
**Detaylı Açıklama:**

**1. while (*\str) Döngüsü :**
  * str dizisinin sonuna kadar her bir karakter üzerinde çalışır.
  * C'deki karakter dizileri, null karakter (\0) ile sonlanır.
  * Null karaktere ulaştığında döngü sona erer.

**2. USART_GetFlagStatus Kontrolü :**
  * USART_FLAG_TXE (Transmit Data Register Empty) bayrağı kontrol edilir.
  * Bu bayrak, veri gönderme registerinin (TXE) boş olduğunu ve yeni veri kabul edebileceğini belirtir.
  * Eğer bayrak RESET durumundaysa, UART meşguldür ve işlem tamamlanana kadar beklenir.

**3. USART_SendData :**
  * Mevcut karakter, UART veri registerine (USART_DR) yazılır.
  * Veri, seri iletişim hattı üzerinden gönderilir.
  * Karakter gönderildikten sonra, USART_FLAG_TXE tekrar SET olur ve yeni veri gönderimine hazır hale gelir.

**Akış :**
1. USART2_SendString("Merhaba"); çağrıldığında, "Merhaba" karakter dizisi sırayla işlenir:
  * 'M' gönderilir, ardından 'e', 'r', 'h', 'a', 'b', 'a', ve son olarak null karaktere ulaşılır.

2. Gönderilen her bir karakter için, TXE bayrağının boş olduğunu kontrol eder ve UART üzerinden gönderir.












































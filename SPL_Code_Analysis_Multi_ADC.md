## Bu kodda, STM32F4 mikrodenetleyicisi kullanılarak iki ADC kanalından analog veri okuması yapılır ve bu veriler DMA ile hafızaya aktarılır. Daha sonra, UART (USART2) kullanılarak bu veriler terminal ekranında görüntülenir.

## 1. GPIO_Config Fonksiyonu

```c
void GPIO_Config()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;                   // Analog mod
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                 // Push-pull, analog modda etkisiz
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;         // PA0 ve PA1 pinleri ADC girişleri
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;               // Pull-up/down kullanılmaz
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;              // Hız 50 MHz

	GPIO_Init(GPIOA, &GPIO_InitStruct);
}
```

* PA0 ve PA1 pinleri ADC giriş pinleri olarak yapılandırılır.
* Analog sinyal girişine uygun olarak pull-up/down direnci kullanılmaz.


## 2. ADC_Config Fonksiyonu 

```c
void ADC_Config()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_CommonStruct.ADC_Mode = ADC_Mode_Independent;                                      // Tek ADC kullanımı
	ADC_CommonStruct.ADC_Prescaler = ADC_Prescaler_Div4;                                   // ADC saat frekansı bölme oranı
	ADC_CommonStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;                       // DMA tek ADC'ye bağlı
	ADC_CommonStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;                 // Örnekleme gecikmesi

	ADC_CommonInit(&ADC_CommonStruct);

	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;                                    // 12-bit çözünürlük
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;                                              // Çoklu kanal okuma
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;                                    // Sağ hizalı veri
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;               // Harici tetikleme yok
	ADC_InitStruct.ADC_ExternalTrigConv = 0;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;                                        // Sürekli çevrim modu
	ADC_InitStruct.ADC_NbrOfConversion = BUFFER_LENGTH;                                    // Çevrim sayısı (2 kanal)

	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_Cmd(ADC1, ENABLE);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);              // Kanal 1 (PA0)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);              // Kanal 2 (PA1)

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);                                      // Her transfer sonrası DMA talebi
	ADC_DMACmd(ADC1, ENABLE);                                                              // ADC ile DMA entegrasyonu
}
```

* **Çoklu Kanal Desteği :**
  * **ADC_ScanConvMode = ENABLE :** Çoklu kanal okuma için tarama modu aktif edilir.
  * **ADC_NbrOfConversion = BUFFER_LENGTH :** İki kanalın çevrim yapılacağını belirtir.
  * Kanal 0 (PA0) ve Kanal 1 (PA1) sırayla seçilir.

* **DMA Desteği :**
  *  ADC, çevrimleri tamamlandığında DMA'ya veri transfer talebi gönderir.
  *  ADC sürekli çevrim modunda çalışır ve iki kanal sırasıyla okunur.


## 3. DMA_Config Fonksiyonu

```c
void DMA_Config()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_InitStruct.DMA_Channel = DMA_Channel_0;                                                // ADC1 DMA kanalı
	DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;                                       // Yüksek öncelik
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;                               // ADC veri adresi
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&adc_value;                                 // Verinin yazılacağı dizi adresi
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;                                       // Çevresel birimden RAM'e aktarım
	DMA_InitStruct.DMA_BufferSize = BUFFER_LENGTH;                                             // Tampon boyutu (2 kanal)
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;                                         // FIFO kullanımı
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;                             // FIFO doluluk oranı
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;                                   // Tekli bellek erişimi
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;                           // Tekli çevresel erişim
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                                               // Sürekli veri transferi
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;                   // 16-bit veri
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;                           // 16-bit bellek
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                                       // Bellek adresi artışı
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                              // Çevresel adres sabit

	DMA_Init(DMA2_Stream0, &DMA_InitStruct);
	DMA_Cmd(DMA2_Stream0, ENABLE);
}
```

* DMA, ADC çevrimlerinden çıkan 16-bit verileri adc_value dizisine aktarır.
* **DMA_DIR_PeripheralToMemory :** ADC veri register'ından (ADC1->DR) RAM'e veri aktarır.
* **DMA_Mode_Circular :** Tampon dolunca baştan yazılmaya devam edilir.


## 4. USART2_Config Fonksiyonu

```c
void USART2_Config()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;                        // Alternatif fonksiyon modu
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;              // TX ve RX pinleri
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;                        // Pull-up etkin
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                      // Push-pull çıkış tipi
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;                   // Hız 50 MHz

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


## 5. USART2_SendString Fonksiyonu

```c
void USART2_SendString(char *str)
{
	while (*str)
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		USART_SendData(USART2, *str++);
	}
}
```

* Gelen karakter dizisi, UART üzerinden karakter karakter gönderilir.
* **USART_FLAG_TXE :** Veri gönderme register'ının boş olduğunu kontrol eder.


## 6. main Fonksiyonu

```c
int main(void)
{
	GPIO_Config();
	ADC_Config();
	DMA_Config();
	USART2_Config();
	ADC_SoftwareStartConv(ADC1); // ADC dönüşüm işlemini başlat

	while (1)
	{
		snprintf(buffer, sizeof(buffer), "ADC Value - 1: %d\n\r", adc_value[0]);
		USART2_SendString(buffer); // İlk kanalın verisini gönder

		snprintf(buffer2, sizeof(buffer2), "ADC Value - 2: %d\n\r", adc_value[1]);
		USART2_SendString(buffer2); // İkinci kanalın verisini gönder

    delay_second(1);                // fonksiyon tanımlanıp saniye formatında bekleme oluşturulur.
  }

}
```



























## Bu kod, STM32F446RE mikrodenetleyicisinin ADC modülü ile DMA'yı kullanarak potansiyometrelerden gelen analog sinyalleri dijital değerlere çevirip bu verileri bir bellek alanına aktarmak için tasarlanmıştır. Kod, ADC konfigürasyonu, GPIO ayarları, DMA ayarları ve saat yapılandırması içerir. 

## RCC_Config (Saat Konfigürasyonu)

```c
void RCC_Config(void)
{
    // HSI'yi kapatıyoruz
    RCC->CR &= ~(1 << 0);                    // HSI OFF

    // HSE'yi aktif ediyoruz
    RCC->CR |= (1 << 16);                    // HSE ON
    while (!(RCC->CR & (1 << 17)));          // HSE aktif olana kadar bekle

    // FLASH latency ayarları (5 wait states)
    FLASH->ACR |= FLASH_ACR_LATENCY_5WS;

    // PLL ayarları (HSE kaynağına göre)
    RCC->PLLCFGR = 0;                        // PLLCFGR'yi sıfırlama

    // PLL ayarları
    RCC->PLLCFGR |= (1 << 22);               // PLL Kaynağı HSE
    RCC->PLLCFGR |= (4 << 0);                // PLL_M = 4 (HSE'yi 2 MHz'e böl)
    RCC->PLLCFGR |= (180 << 6);              // PLL_N = 180 (2 MHz * 180 = 360 MHz)
    RCC->PLLCFGR |= (0 << 16);               // PLL_P = 2 (360 MHz / 2 = 180 MHz)
    RCC->PLLCFGR |= (7 << 24);               // PLL_Q = 7 (USB için)

    // PLL'yi aktif ediyoruz
    RCC->CR |= (1 << 24);                    // PLL ON
    while (!(RCC->CR & (1 << 25)));          // PLL aktif olana kadar bekle

    // Sistem saat kaynağını PLL olarak seç
    RCC->CFGR &= ~(3 << 0);                  // Saat kaynağını temizle
    RCC->CFGR |= (2 << 0);                   // Sistem saat kaynağı olarak PLL'yi seç

    // Saat kaynağının PLL olduğunu kontrol et
    while ((RCC->CFGR & (3 << 2)) != (2 << 2));  // PLL'nin sistem saati olduğunu doğrula
}
```

**RCC->CR**
* **HSION :** Dahili HSI osilatörü kapatılıyor. **RCC->CR &= ~(1 << 0);** 
* **HSEON :** Harici HSE osilatörü etkinleştiriliyor. **RCC->CR |= (1 << 16);**
* **HSERDY :** HSE'nin hazır olduğunu beklemek için kullanılıyor. **while (!(RCC->CR & (1 << 17)));**

**FLASH->ACR**
* **LATENCY :** Flash bellek bekleme durumlarını ayarlar. Burada 5 bekleme durumu seçildi. **FLASH->ACR |= FLASH_ADCR_LATENCY_5WS;**

**RCC->PLLCFGR** 
* **PLLSRC :** PLL kaynağı olarak HSE seçiliyor. **RCC->PLLCFGR |= (1 << 22);**
* **PLLM :** HSE frekansını 2 MHz'e bölmek için PLLM = 4 ayarlanıyor. **RCC->PLLCFGR |= (4 << 0);**
* **PLLN :** PLL çıkış frekansı için PLLN = 180 ayarlanıyor. **RCC->PLLCFGR |= (0 << 16);**
* **PLLP :** Ana sistem saati için PLLP = 2 seçiliyor. **RCC->PLLCFGR |= (0 << 16);**
* **PLLQ :** USB saati için PLLQ = 7 ayarlanıyor. **RCC->PLLCFGR |= (7 << 24);**

**RCC->CFGR**
* **SW :** Sistem saati kaynağı olarak PLL seçiliyor. **RCC->CFGR |= (2 << 0);**
* **SWS :** PLL'nin seçildiğini doğrulamak için kontrol ediliyor. **while ((RCC->CFGR & (3 << 2)) != (2 << 2));**


## GPIO_Config (GPIO Konfigürasyonu)

```c
void GPIO_Config()
{
	RCC->AHB1ENR |= (1 << 0);				//GPIOA clock bus active

	//ANALOG MODE
	GPIOA->MODER |= (1 << 0);
	GPIOA->MODER |= (1 << 1);

	//PUSH PULL
	GPIOA->OTYPER &= ~(1 << 0);

	//PULL DOWN
	GPIOA->PUPDR &= ~(1 << 0);
	GPIOA->PUPDR |= (1 << 1);

	//MEDİUM SPEED
	GPIOA->OSPEEDR |= (1 << 0);
	GPIOA->OSPEEDR &= ~(1 << 1);
}
```


## ADC_Config (ADC Konfigürasyonu)

```c
void ADC_Config()
{
	//APB2 enable
	RCC->APB2ENR |= (1 << 8);

	ADC1->CR1 |= (1 << 8);				//ADC1 scan mode enable

	//Resolution 12 bit
	ADC1->CR1 &= ~(1 << 24);
	ADC1->CR1 &= ~(1 << 25);

	ADC1->CR2 |= (1 << 0);				// ADC enable
	ADC1->CR2 |= (1 << 1);				//Continuous conversion mode

	ADC1->CR2 |= (1 << 8);				//DMA mode enable
	ADC1->CR2 |= (1 << 9);				//DDS enable (DMA sürekli okuma yapsın)

	ADC1->CR2 |= (1 << 10);				//EOC Flag active

	ADC1->CR2 &= ~(1 << 11);			//Right alignment

	ADC1->CR2 |= (1 << 30);				//Start conversion

	//For 1 conversion	0000 : 1 conversion
	ADC1->SQR1 &= ~(1 << 20);
	ADC1->SQR1 &= ~(1 << 21);
	ADC1->SQR1 &= ~(1 << 22);
	ADC1->SQR1 &= ~(1 << 23);

	ADC1->SQR3 &= ~(1 << 0);
}
```

**RCC->APB2ENR**
* **ADC1EN** : ADC1 saat sinyali etkinleştiriliyor. **RCC->APB2ENR |= (1 << 8);**

**ADC1->CR1**
* **SCAN :** Taramalı mod etkinleştiriliyor. **ADC1->CR1 |= (1 << 8);**
* **RES :** ADC çözünürlüğü 12-bit olarak ayarlanıyor. **ADC1->CR1 &= ~(1 << 24);** **ADC1->CR1 &= ~(1 << 25);**

**ADC1->CR2**
* **ADON :** ADC etkinleştiriliyor. **ADC1->CR2 |= (1 << 0);**
* **CONT :** Sürekli dönüşüm modu etkinleştiriliyor. **ADC1->CR2 |= (1 << 1);**
* **DMA :** DMA modu etkinleştiriliyor. **ADC1->CR2 |= (1 << 8);**
* **EOCS :** EOC bayrağı etkinleştiriliyor. **ADC1->CR2 |= (1 << 10);**
* **ALIGN :** Sağ hizalama ayarlanıyor. **ADC1->CR2 &= ~(1 << 11);**
* **SWSTART :** Dönüşüm başlatılıyor. **ADC1->CR2 |= (1 << 30);**

**ADC1->SQR1**
* **L :** Tek dönüşüm için ayarlanıyor. **ADC1->SQR1 &= ~(1 << 20);**

**ADC1->SQR3**
* **SQ1 :** 1. sıradaki kanal seçiliyor (kanal 0). **ADC1->SQR3 &= ~(1 << 0);**


## DMA_Config (DMA Konfigürasyonu)

```c
void DMA_Config()
{
	RCC->AHB1ENR |= (1 << 22);				//DMA2 clock enable

	while ((DMA2_Stream4->CR & 0x00000001) == 1);			//wait for stream4 to be 0


	//Data transfer direction 00 : Peripheral to memory
	DMA2_Stream4->CR &= ~(1 << 6);
	DMA2_Stream4->CR &= ~(1 << 7);

	//Circular mode enable
	DMA2_Stream4->CR |= (1 << 8);

	//Peripheral address fixed (adresi değişmesin! sabit kalsın)
	DMA2_Stream4->CR &= ~(1 << 9);

	//Memory adresi değişsin veriler tek adreste üst üste yazılmasın sırasıyla yazılsın
	DMA2_Stream4->CR |= (1 << 10);		//Memory address pointer is incremented

	//PSIZE -> çevresel birimden gelen data (12 bit adc okuduğumuz için 16 bit(half word) (01) yapabiliriz)
	DMA2_Stream4->CR |= (1 << 11);
	DMA2_Stream4->CR &= ~(1 << 12);

	//MSIZE PSIZE ile aynı 16 bit seçilir
	DMA2_Stream4->CR |= (1 << 13);
	DMA2_Stream4->CR &= ~(1 << 14);

	//Priority level is very high 11
	DMA2_Stream4->CR |= (1 << 16);
	DMA2_Stream4->CR |= (1 << 17);

	//channel 0 selected
	DMA2_Stream4->CR &= ~(1 << 25);
	DMA2_Stream4->CR &= ~(1 << 26);
	DMA2_Stream4->CR &= ~(1 << 27);

	//Kaç kanaldan okuma yapacağız (1 single channel)
	DMA2_Stream4->NDTR |= (1 << 0);

	//peripheral address register
	DMA2_Stream4->PAR |= (uint32_t) &ADC1->DR;

	//Memory zero address (variable address)
	DMA2_Stream4->M0AR |= (uint32_t) &adc_value;


	//FIFO Threshold 1/2 full FIFO
	DMA2_Stream4->FCR |= (1 << 0);
	DMA2_Stream4->FCR &= ~(1 << 1);

	//stream4 active
	DMA2_Stream4->CR |= (1 << 0);

}
```

**RCC->AHB1ENR**
* **DMA2EN :** DMA2 saat sinyali etkinleştiriliyor. **RCC->AHB1ENR |= (1 << 22);**

**DMA2_Stream4->CR**
* **DIR :** Çevresel birimden belleğe veri aktarımı seçiliyor. **DMA2_Stream4->CR &= ~(1 << 6);**
* **CIRC :** Dairesel mod etkinleştiriliyor. **DMA2_Stream4->CR |= (1 << 8);**
* **PINC :** Çevresel adres sabitleniyor. **DMA2_Stream4->CR &= ~(1 << 9);**
* **MINC :** Bellek adresi otomatik artırılıyor. **DMA2_Stream4->CR |= (1 << 10);**
* **PSIZE :** Çevresel birim veri boyutu 16-bit olarak ayarlanıyor. **DMA2_Stream->CR |= (1 << 11);** **DMA2_Stream4->CR &= ~(1 << 12);**
* **MSIZE :** Bellek veri boyutu 16 bit olarak ayarlanıyor. **DMA2_Stream4->CR |= (1 << 13);** **DMA2_Stream4->CR &= ~(1 << 14);**
* **PL :** Öncelik seviyesi çok yüksek olarak ayarlanıyor. **DMA2_Stream4->CR |= (1 << 16);** **DMA2_Stream4->CR |= (1 << 17);**
* **CHSEL :** Kanal 0 seçiliyor. **DMA2_Stream4->CR &= ~(1 << 25);**

**DMA2_Stream4->PAR** 
* **PAR :** Çevresel adres olarak ADC1->DR atanıyor. **DMA2_Stream4->PAR |= (uint32_t)&ADC1->DR;**

**DMA2_Stream4->M0AR**
* **M0AR :** Bellek adresi olarak adc_value atanıyor. **DMA2_Stream->M0AR |= (uint32_t)&adc_value;**

**DMA_Stream4->FCR**
* **FTH :** FIFO eşiği yarı dolu olarak ayarlanıyor. **DMA2_Stream4->FCR |= (1 << 0);** **DMA2_Stream4->FCR &= ~(1 << 1);**


**DMA2_Stream4->CR |= (1 << 0);** DMA stream4 aktif ediliyor.











































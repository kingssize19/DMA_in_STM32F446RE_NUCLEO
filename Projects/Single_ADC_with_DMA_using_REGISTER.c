#include "stm32f4xx.h"
#include "stm32f446xx.h"


uint16_t adc_value[1];
uint16_t adc;

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

void GPIO_Config()
{
	RCC->AHB1ENR |= (1 << 0);				//GPIOA clock bus active

	//LDR CONFİG

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

void delay(uint32_t time)
{
	while (time--);
}



int main(void)
{
	RCC_Config();
	GPIO_Config();
	ADC_Config();
	DMA_Config();

	//ADC1 start
	ADC1->CR2 |= 0x40000000;
  while (1)
  {
	  adc = adc_value[0];
	  delay(1);

  }
}

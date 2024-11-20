#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include <stdio.h>

#define BUFFER_LENGTH	2

uint16_t adc_value[BUFFER_LENGTH];
char buffer[25];
char buffer2[25];

GPIO_InitTypeDef GPIO_InitStruct;
ADC_InitTypeDef ADC_InitStruct;
ADC_CommonInitTypeDef ADC_CommonStruct;
DMA_InitTypeDef DMA_InitStruct;
USART_InitTypeDef USART_InitStruct;

void GPIO_Config()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ADC_Config()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_CommonStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonStruct.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;

	ADC_CommonInit(&ADC_CommonStruct);

	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv = 0;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;			//SÜREKLİ ÇEVRİM MODU
	ADC_InitStruct.ADC_NbrOfConversion = BUFFER_LENGTH;					//ÇEVRİM SAYISI

	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_Cmd(ADC1, ENABLE);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
}


void DMA_Config()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
	DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;  //ADC1 DATA REGİSTER ADRESİ DMA VERİYİ ADC1 DR INDAN ALACAKSIN DİYORUZ
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&adc_value;	//adc1 den okuduğumuz veriyi bu değişken adresine yazıyoruz.
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;		//ÇEVRESEL BİRİMDEN ADRESE YAZACAĞIZ
	DMA_InitStruct.DMA_BufferSize = BUFFER_LENGTH;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;	//FIFO DOLULUK ORANINA GÖRE VERİ ÇIKIŞINI AYARLAR
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;		//NORMAL MODE TEK SEFER, CİRCULAR SÜREKLİ ÇEVRESEL BİRİMDEN VERİ ALIR VE OKUR VE MEMORYE GÖNDERİR
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;		//GELEN DEĞERLEİRN SÜREKLİ FARKLI REGİSTERLARA YAZILMASI İÇİN
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//VERİ OKDUĞUMUZ ADRES SÜREKLİ AYNI SABİT OLDUĞU İÇİN DISABLE EDİLİR.

	DMA_Init(DMA2_Stream0, &DMA_InitStruct);

	DMA_Cmd(DMA2_Stream0, ENABLE);

}

void USART2_Config()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // USART2 TX (PA2) and RX (PA3)
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

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

void USART2_SendString(char *str)
{
    while (*str)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, *str++);
    }
}

int main(void)
{

	GPIO_Config();
	ADC_Config();
	DMA_Config();
	USART2_Config();
	ADC_SoftwareStartConv(ADC1);  //WHİLE1 İÇİNDE YAZARSAK PERİPHERAL->CPU->RAM E YAZMIŞ OLURUZ. DMA SAYESİNDE DİREKT RAME YAZARIZ BURADA YAZARSAK

  while (1)
  {
	  snprintf(buffer, sizeof(buffer), "ADC Value - 1: %d\n\r", adc_value[0]);
	  USART2_SendString(buffer);
	  snprintf(buffer2, sizeof(buffer2), "ADC Value - 2: %d\n\r", adc_value[1]);
	  USART2_SendString(buffer2);
	  for (int i = 0; i < 9000000; i++); // Delay
  }
}
/*
 * Callback used by stm32f4_discovery_audio_codec.c.
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
  /* TODO, implement your code here */
  return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
  /* TODO, implement your code here */
  return -1;
}

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32g0xx.h"
#define delay_ms(ms) for(int i = 3203*ms; i; i--) __NOP()

uint16_t volatile adc_output;
uint8_t volatile adc_flag;

void adc_start()
{
  ADC1->IER |= ADC_IER_EOCIE;
  ADC1->CR |= ADC_CR_ADSTART;
}

int main(void)
{
  RCC->IOPENR |= RCC_IOPSMENR_GPIOBSMEN;
  RCC->APBENR2 |= RCC_APBENR2_ADCEN;

  ADC1->CR |= ADC_CR_ADVREGEN; // ADC Voltage Regulator Enable
  for(uint32_t i = 0; i < SystemCoreClock / 500000; i++) __DSB();

  ADC1->CR &= ~ADC_CR_ADEN;
  ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN;
  ADC1->CR |= ADC_CR_ADCAL; // Calibration
  while(!(ADC1->ISR & ADC_ISR_EOCAL)) __DSB();
  ADC1->ISR |= ADC_ISR_EOCAL;

  ADC->CCR |= (7 << ADC_CCR_PRESC_Pos);
  ADC1->CHSELR = (1 << 9); // Active channel ADC_IN9 - PB1

  NVIC_EnableIRQ(ADC1_COMP_IRQn);
  NVIC_SetPriority(ADC1_COMP_IRQn, 3);

  ADC1->CFGR2 = (4 << ADC_CFGR2_OVSS_Pos) | (7 << ADC_CFGR2_OVSR_Pos) | ADC_CFGR2_OVSE;
  ADC1->SMPR = (7 << ADC_SMPR_SMP1_Pos);

  do {
    ADC1->CR |= ADC_CR_ADEN;
  } while((ADC1->ISR & ADC_ISR_ADRDY) == 0);

  adc_start();

  while(1)
  {
    if(adc_flag)
    {
      adc_flag = 0;
      delay_ms(10);
      adc_start();
    }
    delay_ms(10);
  }
}


void ADC_COMP_IRQHandler(void)
{
  if(ADC1->ISR & ADC_ISR_EOC)
  {
    ADC1->ISR |= ADC_ISR_EOC;
    adc_output = ADC1->DR;
    adc_flag = 1;
  }
}


#include "delay.h"
#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"

void delayMs(uint16_t ms){
    /*ABILITAZIONE LINEE CLOCK*/

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		// abilitazione clock linea gpio PA5
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;		// abilitazione clock linea timer TIM7

	/*SETTAGGIO TIMER7*/

	TIM7->ARR = ms;
	TIM7->PSC |= 0xBB80;
	
	/*ABILITO CONTATORE*/

	TIM7->CR1 |= TIM_CR1_CEN;
	
	while((TIM7->SR & TIM_SR_UIF)!= TIM_SR_UIF);
	TIM7->SR ^= TIM_SR_UIF;
	
	TIM7->CR1 &= ~TIM_CR1_CEN; //disabilito contatore
	RCC->APB1ENR &= ~RCC_APB1ENR_TIM7EN;		// disabilito clock linea timer TIM7
}

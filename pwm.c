#include <stm32f0xx.h>
#include "pwm.h"
#include "pins.h"

// disabled PWM from all pins that use timer 1
// that is needed by the timer.c

// initializes the pwm subsystem
PWMError PWM_init(void){
  __disable_irq();  // disable interrupts

  TIM2->DIER &= (~TIM_DIER_UIE);
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;	// clock enable timer TIM2
  TIM2->CR1 = 0;
  TIM2->ARR = 0xFFFF;	// auto reload register value
  TIM2->PSC |= 0xBB80;	// 1 kHz (clock a 48 MHz)

  __enable_irq();	// enable interrupts

  return PWMSuccess;
}

// how many pwm on this chip
uint8_t PWM_numChannels(void){
  return PINS_NUM;
}

// what was the period i set in the pwm subsystem
// might only require to adjust the prescaler

//verify if PWM is enabled
PWMError PWM_isEnabled(uint8_t c) {
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->ccm1_register)
    return PWMChannelOutOfBound;
  if ((*pin->ccm1_register & pin->com_mask)==0)
    return 0;
  return PWMEnabled;
}

// sets the output on a pwm channel
PWMError PWM_enable(uint8_t c, uint8_t enable){
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->ccm1_register)
    return PWMChannelOutOfBound;
  *pin->dutyc_register=0;
  if (enable){
    *pin->ccm1_register |= pin->com_mask;
    *pin->dir_register |= ((1<<pin->bit*2)<<1);
    if(pin->bit<8)
        *pin->afr_register[0]=0x0010;	//set alternate function for TIM2_CHx
    else
	*pin->afr_register[1]=0x0010;
    TIM2->CCER |= TIM_CCER_CC1E;	// signal output pin enable
    TIM2->EGR = TIM_EGR_UG;		// initialize all registers before lets run the timer
    TIM2->CR1 |= TIM_CR1_CEN;		// counter enable
  } 
  else {
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->EGR &= ~TIM_EGR_UG; 
    TIM2->CCER &= ~TIM_CCER_CC1E;
    *pin->ccm1_register &= ~pin->com_mask;
    *pin->dir_register    &= ~((1<<pin->bit*2)<<1);
    if(pin->bit<8)
    	*pin->afr_register[0]&= ~0x0010;
    else
    	*pin->afr_register[1]&= ~0x0010;
  }
  return PWMSuccess;
}


// what was the duty cycle I last set?
uint8_t PWM_getDutyCycle(uint8_t c){
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->ccm1_register)
    return PWMChannelOutOfBound;
  return 0xFFFF-*pin->dutyc_register;
}

// sets the duty cycle
 PWMError PWM_setDutyCycle(uint8_t c, uint8_t duty_cycle){
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->ccm1_register)
    return PWMChannelOutOfBound;
  *pin->dutyc_register = 0xFFFF-duty_cycle;
  return PWMSuccess;
}


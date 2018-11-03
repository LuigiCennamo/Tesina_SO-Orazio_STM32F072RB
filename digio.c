#include "digio.h"
#include "pins.h"
#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"
#define NUM_CHANNELS 14

//OK
// initializes the digital io pins of the chip
void DigIO_init(void) {
  //enable clock ahb
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  // we set all pins as input
  uint8_t num_channels=DigIO_numChannels();
  for (int i=0; i<num_channels; ++i) {
    DigIO_setDirection(i,0);
    // pull up on each pin
    //DigIO_setValue(i,1);
    GPIOA->PUPDR|=0x2AA800;
    GPIOB->PUPDR|=0x151540;
    GPIOC->PUPDR|=1<<14;
  }
}

//OK
// returns the number of digital io pins on the chip
uint8_t  DigIO_numChannels(void){
  return PINS_NUM; // fixed maximum number of pins mapped to arduino
}

uint8_t DigIO_setDirection(uint8_t pin, PinDirection dir) {
  if (pin>=PINS_NUM)
    return 1;
  const Pin* mapping=pins+pin;
  uint8_t mask=1<<(mapping->bit*2);//vedi il registro MODER (RM_STM32F072.pdf p.157)
  if (dir)
    *(mapping->dir_register)|=mask;	//general purpose output mode
  else
    *(mapping->dir_register)&=~mask;//input mode
  return 0;
}

PinDirection DigIO_getDirection(uint8_t pin){
  if (pin>=PINS_NUM)
    return 0;
  const Pin* mapping=pins+pin;
  uint8_t value=*(mapping->dir_register);
  return (value >> pins[pin].bit)&0x1;
}

void DigIO_setValue(uint8_t pin, uint8_t value) {
  if (pin>=PINS_NUM)
    return;
  const Pin* mapping=pins+pin;
  uint8_t mask=1<<mapping->bit;
  if (value)
    *(mapping->out_register)|=mask;
  else
    *(mapping->out_register)&=~mask;
}

uint8_t DigIO_getValue(uint8_t pin){
  if (pin>=PINS_NUM)
    return 0;
  const Pin* mapping=pins+pin;
  uint8_t value=*(mapping->in_register);
  return (value >> pins[pin].bit)&0x1;
}

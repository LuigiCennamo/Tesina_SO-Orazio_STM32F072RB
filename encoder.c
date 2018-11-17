#include "encoder.h"

#define NUM_ENCODERS 2
#define ENCODER_MASK 0x000001E0 // bits of PORT A used for encoders: from PA5 to PA8

typedef struct{
  uint16_t current_value;
  uint16_t sampled_value;
  uint8_t  pin_state;
}  Encoder;

Encoder _encoders[NUM_ENCODERS]={
  {0,0},
  {0,0}
};


// initializes the encoder subsystem
void Encoder_init(void){
	__disable_irq(); // torn off interrupts
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // enable interrupt PA
	GPIOA->MODER &= ~ENCODER_MASK;// set input mode from PA5 to PA8
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR5_0 | GPIO_PUPDR_PUPDR6_0 | GPIO_PUPDR_PUPDR7_0 | GPIO_PUPDR_PUPDR8_0; // enable pull-up resistor
	EXTI->IMR |= EXTI_IMR_MR5 | EXTI_IMR_MR6 | EXTI_IMR_MR7 | EXTI_IMR_MR8; // enable interrupt musk
	EXTI->RTSR |= EXTI_RTSR_TR5 | EXTI_RTSR_TR6 | EXTI_RTSR_TR7 | EXTI_RTSR_TR8; // enable interrupts on rising edge
	EXTI->FTSR |= EXTI_FTSR_TR5 | EXTI_FTSR_TR6 | EXTI_FTSR_TR7 | EXTI_FTSR_TR8; // enable interrupts on falling edge
	EXTI->PR |= ENCODER_MASK; // clear the pending interrupt bits
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn,0);
	__enable_irq(); // turn on interrupts
}

// samples the encoders, saving the respective values in a temporary storage
void Encoder_sample(void){
	__disable_irq();
  for (uint8_t i=0; i<NUM_ENCODERS; ++i)
    _encoders[i].sampled_value=_encoders[i].current_value;
  __enable_irq();
}

// returns the number of the encoder 
uint8_t Encoder_numEncoders(void){
  return NUM_ENCODERS;
}

// returns the value of an encoder, when sampled with the Encoder_sample();
uint16_t Encoder_getValue(uint8_t num_encoder){
  return _encoders[num_encoder].sampled_value;
}

static const int8_t _transition_table []=
  {
      0,  //0000
     -1, //0001
      1,  //0010
      0,  //0011
      1,  //0100
      0,  //0101
      0,  //0110
     -1, //0111
     -1, //1000
      0,  //1001
      0,  //1010
      1,  //1011
      0,  //1100
      1,  //1101
     -1,  //1110
      0   //1111
    };


void EXTI4_15_IRQHandler(){
	__disable_irq();
	short port_value=GPIOA->IDR&ENCODER_MASK;
	EXTI->PR |= ENCODER_MASK; // clear the pending interrupt bits
	// encoder 0
	uint8_t new_pin_state=(uint8_t)((port_value>>5) & 0x03);
	uint8_t idx=(_encoders[0].pin_state<<2)| new_pin_state ;
	_encoders[0].current_value+=_transition_table[idx];
	_encoders[0].pin_state=new_pin_state;
	//encoder 1
	new_pin_state=(uint8_t)(port_value>>7);
	idx=(_encoders[1].pin_state<<2)| new_pin_state ;
	_encoders[1].current_value+=_transition_table[idx];
	_encoders[1].pin_state=new_pin_state;
	__enable_irq();
}

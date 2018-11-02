#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"
#include <string.h>
#include "timer.h"
#define  NUM_TIMERS 1

typedef struct Timer{
  int timer_num;
  uint16_t duration_ms;
  TimerFn fn;
  void* args;
} Timer;

static Timer timers[NUM_TIMERS];

void Timers_init(void){
  memset(timers, 0, sizeof(timers));
  for (int i=0; i<NUM_TIMERS; ++i)
    timers[i].timer_num=i;
}

// creates a timer that has a duration of ms milliseconds
// bound to the device device
// each duration_ms the function timer_fn will be called
// with arguments timer args
Timer* Timer_create(char* device,
		    uint16_t duration_ms,
		    TimerFn timer_fn,
		    void* timer_args){
  Timer* timer=0;
  if (!strcmp(device,"timer_0"))
    timer=timers;
  else
    return 0;
  timer->duration_ms=duration_ms;
  timer->timer_num=0;
  timer->fn=timer_fn;
  timer->args=timer_args;
  return timer;
}

// stops and destroyes a timer
void Timer_destroy(struct Timer* timer){
  Timer_stop(timer);
  __disable_irq();	// disable interrupt		
  int timer_num=timer->timer_num;
  memset(timer, 0, sizeof(Timer));
  timer->timer_num=timer_num;
  __enable_irq();	// enable interrupt
}

void _timer0_start(struct Timer* timer){	
	uint16_t ocrval=(uint16_t)(timer->duration_ms);

	/* enable clock lines */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		// enable clock gpio PA5
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;		// enable clock timer TIM14

	/* TIMER14 settings */
	TIM14->ARR = ocrval;
	TIM14->PSC |= 0xBB80;	
	NVIC_EnableIRQ(TIM14_IRQn);
	TIM14->DIER |= TIM_DIER_UIE; // Update interrupt enable (p.471 reference manual)
	TIM14->CR1 |= TIM_CR1_CEN; // counter enable
}

// starts a timer
void Timer_start(struct Timer* timer){
	__disable_irq();	// disable interrupt		
  	if (timer->timer_num==0)
    	_timer0_start(timer);
  	__enable_irq();	// enable interrupt
}


// stops a timer
void Timer_stop(struct Timer* timer){
  if (timer->timer_num!=0)
    return;
  __disable_irq();	// disable interrupt	
  TIM14->DIER &= (~TIM_DIER_UIE); // update interrupt disable (p.471 reference manual)
  __enable_irq();	// enable interrupt
}

// interrupt handler
void TIM14_IRQHandler() {
  TIM14->SR &= (~TIM_SR_UIF);
  if(timers[0].fn)
    (*timers[0].fn)(timers[0].args);
}

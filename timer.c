//#include <avr/io.h>//include da eliminare
//#include <avr/interrupt.h>

#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"
#include <string.h>


// here we hook to timer 5 which is not used in any of the primary arduino pins
// its 16 bit and allows us a decent resolution

#include "timer.h"
// just 1 timer for the user
// we hook to timer 5
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
  //cli();                         //inizio sezione critica (disattiva global interrupt)
  __disable_irq();				   //noi fin ora abbiamo usato questo
  int timer_num=timer->timer_num;
  memset(timer, 0, sizeof(Timer));
  timer->timer_num=timer_num;
  //sei();						//fine sezione critica, riabilita interrupts
  __enable_irq();
}

void _timer0_start(struct Timer* timer){
  //uint16_t ocrval=(uint16_t)(15.62*timer->duration_ms);
  uint16_t ocrval=(uint16_t)(timer->duration_ms);
  //registri da cambiare
	  //TCCR1A = 0;		//TCCR1A/B insieme costituiscono dei registri di controllo del timer
	  //TCCR1B = 0;		//se entrambi a 0 significa che il timer si comporta funzionamento 
						//normale dei pin (da 0000 a FFFF con flag attivato ad overflow)
						//(p.170 dataseet AtMega 328, normal mode p.161)
						
	  //OCR1A = ocrval; //OCR1A è il byte meno significativo del registro di confronto, 
						//valore di fine conteggio(p.180), probabilmente da ricalcolare...
						
	  //TCCR1B |= (1 << WGM12) | (1 << CS10) | (1 << CS12); //CS10 e CS12 selezionano la sorgente di clock (p.173)
															//WGM12 (p.172) invece seleziona la modalità CTC (Clear timer on compare match p.132)
															
	  //TIMSK1 |= (1 << OCIE1A);	//abilita l'interrupt (p.184)
	  
	  /*ABILITAZIONE LINEE CLOCK*/
	  
	  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		// abilitazione clock linea gpio PA5
	  RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;		// abilitazione clock linea timer TIM14
	  
	  /*SETTAGGIO TIMER14*/

	  TIM14->ARR = ocrval;
	  TIM14->PSC |= 0xBB80;	//48000 perché il clock va a 48 MHz, così contiamo i ms per colpo di timer (riferimento a riga 66)
	  
	  NVIC_EnableIRQ(TIM14_IRQn);

	  TIM14->DIER |= TIM_DIER_UIE; //Update interrupt abilitato (p.471 reference manual)
	  TIM14->CR1 |= TIM_CR1_CEN;	/*ABILITO CONTATORE*/
	  
  //
}


// starts a timer
void Timer_start(struct Timer* timer){
  cli();								//inizio sezione critica (disattiva global interrupt)
  if (timer->timer_num==0)
    _timer0_start(timer);
  sei();								//fine sezione critica, riabilita interrupts
}


// stops a timer
void Timer_stop(struct Timer* timer){
  if (timer->timer_num!=0)
    return;
  cli();								//inizio sezione critica (disattiva global interrupt)
  //registro da cambiare
  TIMSK1 &= ~(1 << OCIE1A);	//disablita interrupt (vedi riga 78)
  sei();								//fine sezione critica, riabilita interrupts
}


ISR(TIMER1_COMPA_vect) {	//da sostituire con l'handler dell'interrupt del timer selezionato
//void TIM14_IRQHandler() {	//per ora ho messo TIMXX perché ancora non abbiamo definito quale timer usare
  TCNT1 = 0; //registro da cambiare, azzera flag di matching compare (p.185)
  if(timers[0].fn)
    (*timers[0].fn)(timers[0].args);
}

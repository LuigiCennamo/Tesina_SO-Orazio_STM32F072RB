#include "uart.h"
#include "buffer_utils.h"
#include <string.h>
#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"

void setBaud57600(void) {
	#define BAUD 576

	USART2->BRR = 480000 / BAUD;

	#ifdef USE_2X
	  USART1->CR1 |= USART_CR1_OVER8;
	#endif
	#undef BAUD
}

void setBaud115200(void) {
	#define BAUD 1152

	USART2->BRR = 480000 / BAUD;

	#ifdef USE_2X
		USART1->CR1 |= USART_CR1_OVER8;
	#endif
	#undef BAUD
}

#define UART_BUFFER_SIZE 256

typedef struct UART {
  int tx_buffer[UART_BUFFER_SIZE];
  volatile uint8_t tx_start;
  volatile uint8_t tx_end;
  volatile uint8_t tx_size;

  int rx_buffer[UART_BUFFER_SIZE];
  volatile uint8_t rx_start;
  volatile uint8_t rx_end;
  volatile uint8_t rx_size;
  
  int baud;
  int uart_num; // hardware uart;
} UART;

static UART uart_0;

struct UART* UART_init(const char* device __attribute__((unused)), uint32_t baud) {
  UART* uart=&uart_0;
  uart->uart_num=0;

  /* SETTAGGIO GPIOA PER UART2 */
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //abilita clock ahb
  GPIOA->AFR[0] |= 0x00001100; //imposta l'alternate function per il pin 3 e 2
  GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1; //seleziona modalità alternate function
  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	 //per il pin PA2 (USART2_TX) e PA3 (USART2_RX)
  	/*SETTAGGIO USART2*/
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;//abilitazione del bus sulla periferica
	RCC->CFGR3 &= 0xFFFDFFFF; //selezione di HSI, clock interno a 48 MHz come clock del bus

  switch(baud){
  case 57600: setBaud57600(); break;
  case 115200: setBaud115200(); break;
  default: return 0;
  }
  
  uart->tx_start=0;
  uart->tx_end=0;
  uart->tx_size=0;
  uart->rx_start=0;
  uart->rx_end=0;
  uart->rx_size=0;

  USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_RXNEIE;
  NVIC_EnableIRQ(USART2_IRQn);
  return &uart_0;
}

// returns the free space in the buffer
int UART_rxbufferSize(struct UART* uart __attribute__((unused))) {
  return UART_BUFFER_SIZE;
}
 
// returns the free occupied space in the buffer
int  UART_txBufferSize(struct UART* uart __attribute__((unused))) {
  return UART_BUFFER_SIZE;
}

// number of chars in rx buffer
int UART_rxBufferFull(UART* uart) {
  return uart->rx_size;
}

// number of chars in rx buffer
int UART_txBufferFull(UART* uart) {
  return uart->tx_size;
}

// number of free chars in tx buffer
int UART_txBufferFree(UART* uart){
  return UART_BUFFER_SIZE-uart->tx_size;
}

void UART_putChar(struct UART* uart, uint8_t c) {
  // loops until there is some space in the buffer
  while (uart->tx_size>=UART_BUFFER_SIZE);
  __disable_irq();
    uart->tx_buffer[uart->tx_end]=c;
    BUFFER_PUT(uart->tx, UART_BUFFER_SIZE);
    USART2->CR1 |= USART_CR1_TXEIE; // enable transmit interrupt
  __enable_irq();
}

uint8_t UART_getChar(struct UART* uart){
  while(uart->rx_size==0);
  uint8_t c;
  __disable_irq();
     c=uart->rx_buffer[uart->rx_start];
    BUFFER_GET(uart->rx, UART_BUFFER_SIZE);
  __enable_irq();

  return c;
}


void USART2_IRQHandler() {

	/*handler di ricezione*/
	if((USART2->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		uint8_t c=USART2->RDR;
		if (uart_0.rx_size<UART_BUFFER_SIZE){
			uart_0.rx_buffer[uart_0.rx_end] = c;
			BUFFER_PUT(uart_0.rx, UART_BUFFER_SIZE);
		}
	}
	else if((USART2->ISR & USART_ISR_TXE) == USART_ISR_TXE)
	{
		if (! uart_0.tx_size)
		{
			USART2->CR1 &= ~USART_CR1_TXEIE; // disable transmit interrupt
		}
		else
		{
		    USART2->TDR = uart_0.tx_buffer[uart_0.tx_start];
		    BUFFER_GET(uart_0.tx, UART_BUFFER_SIZE);
		}
	}
	else{}	// RESTITUISCI UNA QUALCHE CONDIZIONE DI ERRORE
}

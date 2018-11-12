#include "eeprom.h"
#include "delay.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

struct UART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

char* message="I am a string you stored in the EEPROM";

int main(void){

	//DEBUG SETTINGS

	RCC->APB2ENR |= RCC_APB2RSTR_DBGMCURST;		// ABILITAZIONE CLOCK LINEA DBGMCU
	DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM7_STOP;		// DISABILITO AGGIORNAMENTO DEL TIMER IN IDLE STATE

  EEPROM_init();
  uart=UART_init("uart_0",115200);
  // we write the string on eeprom at address 0
  int size=strlen(message)+1;
  EEPROM_write(0, message, size);
  char eeprom_buffer[size];

  while(1) {
    memset(eeprom_buffer, 0, size);
    // we read the string from the eeprom, in a buffer we clear each time
    EEPROM_read(eeprom_buffer, 0, size);
    eeprom_buffer[size-1]=0;

    // we wrap it in a nice thing
    char tx_message[128];
    sprintf(tx_message, "buffer string:[%s]\n", eeprom_buffer);
    printString(tx_message);
    delayMs(2000);
  }
}

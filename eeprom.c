#include "eeprom.h"
#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"
#include "stm32f0xx_flash.h"	//StdPeriph_Driver
#define OFFSET 0x0801F800 		//start address for page 63
/*
  Memory space available: 2 Kbyte (1 kbyte in EEPROM in AVR8)
*/

void EEPROM_init(void){
	FLASH_Unlock();
		FLASH_ErasePage(OFFSET);
	FLASH_Lock();
}

void EEPROM_read(void* dest_, const uint16_t src, uint16_t size){
  uint32_t s=(uint32_t)src+OFFSET;
  uint32_t s_end=src+OFFSET+(size*2);
  uint16_t* dest=(uint16_t*)dest_;
  //dest=(uint32_t*)dest_;
  while(s<s_end){
	assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));
	*dest=*(uint16_t*)s;
    s+=2;
    ++dest;
  }
}

void EEPROM_write(uint16_t dest, const void* src_,  uint16_t size){
  const uint16_t * s=src_;
  const uint16_t * s_end=s+size;
  FLASH_Unlock();
  while(s<s_end){
	FLASH_ProgramHalfWord((uint32_t)dest+OFFSET,*s);
    ++s;
    dest=dest+2;
  }
  FLASH_Lock();
}

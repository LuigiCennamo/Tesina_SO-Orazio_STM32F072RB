#include "eeprom.h"
//#include "avr/eeprom.h"
#include <stm32f0xx.h>
#include "stm32f0xx_nucleo.h"
#include "stm32f0xx_flash.h"
//#define OFFSET 0x08000000
uint32_t offset=0x08000000;

//penso OK
void EEPROM_init(void){
	const uint32_t dest_end=0x0801FFFF;
	FLASH_Status status = FLASH_COMPLETE;

	FLASH_Unlock();
	while(offset<dest_end){
	    //eeprom_busy_wait();
	    //eeprom_write_byte((uint8_t*)dest, *s);
		status=FLASH_ProgramHalfWord(offset,0xFFFF);
		if ((status!=FLASH_ERROR_WRP)&&(status!=FLASH_ERROR_PROGRAM))
				break;
	    ++offset;
	}
	FLASH_Lock();
}

//penso ok
void EEPROM_read(void* dest_, const uint16_t src, uint16_t size){
  uint32_t s=(uint32_t)src+offset;
  uint16_t s_end=src+size;
  uint32_t* dest;
  dest_=(uint32_t*)dest_;
  while(s<s_end){
    //eeprom_busy_wait();
    //*dest=eeprom_read_byte((uint8_t*)s);
	/* Check the parameters */
	assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));
	*dest=(uint32_t*)s;
    ++s;
    ++dest;
  }
}

//OK
void EEPROM_write(uint16_t dest, const void* src_,  uint16_t size){
  const uint16_t * s=src_;
  const uint16_t * s_end=s+size;
  FLASH_Unlock();
  while(s<s_end){
    //eeprom_busy_wait();
    //eeprom_write_byte((uint8_t*)dest, *s);
	FLASH_ProgramHalfWord((uint32_t)dest+offset,*s);
    ++s;
    ++dest;
  }
  FLASH_Lock();
}

#include <stdint.h>

typedef struct{
  volatile uint16_t* in_register;
  volatile uint16_t* out_register;
  volatile uint32_t* dir_register;
  uint8_t bit;

  // timer registers for PWM
  volatile uint16_t* ccm1_register;
  volatile uint32_t* dutyc_register;
  const uint16_t com_mask;
  volatile uint32_t* afr_register[2];

  // interrupt
}  Pin;

#define PINS_NUM 14
//#define PINS_PWM_NUM 2

extern const Pin pins[];


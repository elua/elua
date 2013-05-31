// CPU definition file for STM32F407ZG

#ifndef __CPU_STM32F407ZG_H__
#define __CPU_STM32F407ZG_H__

// Start from the definition of STM32F407VG, modify what's neeeded

#include "cpu_stm32f407vg.h"

#undef NUM_PIO
#define NUM_PIO               7
#undef NUM_ADC
#define NUM_ADC               24

#endif // #ifndef __CPU_STM32F407ZG_H__


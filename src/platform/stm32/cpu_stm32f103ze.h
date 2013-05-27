// CPU definition file for STM32F103ZE
// Use the STM32F103RE description as a base

#ifndef __CPU_STM32F103ZE_H__
#define __CPU_STM32F103ZE_H__

#include "cpu_stm32f103re.h"

// 21 ADCs instead of 16
#undef NUM_ADC
#define NUM_ADC               21

#endif // #ifndef __CPU_STM32F103ZE_H__


// eLua platform configuration

#ifndef __CPU_STR912FAW44_H__
#define __CPU_STR912FAW44_H__

#include "stacks.h"
#include "type.h"
#include "platform_ints.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               10
#define NUM_SPI               2
#define NUM_UART              3
#define NUM_PWM               4
#define NUM_ADC               8
#define NUM_CAN               1
#define NUM_I2C               2

// One timer is reserved for the system timer / vtimers
#define NUM_TIMER             3
#define NUM_PHYS_TIMER        4

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    10

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
u32 SCU_GetMCLKFreqValue();
#define CPU_FREQUENCY         ( SCU_GetMCLKFreqValue() * 1000 )

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     8

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_ORIGIN           0x40000000
#define SRAM_SIZE             0x18000
#define INTERNAL_RAM1_FIRST_FREE end
#define INTERNAL_RAM1_LAST_FREE  ( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

#define PLATFORM_CPU_CONSTANTS_INTS\
 _C( INT_GPIO_POSEDGE ),\
 _C( INT_GPIO_NEGEDGE ),\
 _C( INT_TMR_MATCH ),

#endif // #ifndef __CPU_STR912FAW44_H__


// eLua platform configuration

#ifndef __CPU_LPC1768_H__
#define __CPU_LPC1768_H__

#include "stacks.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               5
#define NUM_SPI               0
#define NUM_UART              4
#define NUM_PWM               6
#define NUM_ADC               8
#define NUM_CAN               2
#define NUM_TIMER             4

#define ADC_BIT_RESOLUTION    12

// CPU frequency (needed by the CPU module, 0 if not used)
u32 lpc17xx_get_cpu_frequency();
#define CPU_FREQUENCY         lpc17xx_get_cpu_frequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     32

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_ORIGIN           0x10000000
#define SRAM_SIZE             0x8000
#define SRAM2_ORIGIN          0x2007C000
#define SRAM2_SIZE            0x8000
#define INTERNAL_RAM1_FIRST_FREE  end
#define INTERNAL_RAM1_LAST_FREE   ( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )
#define INTERNAL_RAM2_FIRST_FREE  SRAM2_ORIGIN 
#define INTERNAL_RAM2_LAST_FREE   ( SRAM2_ORIGIN + SRAM2_SIZE - 1 )
 
#endif // #ifndef __CPU_LPC1768_H__


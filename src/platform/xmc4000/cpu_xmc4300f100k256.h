
#ifndef __CPU_XMC4300F100K256_H__
#define __CPU_XMC4300F100K256_H__

#include "stacks.h"
#include "DAVE.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               16
#define NUM_SPI               0
#define NUM_UART              1
#define NUM_TIMER             1
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0
#define NUM_DAC               2

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         OSCHP_GetFrequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 13, 16, 13, 7, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 14, 4 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define RAM_SIZE              0x10000
#define RAM_BASE              0x20000000

#define INTERNAL_RAM1_FIRST_FREE RAM_BASE
#define INTERNAL_RAM1_LAST_FREE  ( RAM_BASE + RAM_SIZE - STACK_SIZE_TOTAL - 1 )

#endif // #ifndef __CPU_XMC4300F100K256_H__

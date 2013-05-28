// LPC2888 CPU definition

#ifndef __CPU_LPC2888_H__
#define __CPU_LPC2888_H__

#include "stacks.h"
#include "target.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               8
#define NUM_SPI               0
#define NUM_UART              1
#define NUM_TIMER             2
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         Fcclk

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 32, 20, 4, 6, 12, 6, 4, 1 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_ORIGIN           0x00400000
#define SRAM_SIZE             0x10000
#define INTERNAL_RAM1_FIRST_FREE end
#define INTERNAL_RAM1_LAST_FREE  ( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

#endif // #ifndef __CPU_LPC2888_H__


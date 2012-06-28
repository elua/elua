// LPC2468 CPU definitions

#ifndef __LPC2468_H__
#define __LPC2468_H__

#include "target.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               5
#define NUM_SPI               0
#define NUM_UART              4
#define NUM_PWM               12
#define NUM_ADC               8
#define NUM_CAN               0
#define NUM_TIMER             4

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         Fcclk

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     32

#define SRAM_ORIGIN           0x40000000
#define SRAM_SIZE             0x10000 // [TODO]: make this 96k?

#define INTERNAL_FLASH_START_ADDRESS    0
#define INTERNAL_FLASH_SIZE             ( 512 * 1024 )

#endif // #ifndef __LPC2468_H__


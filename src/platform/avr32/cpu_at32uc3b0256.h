// AT32UC3B0256 CPU configuration

#ifndef __CPU_AT32UC3B0256_H__
#define __CPU_AT32UC3B0256_H__

#include "board.h"
#include "stacks.h"
#include "platform_ints.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               2
#define NUM_SPI               4
#define NUM_UART              2
#define NUM_TIMER             3
#define NUM_PWM               6
#define NUM_ADC               8
#define NUM_CAN               0

#define ADC_BIT_RESOLUTION    10

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         REQ_CPU_FREQ

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 32, 12 }
#define AVR32_NUM_GPIO        44

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define INTERNAL_RAM1_FIRST_FREE  end
#define INTERNAL_RAM1_LAST_FREE   ( AVR32_SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

#define PLATFORM_CPU_CONSTANTS_INTS\
 _C( INT_UART_RX ),\
 _C( INT_TMR_MATCH ),\
 _C( INT_GPIO_POSEDGE ),\
 _C( INT_GPIO_NEGEDGE ),

#endif // #ifndef __CPU_AT32UC3B0256_H__


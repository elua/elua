// AT91SAM7X256 CPU definition

#ifndef __CPU_AT91SAM7X256__
#define __CPU_AT91SAM7X256__

#include "stacks.h"
#include "platform_ints.h"
#include "board.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               2
#define NUM_SPI               0
#define NUM_UART              2
#define NUM_TIMER             3
#define NUM_PWM               4
#define NUM_ADC               0
#define NUM_CAN               0

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         BOARD_MCK

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     31

#define SRAM_ORIGIN           0x200000
#ifndef SRAM_SIZE
#define SRAM_SIZE             0x10000
#endif
#define INTERNAL_RAM1_FIRST_FREE end
#define INTERNAL_RAM1_LAST_FREE  ( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

#define PLATFORM_CPU_CONSTANTS_INTS\
 _C( INT_UART_RX ),

#endif // #ifndef __CPU_AT91SAM7X256__


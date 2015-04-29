// LPC2368 CPU definitions

#ifndef __CPU_LPC2368_H__
#define __CPU_LPC2368_H__

#include "stacks.h"
#include "target.h"
#include "platform_ints.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               16
#define NUM_SPI               1
#define NUM_UART              3
#define NUM_TIMER             4
#define NUM_PHYS_TIMER        0
#define NUM_PWM               6
#define NUM_ADC               6
#define NUM_CAN               2

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    10

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         Fcclk

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     32

// Internal RAM
#define SRAM_ORIGIN           0x40000000
#define SRAM_SIZE             ( 32 * 1024 ) // [TODO]: make this 96k?
#define INTERNAL_RAM1_FIRST_FREE         end
#define INTERNAL_RAM1_LAST_FREE          ( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

// Interrupt list for this CPU
#define PLATFORM_CPU_CONSTANTS_INTS\
  _C( INT_GPIO_POSEDGE ),     \
  _C( INT_GPIO_NEGEDGE ),     \
  _C( INT_TMR_MATCH ),        \
  _C( INT_UART_RX ),

#endif // #ifndef __CPU_LPC2368_H__


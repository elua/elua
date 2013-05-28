// AT32UC3A0128 CPU configuration

#ifndef __CPU_AT32UC3A0128_H__
#define __CPU_AT32UC3A0128_H__

#include "board.h"
#include "stacks.h"
#include "platform_ints.h"
#include "sdramc.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               4
#define NUM_SPI               6
#define NUM_UART              2
#define NUM_TIMER             3
#define NUM_PWM               6         // PWM7 is on GPIO50
#define NUM_I2C               1
#define NUM_ADC               8         // Though ADC3 pin is the Ethernet IRQ
#define NUM_CAN               0

#define ADC_BIT_RESOLUTION    10

#define CPU_FREQUENCY         REQ_CPU_FREQ

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 31, 32, 32, 14 }
#define AVR32_NUM_GPIO        110 // actually 109, but consider also PA31

#define RAM_SIZE 0x8000
#define INTERNAL_RAM1_FIRST_FREE  end
#define INTERNAL_RAM1_LAST_FREE   ( RAM_SIZE - STACK_SIZE_TOTAL - 1 )

#define PLATFORM_CPU_CONSTANTS_INTS\
 _C( INT_UART_RX ),\
 _C( INT_TMR_MATCH ),\
 _C( INT_GPIO_POSEDGE ),\
 _C( INT_GPIO_NEGEDGE ),

#endif // #ifndef __CPU_AT32UC3A0128_H__


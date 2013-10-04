// CPU definition file for STM32F205RF as used on the Netduino 2

#ifndef __CPU_STM32F205RF_H__
#define __CPU_STM32F205RF_H__

#include "type.h"
#include "stacks.h"
#include "stm32f2xx.h"
#include "platform_ints.h"

// Number of resources (0 if not available/not implemented)
#if defined( ELUA_BOARD_NETDUINO2 )
#define NUM_PIO               3
#define NUM_SPI               1
#define NUM_UART              4
#else
#define NUM_PIO               7
#define NUM_SPI               3
#define NUM_UART              6
#endif
#define NUM_TIMER             12
#define NUM_PHYS_TIMER        12
#define NUM_PWM               4
#define NUM_ADC               16
#define NUM_CAN               2

#define ADC_BIT_RESOLUTION    12

u32 platform_s_cpu_get_frequency();
#define CPU_FREQUENCY         platform_s_cpu_get_frequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     16

// Internal memory data
#define SRAM_SIZE                       ( 128 * 1024 )
#define INTERNAL_RAM1_FIRST_FREE         end
#define INTERNAL_RAM1_LAST_FREE          ( SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

#define INTERNAL_FLASH_SIZE             ( 768 * 1024 )
#define INTERNAL_FLASH_SECTOR_ARRAY     { 0x4000, 0x4000, 0x4000, 0x4000, 0x10000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000 }
#define INTERNAL_FLASH_START_ADDRESS    0x08000000

// Interrupt list for this CPU
#define PLATFORM_CPU_CONSTANTS_INTS\
  _C( INT_GPIO_POSEDGE ),     \
  _C( INT_GPIO_NEGEDGE ),     \
  _C( INT_TMR_MATCH ),        \
  _C( INT_UART_RX ),

#endif // #ifndef __CPU_STM32F205RF_H__


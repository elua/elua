// CPU definition file for STM32F103

#ifndef __STM32F103_H__
#define __STM32F103_H__

#include "type.h"

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               7
#define NUM_SPI               2
#define NUM_UART              5
#define NUM_TIMER             5
#define NUM_PHYS_TIMER        5
#define NUM_PWM               4
#define NUM_ADC               16
#define NUM_CAN               1

u32 platform_s_cpu_get_frequency();
#define CPU_FREQUENCY         platform_s_cpu_get_frequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     16

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_SIZE             ( 64 * 1024 )
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

#define INTERNAL_FLASH_SIZE             ( 512 * 1024 )
#define INTERNAL_FLASH_SECTOR_SIZE      2048
#define INTERNAL_FLASH_START_ADDRESS    0x08000000

#endif //#ifndef __STM32F103_H__


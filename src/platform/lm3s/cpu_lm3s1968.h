// LM3S1968 CPU definition

#ifndef __CPU_LM3S1968_H__
#define __CPU_LM3S1968_H__

#include "driverlib/sysctl.h"
#include "rom_map.h"
#include "stacks.h"
#include "hw_memmap.h"
#include "platform_ints.h"
#include "hw_ints.h"

#define NUM_PIO               8
#define NUM_SPI               2
#define NUM_UART              3
#define NUM_TIMER             4
#define NUM_PWM               6
#define NUM_ADC               8
#define NUM_CAN               0

#define ADC_BIT_RESOLUTION    10

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         MAP_SysCtlClockGet()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 8, 8, 8, 4, 4, 8, 8, 4 }

// Internal Flash data
#define INTERNAL_FLASH_SIZE             ( 256 * 1024 )
#define INTERNAL_FLASH_WRITE_UNIT_SIZE  4
#define INTERNAL_FLASH_SECTOR_SIZE      1024
#define INTERNAL_FLASH_START_ADDRESS    0

#define SRAM_SIZE ( 0x10000 )
// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define INTERNAL_RAM1_FIRST_FREE        end
#define INTERNAL_RAM1_LAST_FREE         ( SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1 )

#define PLATFORM_CPU_CONSTANTS_INTS\
  _C( INT_UART_RX ),

#endif // #ifndef __CPU_LM3S1968_H__


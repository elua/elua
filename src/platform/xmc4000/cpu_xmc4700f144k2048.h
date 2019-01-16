
#ifndef __CPU_XMC4700F144K2048_H__
#define __CPU_XMC4700F144K2048_H__

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
#define CPU_FREQUENCY         144000000

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 16, 16, 16, 16, 8, 12, 7, 0, 0, 0, 0, 0, 0, 0, 14, 12 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define DSRAM1_SIZE           ( 128 * 1024 )
#define DSRAM1_BASE           0x20000000
#define DSRAM2_SIZE           ( 128 * 1024 )
#define DSRAM2_BASE           0x20020000
#define PSRAM_SIZE            ( 96 * 1024 )
#define PSRAM_BASE            0x1FFE8000
#define INTERNAL_RAM1_FIRST_FREE end
#define INTERNAL_RAM1_LAST_FREE  ( DSRAM1_BASE + DSRAM1_SIZE - STACK_SIZE_TOTAL - 1 )
#define INTERNAL_RAM2_FIRST_FREE DSRAM2_BASE
#define INTERNAL_RAM2_LAST_FREE  ( DSRAM2_BASE + DSRAM2_SIZE - 1 )
#define INTERNAL_RAM3_FIRST_FREE PSRAM_BASE
#define INTERNAL_RAM3_LAST_FREE  ( PSRAM_BASE + PSRAM_SIZE - 1 )

// Internal Flash data: Organization: 2MBytes.
#define INTERNAL_FLASH_SIZE             ( 2048 * 1024 )
#define INTERNAL_FLASH_SECTOR_ARRAY     { 16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384, 131072, 262144, 262144, 262144, 262144, 262144, 262144, 262144 }
#define INTERNAL_FLASH_START_ADDRESS    0x08000000

#endif // #ifndef __CPU_XMC4700F144K2048_H__

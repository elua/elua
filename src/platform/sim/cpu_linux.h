// Linux "CPU" description for the eLua simulator

#ifndef __CPU_LINUX_H__
#define __CPU_LINUX_H__

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               0
#define NUM_SPI               0
#define NUM_UART              0
#define NUM_TIMER             0
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     0

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
extern void *memory_start_address;
extern void *memory_end_address;

// Default to 1M of memory if not specified
#ifndef SIM_MEM_SIZE
#define SIM_MEM_SIZE (1024 * 1024)
#endif

#define INTERNAL_RAM1_FIRST_FREE ( void* )memory_start_address
#define INTERNAL_RAM1_LAST_FREE  ( void* )memory_end_address

#endif


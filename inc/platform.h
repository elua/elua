// Platform-specific functions

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "devman.h"
#include "type.h"

// Error codes
enum
{
  PLATFORM_ERR,
  PLATFORM_OK
};

// Platform initialization
int platform_init();

// *****************************************************************************
// PIO subsection
// "Virtual ports": 16 ports (PA...PF), 32-bits each for a total of 512 I/O pins.
// They are coded within a single integer, where the high part encodes the port
// number, while the lower part encodes the pin number

typedef u32 pio_type;
typedef u32 pio_code;
#define PLATFORM_IO_PORTS                     16
#define PLATFORM_IO_PORTS_BITS                4
#define PLATFORM_IO_PINS                      32
#define PLATFORM_IO_PINS_BITS                 5
#define PLATFORM_IO_FULL_PORT_BIT             14
#define PLATFORM_IO_FULL_PORT_MASK            ( 1 << PLATFORM_IO_FULL_PORT_BIT )
#define PLATFORM_IO_ENCODE( port, pin, full ) ( ( ( port ) << PLATFORM_IO_PINS_BITS ) | ( pin ) | ( ( full ) ? PLATFORM_IO_FULL_PORT_MASK : 0 ) )
#define PLATFORM_IO_GET_PORT( code )          ( ( ( code ) >> PLATFORM_IO_PINS_BITS ) & ( ( 1 << PLATFORM_IO_PORTS_BITS ) - 1 ) )
#define PLATFORM_IO_GET_PIN( code )           ( ( code ) & ( ( 1 << PLATFORM_IO_PINS_BITS ) -1 ) )
#define PLATFORM_IO_IS_PORT( code )           ( ( ( code ) & PLATFORM_IO_FULL_PORT_MASK ) != 0 )
#define PLATFORM_IO_ALL_PINS                  0xFFFFFFFFUL
#define PLATFORM_IO_ENC_PORT                  1
#define PLATFORM_IO_ENC_PIN                   0

enum
{
  // Pin operations
  PLATFORM_IO_PIN_SET,
  PLATFORM_IO_PIN_CLEAR,
  PLATFORM_IO_PIN_GET,
  PLATFORM_IO_PIN_DIR_INPUT,
  PLATFORM_IO_PIN_DIR_OUTPUT,
  PLATFORM_IO_PIN_PULLUP,
  PLATFORM_IO_PIN_PULLDOWN,
  PLATFORM_IO_PIN_NOPULL,
  // Port operations
  PLATFORM_IO_PORT_SET_VALUE,
  PLATFORM_IO_PORT_GET_VALUE,
  PLATFORM_IO_PORT_DIR_INPUT,
  PLATFORM_IO_PORT_DIR_OUTPUT  
};

// The platform I/O functions
int platform_pio_has_port( unsigned port );
const char* platform_pio_get_prefix( unsigned port );
int platform_pio_has_pin( unsigned port, unsigned pin );
pio_type platform_pio_op( unsigned port, pio_type pinmask, int op );

// *****************************************************************************
// SPI subsection

// There are 4 "virtual" SPI ports (SPI0...SPI3).
#define PLATFORM_SPI_TOTAL                    4

// SPI mode
#define PLATFORM_SPI_MASTER                   1
#define PLATFORM_SPI_SLAVE                    0
// SS values
#define PLATFORM_SPI_SELECT_ON                1
#define PLATFORM_SPI_SELECT_OFF               0
// SPI enable/disable
#define PLATFORM_SPI_ENABLE                   1
#define PLATFORM_SPI_DISABLE                  0

// Data types
typedef u32 spi_data_type;

// The platform SPI functions
int platform_spi_exists( unsigned id );
u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits );
spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data );
void platform_spi_select( unsigned id, int is_select );

// *****************************************************************************
// UART subsection

// There are 4 "virtual" UART ports (UART0...UART3).
#define PLATFORM_UART_TOTAL                   4

// Parity
enum
{
  PLATFORM_UART_PARITY_EVEN,
  PLATFORM_UART_PARITY_ODD,
  PLATFORM_UART_PARITY_NONE
};

// Stop bits
enum
{
  PLATFORM_UART_STOPBITS_1,
  PLATFORM_UART_STOPBITS_1_5,
  PLATFORM_UART_STOPBITS_2
};

// "Infinite timeout" constant for recv
#define PLATFORM_UART_INFINITE_TIMEOUT        (-1)

// The platform UART functions
int platform_uart_exists( unsigned id );
u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits );
void platform_uart_send( unsigned id, u8 data );
int platform_uart_recv( unsigned id, unsigned timer_id, int timeout );

// *****************************************************************************
// Timer subsection

// There are 16 "virtual" timers (TMR0...TMR15)
#define PLATFORM_TIMER_TOTAL                  16

// Data types
typedef u32 timer_data_type;

// Timer operations
enum
{
  PLATFORM_TIMER_OP_START,
  PLATFORM_TIMER_OP_READ,
  PLATFORM_TIMER_OP_SET_CLOCK,
  PLATFORM_TIMER_OP_GET_CLOCK,
  PLATFORM_TIMER_OP_GET_MAX_DELAY,
  PLATFORM_TIMER_OP_GET_MIN_DELAY
};

// The platform timer functions
int platform_timer_exists( unsigned id );
void platform_timer_delay( unsigned id, u32 delay_us );
u32 platform_timer_op( unsigned id, int op, u32 data );
u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start );


// *****************************************************************************
// OLED DISP subsection


void platform_disp_init(unsigned long freq);

void platform_disp_clear(void);

void platform_disp_stringDraw(const char *str, unsigned long x, unsigned long y,
                              unsigned char level);
                              
void platform_disp_imageDraw(const unsigned char *img,
                             unsigned long x, unsigned long y,
                             unsigned long width, unsigned long height);
                             
void platform_disp_enable(unsigned long freq);

void platform_disp_disable(void);

void platform_disp_displayOn(void);

void platform_disp_displayOff(void);


 

// *****************************************************************************
// PWM subsection

// There are 16 "virtual" PWM channels (PWM0...PWM15)
#define PLATFORM_PWM_TOTAL                    16

// PWM operations
enum
{
  PLATFORM_PWM_OP_START,
  PLATFORM_PWM_OP_STOP,
  PLATFORM_PWM_OP_SET_CLOCK,
  PLATFORM_PWM_OP_GET_CLOCK
};

// The platform PWM functions
int platform_pwm_exists( unsigned id );
u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty );
u32 platform_pwm_op( unsigned id, int op, u32 data );

// *****************************************************************************
// CPU specific functions

void platform_cpu_enable_interrupts();
void platform_cpu_disable_interrupts();
u32 platform_cpu_get_frequency();

// *****************************************************************************
// Ethernet specific functions

void platform_eth_send_packet( const void* src, u32 size );
u32 platform_eth_get_packet_nb( void* buf, u32 maxlen );
void platform_eth_force_interrupt();
u32 platform_eth_get_elapsed_time();

// *****************************************************************************
// Allocator support

void* platform_get_first_free_ram( unsigned id );
void* platform_get_last_free_ram( unsigned id );

#endif

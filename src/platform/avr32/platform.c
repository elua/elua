// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include "stacks.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "utils.h"
#include "platform_conf.h"

// Platform-specific includes
#include <avr32/io.h>
#include "compiler.h"
#include "flashc.h"
#include "pm.h"
#include "board.h"
#include "usart.h"
#include "gpio.h"

// *****************************************************************************
// std functions

static void uart_send( int fd, char c )
{
  fd = fd;
  platform_uart_send( CON_UART_ID, c );
}

static int uart_recv()
{
  return platform_uart_recv( CON_UART_ID, 0, PLATFORM_UART_INFINITE_TIMEOUT );
}

// ****************************************************************************
// Platform initialization

extern int pm_configure_clocks( pm_freq_param_t *param );

int platform_init()
{
  pm_freq_param_t pm_freq_param =
  {
    REQ_CPU_FREQ,
    REQ_PBA_FREQ,
    FOSC0,
    OSC0_STARTUP, 
  };
   
  Disable_global_interrupt();  
  
  // Setup clocks
  if( PM_FREQ_STATUS_FAIL == pm_configure_clocks( &pm_freq_param ) )
    return PLATFORM_ERR;  
    
  // Setup UART for eLua
  platform_uart_setup( CON_UART_ID, CON_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );  
  
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );  
  
  // All done  
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO functions

int platform_pio_has_port( unsigned port )
{
  return 0;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%c", ( char )( port + 'A' ) );
  return c;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
  return port <= 1 && pin <= 30;
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  return 0;
}

// ****************************************************************************
// UART functions

#define NUM_UARTS     4

static const u32 uart_base_addr[ NUM_UARTS ] = { AVR32_USART0_ADDRESS, AVR32_USART1_ADDRESS, AVR32_USART2_ADDRESS, AVR32_USART3_ADDRESS };
static const gpio_map_t uart_pins = 
{
  // UART 0
  { AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION },
  { AVR32_USART0_TXD_0_0_PIN, AVR32_USART0_TXD_0_0_FUNCTION },
  // UART 1
  { AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION },
  { AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION },
  // UART 2
  { AVR32_USART2_RXD_0_0_PIN, AVR32_USART2_RXD_0_0_FUNCTION },
  { AVR32_USART2_TXD_0_0_PIN, AVR32_USART2_TXD_0_0_FUNCTION },
  // UART 3
  { AVR32_USART3_RXD_0_0_PIN, AVR32_USART3_RXD_0_0_FUNCTION },
  { AVR32_USART3_TXD_0_0_PIN, AVR32_USART3_TXD_0_0_FUNCTION }
};

// The platform UART functions
int platform_uart_exists( unsigned id )
{
  return id < NUM_UARTS;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];  
  usart_options_t opts;
  
  opts.channelmode = USART_NORMAL_CHMODE;
  opts.charlength = databits;
  opts.baudrate = baud;
  
  // Set stopbits
  if( stopbits == PLATFORM_UART_STOPBITS_1 )
    opts.stopbits = USART_1_STOPBIT;
  else if( stopbits == PLATFORM_UART_STOPBITS_1_5 )
    opts.stopbits = USART_1_5_STOPBITS;
  else
    opts.stopbits = USART_2_STOPBITS;    
    
  // Set parity
  if( parity == PLATFORM_UART_PARITY_EVEN )
    opts.paritytype = USART_EVEN_PARITY;
  else if( parity == PLATFORM_UART_PARITY_ODD )
    opts.paritytype = USART_ODD_PARITY;
  else
    opts.paritytype = USART_NO_PARITY;  
    
  // Set actual interface
  gpio_enable_module(uart_pins + id * 2, 2 );
  usart_init_rs232( pusart, &opts, REQ_PBA_FREQ );  
  
  // [TODO] Return actual baud here
  return baud;
}

void platform_uart_send( unsigned id, u8 data )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];  
  
  usart_putchar( pusart, data );
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ id ];  
  int temp;

  if( timeout == 0 )
  {
    if( usart_read_char( pusart, &temp ) != USART_SUCCESS )
      return -1;
    else
      return temp;
  }
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
  {
    return usart_getchar( pusart );
  }
  else
  {
    return -1;
  }  
}

// ****************************************************************************
// CPU functions

u32 platform_cpu_get_frequency()
{
  return REQ_CPU_FREQ;
}

// ****************************************************************************
// Allocator support
extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )end;
}

#define SRAM_ORIGIN 0x0
#define SRAM_SIZE 0x10000

void* platform_get_last_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 );
}

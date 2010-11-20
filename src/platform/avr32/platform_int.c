// AVR32 interrupt support

#include "platform_conf.h"
#if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

// Generic headers
#include "platform.h"
#include "elua_int.h"
#include "common.h"

// Platform includes
#include <avr32/io.h>
#include "usart.h"
#include "intc.h"

// ****************************************************************************
// Interrupt handlers

// ----------------------------------------------------------------------------
// UART_RX interrupt

#ifndef AVR32_USART2_IRQ
#define AVR32_USART2_IRQ      ( -1 )
#endif

#ifndef AVR32_USART3_IRQ
#define AVR32_USART3_IRQ      ( -1 )
#endif

extern const u32 uart_base_addr[];

static int usart_irqs[] = { AVR32_USART0_IRQ, AVR32_USART1_IRQ, AVR32_USART2_IRQ, AVR32_USART3_IRQ };

static void uart_common_rx_handler( int resnum )
{
  cmn_int_handler( INT_UART_RX, resnum );
}

__attribute__((__interrupt__)) static void uart0_rx_handler()
{
  uart_common_rx_handler( 0 );
}

__attribute__((__interrupt__)) static void uart1_rx_handler()
{
  uart_common_rx_handler( 1 );
}

__attribute__((__interrupt__)) static void uart2_rx_handler()
{
  uart_common_rx_handler( 2 );
}

__attribute__((__interrupt__)) static void uart3_rx_handler()
{
  uart_common_rx_handler( 3 );
}

// ****************************************************************************
// Interrupt: INT_UART_RX

static int int_uart_rx_get_status( elua_int_resnum resnum )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ resnum ];      
  
  return ( pusart->imr & AVR32_USART_IMR_RXRDY_MASK ) ? 1 : 0;  
}

static int int_uart_rx_set_status( elua_int_resnum resnum, int status )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ resnum ];
  int prev = int_uart_rx_get_status( resnum );
  
  if( status == PLATFORM_CPU_ENABLE )      
    pusart->ier = AVR32_USART_IER_RXRDY_MASK;
  else
    pusart->idr = AVR32_USART_IDR_RXRDY_MASK;   
  return prev;
}

static int int_uart_rx_get_flag( elua_int_resnum resnum, int clear )
{
  volatile avr32_usart_t *pusart = ( volatile avr32_usart_t* )uart_base_addr[ resnum ];
  
  ( void )clear; // the flag is automatically cleared after reading the UART char
  return ( pusart->csr & AVR32_USART_CSR_RXRDY_MASK ) ? 1 : 0;
}

// ****************************************************************************
// Interrupt initialization

typedef void ( *phandler )();
static phandler phandlers[] = { uart0_rx_handler, uart1_rx_handler, uart2_rx_handler, uart3_rx_handler };

void platform_int_init()
{
  unsigned i;
  
  for( i = 0; i < NUM_UART; i ++ )
   INTC_register_interrupt( phandlers[ i ], usart_irqs[ i ], AVR32_INTC_INT0 );   
  Enable_global_interrupt();   
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag }
};

#endif // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

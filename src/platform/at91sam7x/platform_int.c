// AVR32 interrupt support

#include "platform_conf.h"
#if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

// Generic headers
#include "platform.h"
#include "elua_int.h"
#include "common.h"

// Platform includes
#include "aic.h"
#include <board.h>

// "Stubs" used for our interrupt handlers
// Just a trick to avoid interworking and some other complications

#define INT_STUB( func )\
  asm volatile(\
  "push {lr}\n\t"\
  "bl   " #func "\n\t"\
  "pop  {r0}\n\t"\
  "bx   r0\n\t"\
 )\
 
// ****************************************************************************
// Interrupt handlers

// ----------------------------------------------------------------------------
// UART_RX interrupt

static const int usart_int_ids[] = { AT91C_ID_US0, AT91C_ID_US1 };
static AT91S_USART* const usart_bases[] = { AT91C_BASE_US0, AT91C_BASE_US1 };
 
static void uart_common_rx_handler( int resnum )
{
  cmn_int_handler( INT_UART_RX, resnum );
  AT91C_BASE_AIC->AIC_ICCR = 1 << usart_int_ids[ resnum ];
  AT91C_BASE_AIC->AIC_EOICR = 0;
}

void __uart0_rx_handler_helper()
{
  uart_common_rx_handler( 0 );
}

static void __attribute__((naked)) uart0_rx_handler()
{
  INT_STUB( __uart0_rx_handler_helper );
}

void __uart1_rx_handler_helper()
{
  uart_common_rx_handler( 1 );
}

static void __attribute__((naked)) uart1_rx_handler()
{
  INT_STUB( __uart1_rx_handler_helper );
}

// ****************************************************************************
// Interrupt: INT_UART_RX

static int int_uart_rx_get_status( elua_int_resnum resnum )
{
  volatile AT91S_USART *pusart = ( volatile AT91S_USART* )usart_bases[ resnum ];
  return ( pusart->US_IMR & AT91C_US_RXRDY ) ? 1 : 0;
}

static int int_uart_rx_set_status( elua_int_resnum resnum, int status )
{
  volatile AT91S_USART *pusart = ( volatile AT91S_USART* )usart_bases[ resnum ];
  int prev = int_uart_rx_get_status( resnum );
  
  if( status == PLATFORM_CPU_ENABLE )      
    pusart->US_IER = AT91C_US_RXRDY;
  else
    pusart->US_IDR = AT91C_US_RXRDY;    
  return prev;
}

static int int_uart_rx_get_flag( elua_int_resnum resnum, int clear )
{
  volatile AT91S_USART *pusart = ( volatile AT91S_USART* )usart_bases[ resnum ];

  ( void )clear; // Note: this interrupt will be cleared automatically when the RHR is read
  return( pusart->US_CSR & AT91C_US_RXRDY ) ? 1 : 0;    
}

// ****************************************************************************
// Interrupt initialization

typedef void ( *phandler )();
static phandler phandlers[] = { uart0_rx_handler, uart1_rx_handler };

void platform_int_init()
{
  unsigned i;
  
  for( i = 0; i < NUM_UART; i ++ )
  {
    AIC_ConfigureIT( usart_int_ids[ i ], 0, phandlers[ i ] ); 
    AIC_EnableIT( usart_int_ids[ i ] );
  }  
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_ints.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag }
};

#endif // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

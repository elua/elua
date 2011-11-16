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
#include "tc.h"

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

static const int usart_irqs[] = { AVR32_USART0_IRQ, AVR32_USART1_IRQ, AVR32_USART2_IRQ, AVR32_USART3_IRQ };

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

// ----------------------------------------------------------------------------
// TMR_MATCH interrupts

#ifndef VTMR_CH
  #if VTMR_NUM_TIMERS > 0 
  #define VTMR_CH     (2)
  #else // #if VTMR_NUM_TIMERS > 0 
  #define VTMR_CH     0xFFFF
  #endif // #if VTMR_NUM_TIMERS > 0 
#endif // #ifndef VTMR_CH

extern void platform_eth_timer_handler();
static const int tmr_irqs[] = { AVR32_TC_IRQ0, AVR32_TC_IRQ1, AVR32_TC_IRQ2 };
extern u8 avr32_timer_int_periodic_flag[ 3 ];

static void tmr_match_common_handler( int id )
{
  volatile avr32_tc_t *tc = &AVR32_TC;

  tc_read_sr( tc, id ); // clear interrupt
  if( id == VTMR_CH )
  {
    cmn_virtual_timer_cb();
    platform_eth_timer_handler();
  }
  else
    cmn_int_handler( INT_TMR_MATCH, id );
  if( avr32_timer_int_periodic_flag[ id ] != PLATFORM_TIMER_INT_CYCLIC )
  {
    tc->channel[ id ].IDR.cpcs = 1;
    tc->channel[ id ].CMR.waveform.wavsel = TC_WAVEFORM_SEL_UP_MODE;
  }
}

__attribute__((__interrupt__)) static void tmr0_int_handler()
{
  tmr_match_common_handler( 0 );
}

__attribute__((__interrupt__)) static void tmr1_int_handler()
{
  tmr_match_common_handler( 1 );
}

__attribute__((__interrupt__)) static void tmr2_int_handler()
{
  tmr_match_common_handler( 2 );
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
// Interrupt: INT_TMR_MATCH

static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  volatile avr32_tc_channel_t *pch = AVR32_TC.channel + resnum;

  return pch->IMR.cpcs;
}

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  volatile avr32_tc_channel_t *pch = AVR32_TC.channel + resnum;
  int previous = pch->IMR.cpcs;

  if( status == PLATFORM_CPU_ENABLE )
    pch->IER.cpcs = 1;
  else
    pch->IDR.cpcs = 1;
  return previous;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
  volatile avr32_tc_channel_t *pch = AVR32_TC.channel + resnum;

  ( void )clear; // reading the status register will automatically clear the interrupt flag
  return pch->SR.cpcs;
}

// ****************************************************************************
// Interrupt initialization

typedef void ( *phandler )();
static const phandler phandlers_usart[] = { uart0_rx_handler, uart1_rx_handler, uart2_rx_handler, uart3_rx_handler };
static const phandler phandlers_tmr[] = { tmr0_int_handler, tmr1_int_handler, tmr2_int_handler };

void platform_int_init()
{
  unsigned i;
  
  for( i = 0; i < NUM_UART; i ++ )
    if( usart_irqs[ i ] != -1 )
      INTC_register_interrupt( phandlers_usart[ i ], usart_irqs[ i ], AVR32_INTC_INT0 );   
  for( i = 0; i < sizeof( phandlers_tmr ) / sizeof( phandler ); i ++ )
    INTC_register_interrupt( phandlers_tmr[ i ], tmr_irqs[ i ], AVR32_INTC_INT0 );
  Enable_global_interrupt();   
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag }
};

#endif // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )


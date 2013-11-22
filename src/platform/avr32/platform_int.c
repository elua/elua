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
#include "gpio.h"

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

extern void platform_eth_timer_handler();
extern void platform_cdc_timer_handler();
static const int tmr_irqs[] = { AVR32_TC_IRQ0, AVR32_TC_IRQ1, AVR32_TC_IRQ2 };
extern u8 avr32_timer_int_periodic_flag[ 3 ];

static void tmr_match_common_handler( int id )
{
  volatile avr32_tc_t *tc = &AVR32_TC;

  tc_read_sr( tc, id ); // clear interrupt
#if VTMR_NUM_TIMERS > 0
  if( id == VTMR_CH )
  {
    cmn_virtual_timer_cb();
    platform_eth_timer_handler();
    platform_cdc_timer_handler();
  }
  else
#endif
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

// ----------------------------------------------------------------------------
// GPIO interrupts and helpers

// AVR32 has a special "pin change" interrupt mode. eLua doesn't have this 
// interrupt, so we'll enable it when both POSEDGE and NEGEDGE for a pin are
// enabled. To do this, we must keep the individual per-pin status of POSEDGE
// and NEGEDGE in the arrays below.

#define AVR32_NUM_PORTS       ( ( AVR32_NUM_GPIO + 31 ) >> 5 )
static u32 gpio_posedge_status[ AVR32_NUM_PORTS ];
static u32 gpio_negedge_status[ AVR32_NUM_PORTS ];

static void gpioh_set_interrupt( elua_int_resnum resnum )
{
  u8 port = resnum >> 5, pin = resnum & 0x1F;
  u32 posedge_mask = gpio_posedge_status[ port ] & ( 1 << pin );
  u32 negedge_mask = gpio_negedge_status[ port ] & ( 1 << pin );

  if( posedge_mask && negedge_mask )
    gpio_enable_pin_interrupt( resnum, GPIO_PIN_CHANGE );
  else if( posedge_mask )
    gpio_enable_pin_interrupt( resnum, GPIO_RISING_EDGE );
  else if( negedge_mask )
    gpio_enable_pin_interrupt( resnum, GPIO_FALLING_EDGE );
  else
    gpio_disable_pin_interrupt( resnum );
}

/* Now for the interrupt handlers ... a bit weird, but easy to follow. The manual
has this to say: "In every port there are four interrupt lines connected to the 
interrupt controller. Every eight interrupts in the port are ORed together to form 
an interrupt line.". So we have an IRQ for each group of 8 consecutive GPIO pins.
We could use a single interrupt handler and scan through all pins to see which one
generated the interrupt, but it would take a lot of time for 109 pins. Instead,
we use all the interrupts that the system can provide and we point them to a single
handler; with the proper arguments, it needs to scan only 8 GPIO pins. */

static void gpio_int_common_handler( int resnum )
{
  int i;

  for( i = 0; i < 8; i ++, resnum ++ )
    if( gpio_get_pin_interrupt_flag( resnum ) )
    {
      if( gpio_get_pin_value( resnum ) )
        cmn_int_handler( INT_GPIO_POSEDGE, resnum );
      else
        cmn_int_handler( INT_GPIO_NEGEDGE, resnum );
      gpio_clear_pin_interrupt_flag( resnum );
    }
}

#define DEFINE_GPIO_HANDLER( num )\
  __attribute__((__interrupt__)) static void gpio_irq##num##_handler ()\
  {\
    gpio_int_common_handler( num << 3 );\
  }

DEFINE_GPIO_HANDLER( 0 )
DEFINE_GPIO_HANDLER( 1 )
DEFINE_GPIO_HANDLER( 2 )
DEFINE_GPIO_HANDLER( 3 )
DEFINE_GPIO_HANDLER( 4 )
DEFINE_GPIO_HANDLER( 5 )
#if AVR32_NUM_GPIO > 44 // not an UC3B, asume 109 pins, thus 14 IRQs
DEFINE_GPIO_HANDLER( 6 )
DEFINE_GPIO_HANDLER( 7 )
DEFINE_GPIO_HANDLER( 8 )
DEFINE_GPIO_HANDLER( 9 )
DEFINE_GPIO_HANDLER( 10 )
DEFINE_GPIO_HANDLER( 11 )
DEFINE_GPIO_HANDLER( 12 )
DEFINE_GPIO_HANDLER( 13 )
#endif // #if AVR32_NUM_GPIO > 44

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
// Interrupt: INT_GPIO_POSEDGE

static int int_gpio_posedge_get_status( elua_int_resnum resnum )
{
  u8 port = resnum >> 5, pin = resnum & 0x1F;

  return ( gpio_posedge_status[ port ] & ( 1 << pin ) ) != 0;
}

static int int_gpio_posedge_set_status( elua_int_resnum resnum, int status )
{
  u8 port = resnum >> 5, pin = resnum & 0x1F;
  int prev = int_gpio_posedge_get_status( resnum );

  if( status == PLATFORM_CPU_ENABLE )
    gpio_posedge_status[ port ] |= 1 << pin;
  else
    gpio_posedge_status[ port ] &= ~( 1 << pin );
  gpioh_set_interrupt( resnum );
  return prev;
}

static int int_gpio_posedge_get_flag( elua_int_resnum resnum, int clear )
{
  if( !int_gpio_posedge_get_status( resnum ) )
    return 0;
  int status = gpio_get_pin_interrupt_flag( resnum );

  if( clear )
    gpio_clear_pin_interrupt_flag( resnum );
  return status;
}

// ****************************************************************************
// Interrupt: INT_GPIO_NEGEDGE

static int int_gpio_negedge_get_status( elua_int_resnum resnum )
{
  u8 port = resnum >> 5, pin = resnum & 0x1F;

  return ( gpio_negedge_status[ port ] & ( 1 << pin ) ) != 0;
}

static int int_gpio_negedge_set_status( elua_int_resnum resnum, int status )
{
  u8 port = resnum >> 5, pin = resnum & 0x1F;
  int prev = int_gpio_negedge_get_status( resnum );

  if( status == PLATFORM_CPU_ENABLE )
    gpio_negedge_status[ port ] |= 1 << pin;
  else
    gpio_negedge_status[ port ] &= ~( 1 << pin );
  gpioh_set_interrupt( resnum );
  return prev;
}

static int int_gpio_negedge_get_flag( elua_int_resnum resnum, int clear )
{
  if( !int_gpio_negedge_get_status( resnum ) )
    return 0;
  int status = gpio_get_pin_interrupt_flag( resnum );

  if( clear )
    gpio_clear_pin_interrupt_flag( resnum );
  return status;
}

// ****************************************************************************
// Interrupt initialization

typedef void ( *phandler )();
static const phandler phandlers_usart[] = { uart0_rx_handler, uart1_rx_handler, uart2_rx_handler, uart3_rx_handler };
static const phandler phandlers_tmr[] = { tmr0_int_handler, tmr1_int_handler, tmr2_int_handler };
#if AVR32_NUM_GPIO == 44 // UC3B - 44 pins
static const phandler phandlers_gpio[] = { gpio_irq0_handler, gpio_irq1_handler, gpio_irq2_handler, gpio_irq3_handler, gpio_irq4_handler, gpio_irq5_handler };
#else // UC3A - 109 pins
static const phandler phandlers_gpio[] = { gpio_irq0_handler, gpio_irq1_handler, gpio_irq2_handler, gpio_irq3_handler, gpio_irq4_handler, gpio_irq5_handler,
  gpio_irq6_handler, gpio_irq7_handler, gpio_irq8_handler, gpio_irq9_handler, gpio_irq10_handler, gpio_irq11_handler, gpio_irq12_handler, gpio_irq13_handler };
#endif

void platform_int_init()
{
  unsigned i;
  
  for( i = 0; i < NUM_UART; i ++ )
    if( usart_irqs[ i ] != -1 )
      INTC_register_interrupt( phandlers_usart[ i ], usart_irqs[ i ], AVR32_INTC_INT0 );   
  for( i = 0; i < sizeof( phandlers_tmr ) / sizeof( phandler ); i ++ )
    INTC_register_interrupt( phandlers_tmr[ i ], tmr_irqs[ i ], AVR32_INTC_INT0 );
  for( i = 0; i < sizeof( phandlers_gpio ) / sizeof( phandler ); i ++ )
    INTC_register_interrupt( phandlers_gpio[ i ], AVR32_GPIO_IRQ_0 + i, AVR32_INTC_INT0 );
  Enable_global_interrupt();   
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_ints.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag },
  { int_gpio_posedge_set_status, int_gpio_posedge_get_status, int_gpio_posedge_get_flag },
  { int_gpio_negedge_set_status, int_gpio_negedge_get_status, int_gpio_negedge_get_flag }
};

#endif // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )


// AVR32 interrupt support

#include "platform_conf.h"
#if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

// Generic headers
#include "platform.h"
#include "elua_int.h"
#include "common.h"

// Platform includes
#if defined( FORLM3S9B92 )
  #define TARGET_IS_TEMPEST_RB1

  #include "lm3s9b92.h"
#elif defined( FORLM3S9D92 )
  #define TARGET_IS_FIRESTORM_RA2

  #include "lm3s9d92.h"
#elif defined( FORLM3S8962 )
  #include "lm3s8962.h"
#elif defined( FORLM3S6965 )
  #include "lm3s6965.h"
#elif defined( FORLM3S6918 )
  #include "lm3s6918.h"
#endif

#include "rom.h"
#include "rom_map.h"
#include "hw_ints.h"
#include "hw_gpio.h"
#include "hw_timer.h"
#include "gpio.h"
#include "uart.h"
#include "interrupt.h"
#include "driverlib/timer.h"

#define GPIO_INT_POSEDGE_ENABLED        1
#define GPIO_INT_NEGEDGE_ENABLED        2
#define GPIO_INT_BOTH_ENABLED           ( GPIO_INT_POSEDGE_ENABLED | GPIO_INT_NEGEDGE_ENABLED )

// ****************************************************************************
// Interrupt handlers

extern const u32 uart_base[];
extern const u32 pio_base[];
static const int uart_int_mask = UART_INT_RX | UART_INT_RT;
static const u8 gpio_int_ids[] = { INT_GPIOA, INT_GPIOB, INT_GPIOC, INT_GPIOD, INT_GPIOE, INT_GPIOF,
                                   INT_GPIOG, INT_GPIOH, INT_GPIOJ };
extern const u32 timer_base[];
extern u8 lm3s_timer_int_periodic_flag[ NUM_TIMER ];
static const u8 timer_int_ids[] = { INT_TIMER0A, INT_TIMER1A, INT_TIMER2A, INT_TIMER3A };

// ----------------------------------------------------------------------------
// UART_RX interrupt

static void uart_common_rx_handler( int resnum )
{
  MAP_UARTIntClear( uart_base[ resnum ], uart_int_mask );
  while( MAP_UARTCharsAvail( uart_base[ resnum ] ) )  
    cmn_int_handler( INT_UART_RX, resnum );  
}

void uart0_handler()
{
  uart_common_rx_handler( 0 );
}

void uart1_handler()
{
  uart_common_rx_handler( 1 );
}

void uart2_handler()
{
  uart_common_rx_handler( 2 );
}

// ----------------------------------------------------------------------------
// GPIO interrupts (POSEDGE/NEGEDGE)

static void gpio_common_handler( int port )
{
  u32 base = pio_base[ port ];
  u8 pin, pinmask;
  u32 ibe = HWREG( base + GPIO_O_IBE );
  u32 iev = HWREG( base + GPIO_O_IEV );

  // Check each pin in turn
  for( pin = 0, pinmask = 1; pin < 8; pin ++, pinmask <<= 1 )
    if( HWREG( base + GPIO_O_MIS ) & pinmask ) // interrupt on pin
    {
      if( MAP_GPIOPinRead( base, pinmask ) && ( ( ibe & pinmask ) || ( iev & pinmask ) ) ) // high level and posedge interrupt enabled 
        cmn_int_handler( INT_GPIO_POSEDGE, PLATFORM_IO_ENCODE( port, pin, 0 ) );
      else if( ( ibe & pinmask ) || !( iev & pinmask ) ) // low level and negedge interrupt enabled
        cmn_int_handler( INT_GPIO_NEGEDGE, PLATFORM_IO_ENCODE( port, pin, 0 ) );
      HWREG( base + GPIO_O_ICR ) = pinmask;
    }
}

void gpioa_handler()
{
  gpio_common_handler( 0 );
}

void gpiob_handler()
{
  gpio_common_handler( 1 );
}

void gpioc_handler()
{
  gpio_common_handler( 2 );
}

void gpiod_handler()
{
  gpio_common_handler( 3 );
}

void gpioe_handler()
{
  gpio_common_handler( 4 );
}

void gpiof_handler()
{
  gpio_common_handler( 5 );
}

void gpiog_handler()
{
  gpio_common_handler( 6 );
}

void gpioh_handler()
{
  gpio_common_handler( 7 );
}

void gpioj_handler()
{
  gpio_common_handler( 8 );
}

// ----------------------------------------------------------------------------
// Timer interrupts

static void tmr_common_handler( elua_int_resnum id )
{
  u32 base = timer_base[ id ];

  MAP_TimerIntClear( base, TIMER_TIMA_TIMEOUT );
  if( lm3s_timer_int_periodic_flag[ id ] != PLATFORM_TIMER_INT_CYCLIC )
  {
    MAP_TimerIntDisable( base, TIMER_TIMA_TIMEOUT );
    MAP_TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
  }
  cmn_int_handler( INT_TMR_MATCH, id );
}

void tmr0_handler()
{
  tmr_common_handler( 0 );
}

void tmr1_handler()
{
  tmr_common_handler( 1 );
}

void tmr2_handler()
{
  tmr_common_handler( 2 );
}

void tmr3_handler()
{
  tmr_common_handler( 3 );
}

// ****************************************************************************
// Helpers

// Get GPIO interrupt status as a mask
static int inth_gpio_get_int_status( elua_int_resnum resnum )
{
  const u32 portbase = pio_base[ PLATFORM_IO_GET_PORT( resnum ) ];
  const u8 pinmask = 1 << PLATFORM_IO_GET_PIN( resnum );

  if( ( HWREG( portbase + GPIO_O_IM ) & pinmask ) == 0 )
    return 0;
  if( ( HWREG( portbase + GPIO_O_IBE ) & pinmask ) != 0 )
    return GPIO_INT_BOTH_ENABLED;
  else if( ( HWREG( portbase + GPIO_O_IEV ) & pinmask ) != 0 )
    return GPIO_INT_POSEDGE_ENABLED;
  else
    return GPIO_INT_NEGEDGE_ENABLED;
}

// ****************************************************************************
// Interrupt: INT_UART_RX

static int int_uart_rx_get_status( elua_int_resnum resnum )
{  
  return ( MAP_UARTIntStatus( uart_base[ resnum ], false ) & uart_int_mask ) == uart_int_mask ? 1 : 0;
}

static int int_uart_rx_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_uart_rx_get_status( resnum );
  
  if( status == PLATFORM_CPU_ENABLE )
    MAP_UARTIntEnable( uart_base[ resnum ], uart_int_mask );
  else
    MAP_UARTIntDisable( uart_base[ resnum ], uart_int_mask );  
  return prev;
}

static int int_uart_rx_get_flag( elua_int_resnum resnum, int clear )
{

  int flag = ( MAP_UARTIntStatus( uart_base[ resnum ], false ) & uart_int_mask ) == uart_int_mask ? 1 : 0;
  
  if( clear )
    MAP_UARTIntClear( uart_base[ resnum ], uart_int_mask ); 
  return flag;  
}

// ****************************************************************************
// Interrupt: INT_GPIO_POSEDGE

static int int_gpio_posedge_get_status( elua_int_resnum resnum )
{
  int port = PLATFORM_IO_GET_PORT( resnum ), pin = PLATFORM_IO_GET_PIN( resnum );
  unsigned long type;

  if( ( HWREG( pio_base[ port ] + GPIO_O_IM ) & ( 1 << pin ) ) == 0 )
    return 0;
  type = MAP_GPIOIntTypeGet( pio_base[ port ], pin );   
  return ( type == GPIO_RISING_EDGE || type == GPIO_BOTH_EDGES ) ? 1 : 0;
}

static int int_gpio_posedge_set_status( elua_int_resnum resnum, int status )
{
  unsigned long portbase = pio_base[ PLATFORM_IO_GET_PORT( resnum ) ];
  u8 pinmask = 1 << PLATFORM_IO_GET_PIN( resnum );
  int prev = int_gpio_posedge_get_status( resnum );
  int crtstat = inth_gpio_get_int_status( resnum );

  if( status == PLATFORM_CPU_ENABLE )
  {
    // If already configured for falling edge, set both edges
    // Otherwise set only posedge
    if( crtstat & GPIO_INT_NEGEDGE_ENABLED )
      HWREG( portbase + GPIO_O_IBE ) |= pinmask;
    else
      HWREG( portbase + GPIO_O_IEV ) |= pinmask;
    MAP_GPIOPinIntEnable( portbase, pinmask );
  }
  else
  {
    // If configured for both, enable only falling edge
    // Otherwise disable interrupts completely
    if( crtstat == GPIO_INT_BOTH_ENABLED )
    {
      HWREG( portbase + GPIO_O_IBE ) &= ( u8 )~pinmask;
      HWREG( portbase + GPIO_O_IEV ) &= ( u8 )~pinmask;
    }
    else if( crtstat == GPIO_INT_POSEDGE_ENABLED )
      MAP_GPIOPinIntDisable( portbase, pinmask );
  }
  return prev;
}

static int int_gpio_posedge_get_flag( elua_int_resnum resnum, int clear )
{
  unsigned long portbase = pio_base[ PLATFORM_IO_GET_PORT( resnum ) ];
  u8 pinmask = 1 << PLATFORM_IO_GET_PIN( resnum );

  if( MAP_GPIOPinRead( portbase, pinmask ) == 0 )
    return 0;
  if( MAP_GPIOPinIntStatus( portbase, true ) & pinmask )
  {
    if( clear )
      MAP_GPIOPinIntClear( portbase, pinmask );
    return 1;
  }
  return 0;
}

// ****************************************************************************
// Interrupt: INT_GPIO_NEGEDGE

static int int_gpio_negedge_get_status( elua_int_resnum resnum )
{
  int port = PLATFORM_IO_GET_PORT( resnum ), pin = PLATFORM_IO_GET_PIN( resnum );
  unsigned long type;

  if( ( HWREG( pio_base[ port ] + GPIO_O_IM ) & ( 1 << pin ) ) == 0 )
    return 0;
  type = MAP_GPIOIntTypeGet( pio_base[ port ], pin );   
  return ( type == GPIO_FALLING_EDGE || type == GPIO_BOTH_EDGES ) ? 1 : 0;
}

static int int_gpio_negedge_set_status( elua_int_resnum resnum, int status )
{
  unsigned long portbase = pio_base[ PLATFORM_IO_GET_PORT( resnum ) ];
  u8 pinmask = 1 << PLATFORM_IO_GET_PIN( resnum );
  int prev = int_gpio_posedge_get_status( resnum );
  int crtstat = inth_gpio_get_int_status( resnum );

  if( status == PLATFORM_CPU_ENABLE )
  {
    // If already configured for rising edge, set both edges
    // Otherwise set only negedge
    if( crtstat & GPIO_INT_POSEDGE_ENABLED )
      HWREG( portbase + GPIO_O_IBE ) |= pinmask;
    else
      HWREG( portbase + GPIO_O_IEV ) &= ( u8 )~pinmask;
    MAP_GPIOPinIntEnable( portbase, pinmask );
  }
  else
  {
    // If configured for both, enable only rising edge
    // Otherwise disable interrupts completely
    if( crtstat == GPIO_INT_BOTH_ENABLED )
    {
      HWREG( portbase + GPIO_O_IBE ) &= ( u8 )~pinmask;
      HWREG( portbase + GPIO_O_IEV ) |= pinmask;
    }
    else if( crtstat == GPIO_INT_NEGEDGE_ENABLED )
      MAP_GPIOPinIntDisable( portbase, pinmask );
  }
  return prev;
}

static int int_gpio_negedge_get_flag( elua_int_resnum resnum, int clear )
{
  unsigned long portbase = pio_base[ PLATFORM_IO_GET_PORT( resnum ) ];
  u8 pinmask = 1 << PLATFORM_IO_GET_PIN( resnum );

  if( MAP_GPIOPinRead( portbase, pinmask ) != 0 )
    return 0;
  if( MAP_GPIOPinIntStatus( portbase, true ) & pinmask )
  {
    if( clear )
      MAP_GPIOPinIntClear( portbase, pinmask );
    return 1;
  }
  return 0;
}

// ****************************************************************************
// Interrupt: INT_TMR_MATCH

#define tmr_is_enabled( base )  ( ( HWREG( base + TIMER_O_CTL ) & TIMER_CTL_TAEN ) != 0 )

static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  u32 base = timer_base[ resnum ];

  return ( tmr_is_enabled( base ) && ( HWREG( base + TIMER_O_IMR ) & TIMER_TIMA_TIMEOUT ) ) ? 1 : 0;
}

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_tmr_match_get_status( resnum );
  u32 base = timer_base[ resnum ];

  if( status == PLATFORM_CPU_ENABLE )
  {
    MAP_TimerEnable( base, TIMER_A );
    MAP_TimerIntEnable( base, TIMER_TIMA_TIMEOUT );
  }
  else
  {
    MAP_TimerIntDisable( base, TIMER_TIMA_TIMEOUT );
    MAP_TimerDisable( base, TIMER_A );
  }
  return prev;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
  u32 base = timer_base[ resnum ];
  int status = MAP_TimerIntStatus( base, true ) & TIMER_TIMA_TIMEOUT;

  if( clear )
    MAP_TimerIntClear( base, TIMER_TIMA_TIMEOUT );
  return status && tmr_is_enabled( base ) ? 1 : 0;
}

// ****************************************************************************
// Interrupt initialization

void platform_int_init()
{
  unsigned i;

  MAP_IntEnable( INT_UART0 );
  MAP_IntEnable( INT_UART1 );
  MAP_IntEnable( INT_UART2 );
  for( i = 0; i < sizeof( gpio_int_ids ) / sizeof( u8 ); i ++ )
    MAP_IntEnable( gpio_int_ids[ i ] ) ;
  for( i = 0; i < sizeof( timer_int_ids ) / sizeof( u8 ); i ++ )
    MAP_IntEnable( timer_int_ids[ i ] );
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag },
  { int_gpio_posedge_set_status, int_gpio_posedge_get_status, int_gpio_posedge_get_flag },
  { int_gpio_negedge_set_status, int_gpio_negedge_get_status, int_gpio_negedge_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag }
};

#else // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )

void gpioa_handler()
{
}

void gpiob_handler()
{
}

void gpioc_handler()
{
}

void gpiod_handler()
{
}

void gpioe_handler()
{
}

void gpiof_handler()
{
}

void gpiog_handler()
{

void gpioh_handler()
{

void gpioj_handler()
{
}

void tmr0_handler()
{
}

void tmr1_handler()
{
}

void tmr2_handler()
{
}

void tmr3_handler()
{
}

#endif // #if defined( BUILD_C_INT_HANDLERS ) || defined( BUILD_LUA_INT_HANDLERS )


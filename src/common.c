// Common platform functions

#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "genstd.h"
#include "common.h"
#include <stdio.h>

// *****************************************************************************
// std functions and platform initialization

static void uart_send( int fd, char c )
{
  fd = fd;
  platform_uart_send( CON_UART_ID, c );
}

static int uart_recv()
{
  return platform_uart_recv( CON_UART_ID, 0, PLATFORM_UART_INFINITE_TIMEOUT );
}

void cmn_platform_init()
{
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );  
}

// ****************************************************************************
// PIO functions

int platform_pio_has_port( unsigned port )
{
  return port < NUM_PIO;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%c", ( char )( port + PIO_PREFIX ) );
  return c;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
#if defined( PIO_PINS_PER_PORT )
  return port < NUM_PIO && pin < PIO_PINS_PER_PORT;
#elif defined( PIO_PIN_ARRAY )
  const u8 pio_port_pins[] = PIO_PIN_ARRAY;
  return port < NUM_PIO && pin < pio_port_pins[ port ];
#else
  #error "You must define either PIO_PINS_PER_PORT of PIO_PIN_ARRAY in platform_conf.h"
#endif
}
 
// ****************************************************************************
// UART functions

// The platform UART functions
int platform_uart_exists( unsigned id )
{
  return id < NUM_UART;
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  timer_data_type tmr_start, tmr_crt;
  int res;
  
  if( timeout == 0 )
    return platform_s_uart_recv( id, timer_id, timeout );
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
    return platform_s_uart_recv( id, timer_id, timeout );
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
    while( 1 )
    {
      if( ( res = platform_s_uart_recv( id, timer_id, 0 ) ) >= 0 )
        break;
      tmr_crt = platform_timer_op( timer_id, PLATFORM_TIMER_OP_READ, 0 );
      if( platform_timer_get_diff_us( timer_id, tmr_crt, tmr_start ) >= timeout )
        break;
    }
    return res;
  }
}

// ****************************************************************************
// Timers (and vtimers) functions

#if VTMR_NUM_TIMERS > 0
static volatile u32 vtmr_counters[ VTMR_NUM_TIMERS ];
static volatile s8 vtmr_reset_idx = -1;

// This should be called from the platform's timer interrupt at VTMR_FREQ_HZ
void cmn_virtual_timer_cb()
{
  unsigned i;

  for( i = 0; i < VTMR_NUM_TIMERS; i ++ )
    vtmr_counters[ i ] ++;  
  if( vtmr_reset_idx != -1 )
  {
    vtmr_counters[ vtmr_reset_idx ] = 0;
    vtmr_reset_idx = -1;
  }
}

static void vtmr_reset_timer( unsigned vid )
{
  unsigned id = VTMR_GET_ID( vid );

  vtmr_reset_idx = ( s8 )id;
  while( vtmr_reset_idx != -1 );  
}

static void vtmr_delay( unsigned vid, u32 delay_us )
{
  timer_data_type final;
  unsigned id = VTMR_GET_ID( vid );
  
  final = ( ( u64 )delay_us * VTMR_FREQ_HZ ) / 1000000;
  vtmr_reset_timer( vid );
  while( vtmr_counters[ id ] < final );  
}

#else // #if VTMR_NUM_TIMERS > 0

void cmn_virtual_timer_cb()
{
}
#endif // #if VTMR_NUM_TIMERS > 0

int platform_timer_exists( unsigned id )
{
#if VTMR_NUM_TIMERS > 0
  if( TIMER_IS_VIRTUAL( id ) )
    return TIMER_IS_VIRTUAL( id );
  else
#endif
    return id < NUM_TIMER;
}

void platform_timer_delay( unsigned id, u32 delay_us )
{
#if VTMR_NUM_TIMERS > 0
  if( TIMER_IS_VIRTUAL( id ) )
    vtmr_delay( id, delay_us );
  else
#endif
    platform_s_timer_delay( id, delay_us );
}
      
u32 platform_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  if( ( VTMR_NUM_TIMERS == 0 ) || ( !TIMER_IS_VIRTUAL( id ) ) )
    return platform_s_timer_op( id, op, data );
#if VTMR_NUM_TIMERS > 0
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      vtmr_reset_timer( id );
      res = 0;
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = vtmr_counters[ VTMR_GET_ID( id ) ];
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = VTMR_FREQ_HZ;
      break;      
  }
#endif
  return res;
}

u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start )
{
  timer_data_type temp;
  u32 freq;
    
  freq = platform_timer_op( id, PLATFORM_TIMER_OP_GET_CLOCK, 0 );
  if( start < end )
  {
    temp = end;
    end = start;
    start = temp;
  }
  return ( ( u64 )( start - end ) * 1000000 ) / freq;
}

// ****************************************************************************
// SPI functions

int platform_spi_exists( unsigned id )
{
  return id < NUM_SPI;
}

// ****************************************************************************
// PWM functions

int platform_pwm_exists( unsigned id )
{
  return id < NUM_PWM;
}

// ****************************************************************************
// CPU functions

u32 platform_cpu_get_frequency()
{
  return CPU_FREQUENCY;
}

// ****************************************************************************
// Allocator support

extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  void* mstart[] = MEM_START_ADDRESS;
  
  return id >= sizeof( mstart ) / sizeof( void* ) ? NULL : mstart[ id ];
}

void* platform_get_last_free_ram( unsigned id )
{
  void* mend[] = MEM_END_ADDRESS;
  
  return id >= sizeof( mend ) / sizeof( void* ) ? NULL : mend[ id ];
}

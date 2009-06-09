// Common code for all backends

#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "genstd.h"
#include "common.h"
#include "buf.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "math.h"
#include "elua_adc.h"
#include "term.h"
#include "xmodem.h"

// ****************************************************************************
// XMODEM support code

#ifdef BUILD_XMODEM

static void xmodem_send( u8 data )
{
  platform_uart_send( CON_UART_ID, data );
}

static int xmodem_recv( u32 timeout )
{
  return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, timeout );
}

#endif // #ifdef BUILD_XMODEM

// ****************************************************************************
// Terminal support code

#ifdef BUILD_TERM

#define TERM_TIMEOUT    100000 

static void term_out( u8 data )
{
  platform_uart_send( CON_UART_ID, data );
}

static int term_in( int mode )
{
  if( mode == TERM_INPUT_DONT_WAIT )
    return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, 0 );
  else
    return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, PLATFORM_UART_INFINITE_TIMEOUT );
}

static int term_translate( int data )
{
  int c;
  
  if( isprint( data ) )
    return data;
  else if( data == 0x1B ) // escape sequence
  {
    // If we don't get a second char, we got a simple "ESC", so return KC_ESC
    // If we get a second char it must be '[', the next one is relevant for us
    if( platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT ) == -1 )
      return KC_ESC;
    if( ( c = platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT ) ) == -1 )
      return KC_UNKNOWN;
    switch( c )
    {
      case 0x41:
        return KC_UP;
      case 0x42:
        return KC_DOWN;
      case 0x43:
        return KC_RIGHT;
      case 0x44:
        return KC_LEFT;               
    }
  }
  else if( data == 0x0D )
  {
    // CR/LF sequence, read the second char (LF) if applicable
    platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT );
    return KC_ENTER;
  }
  else
  {
    switch( data )
    {
      case 0x09:
        return KC_TAB;
      case 0x16:
        return KC_PAGEDOWN;
      case 0x15:
        return KC_PAGEUP;
      case 0x05:
        return KC_END;
      case 0x01:
        return KC_HOME;
      case 0x7F:
      case 0x08:
        return KC_BACKSPACE;
    }
  }
  return KC_UNKNOWN;
}

#endif // #ifdef BUILD_TERM


// *****************************************************************************
// std functions and platform initialization

static void uart_send( int fd, char c )
{
  fd = fd;
  platform_uart_send( CON_UART_ID, c );
}

static int uart_recv( s32 to )
{
  return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, to );
}

void cmn_platform_init()
{
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );  

#ifdef BUILD_XMODEM  
  // Initialize XMODEM
  xmodem_init( xmodem_send, xmodem_recv );    
#endif

#ifdef BUILD_TERM  
  // Initialize terminal
  term_init( TERM_LINES, TERM_COLS, term_out, term_in, term_translate );
#endif
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

// Helper function for buffers
static int cmn_recv_helper( unsigned id, s32 timeout )
{
#ifdef BUF_ENABLE_UART
  t_buf_data data;
  
  if( buf_is_enabled( BUF_ID_UART, id ) )
  {
    if( timeout == 0 )
    {
      if ( ( buf_read( BUF_ID_UART, id, &data ) ) == PLATFORM_UNDERFLOW )
        return -1;
    }
    else
    {
      while( ( buf_read( BUF_ID_UART, id, &data ) ) == PLATFORM_UNDERFLOW );
    }
    return ( int )data;
  }
  else
#endif
  return platform_s_uart_recv( id, timeout );
}

int platform_uart_recv( unsigned id, unsigned timer_id, s32 timeout )
{
  timer_data_type tmr_start, tmr_crt;
  int res;
  
  if( timeout == 0 )
    return cmn_recv_helper( id, timeout );
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
    return cmn_recv_helper( id, timeout );
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
    while( 1 )
    {
      if( ( res = cmn_recv_helper( id, 0 ) ) >= 0 )
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
// ADC functions



int platform_adc_exists( unsigned id )
{
  return id < NUM_ADC;
}

#ifdef BUILD_ADC

u32 platform_adc_op( unsigned id, int op, u32 data )
{  
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  u32 res = 0;

  switch( op )
  {
    case PLATFORM_ADC_GET_MAXVAL:
      res = pow( 2, ADC_BIT_RESOLUTION ) - 1;
      break;

    case PLATFORM_ADC_SET_SMOOTHING:
      res = adc_update_smoothing( id, ( u8 )intlog2( ( unsigned ) data ) );
      break;
      
    case PLATFORM_ADC_SET_BLOCKING:
      s->blocking = data;
      break;
      
    case PLATFORM_ADC_IS_DONE:
      res = ( s->op_pending == 0 );
      break;
    
    case PLATFORM_ADC_OP_SET_TIMER:
      if ( d->timer_id != data )
        d->running = 0;
      platform_adc_stop( id );
      d->timer_id = data;
      break;
    
    case PLATFORM_ADC_OP_SET_CLOCK:
      res = platform_adc_setclock( id, data );
      break;
      
    case PLATFORM_ADC_SET_FREERUNNING:
      s->freerunning = data;
      break;
  }
  return res;
}
#endif // #ifdef BUILD_ADC

// ****************************************************************************
// Allocator support

#define MIN_ALIGN         8
#define MIN_ALIGN_SHIFT   3

extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  void* mstart[] = MEM_START_ADDRESS;
  u32 p;

  if( id >= sizeof( mstart ) / sizeof( void* ) )
    return NULL;
  p = ( u32 )mstart[ id ];
  if( p & ( MIN_ALIGN - 1 ) )
    p = ( ( p >> MIN_ALIGN_SHIFT ) + 1 ) << MIN_ALIGN_SHIFT;
  return ( void* )p;
}

void* platform_get_last_free_ram( unsigned id )
{
  void* mend[] = MEM_END_ADDRESS;
  u32 p;

  if( id >= sizeof( mend ) / sizeof( void* ) )
    return NULL;
  p = ( u32 )mend[ id ];
  if( p & ( MIN_ALIGN - 1 ) )
    p = ( ( p >> MIN_ALIGN_SHIFT ) - 1 ) << MIN_ALIGN_SHIFT;
  return ( void* )p;
}

// ****************************************************************************
// Misc support
unsigned int intlog2( unsigned int v )
{
  unsigned r = 0;

  while (v >>= 1)
  {
    r++;
  }
  return r;
}


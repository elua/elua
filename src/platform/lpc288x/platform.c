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
#include "lpc288x.h"
#include "target.h"
#include "uart.h"
#include "utils.h"

// *****************************************************************************
// std functions

static void uart_send( int fd, char c )
{
  uart_write( c );
}

static int uart_recv()
{
  return uart_read();
}

// ****************************************************************************
// Platform initialization

int platform_init()
{
  // Initialize CPU
  lpc288x_init();
  
  // Initialize UART
  uart_init( 115200, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  
  // Initialize timers
  T0CTRL = 0;
  T1CTRL = 0;
  INT_REQ5 = ( 1 << 28 ) | ( 1 << 27 ) | ( 1 << 26 ) | ( 1 << 16 ) | 0x1;
  INT_REQ6 = ( 1 << 28 ) | ( 1 << 27 ) | ( 1 << 26 ) | ( 1 << 16 ) | 0x1;    
  
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );  
  
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO functions

// Array with register addresses
typedef volatile unsigned int* vu_ptr;

static const vu_ptr pio_m0s_regs[] = { &MODE0S_0, &MODE0S_1, &MODE0S_2, &MODE0S_3, &MODE0S_4, &MODE0S_5, &MODE0S_6, &MODE0S_7 };
static const vu_ptr pio_m0c_regs[] = { &MODE0C_0, &MODE0C_1, &MODE0C_2, &MODE0C_3, &MODE0C_4, &MODE0C_5, &MODE0C_6, &MODE0C_7 };
static const vu_ptr pio_m1s_regs[] = { &MODE1S_0, &MODE1S_1, &MODE1S_2, &MODE1S_3, &MODE1S_4, &MODE1S_5, &MODE1S_6, &MODE1S_7 };
static const vu_ptr pio_m1c_regs[] = { &MODE1C_0, &MODE1C_1, &MODE1C_2, &MODE1C_3, &MODE1C_4, &MODE1C_5, &MODE1C_6, &MODE1C_7 };
static const vu_ptr pio_m0_regs[] = { &MODE0_0, &MODE0_1, &MODE0_2, &MODE0_3, &MODE0_4, &MODE0_5, &MODE0_6, &MODE0_7 };
static const vu_ptr pio_m1_regs[] = { &MODE1_0, &MODE1_1, &MODE1_2, &MODE1_3, &MODE1_4, &MODE1_5, &MODE1_6, &MODE1_7 };
static const vu_ptr pio_pin_regs[] = { &PINS_0, &PINS_1, &PINS_2, &PINS_3, &PINS_4, &PINS_5, &PINS_6, &PINS_7 };

int platform_pio_has_port( unsigned port )
{
  return port < 8;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%d", port );
  return c;
}

// Maximum number of pins per port
static const unsigned pins_per_port[] = { 32, 20, 4, 6, 12, 6, 4, 1 };
int platform_pio_has_pin( unsigned port, unsigned pin )
{
  return port < 8 && pin < pins_per_port[ port ];
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 0;
  
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:
      *pio_m0_regs[ port ] = pinmask;        
      break;
      
    case PLATFORM_IO_PIN_SET:
      *pio_m0s_regs[ port ] = pinmask;    
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      *pio_m0c_regs[ port ] = pinmask;
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:
      *pio_m1_regs[ port ] = 0xFFFFFFFF;
      break;
      
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      *pio_m1s_regs[ port ] = pinmask;
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      *pio_m1_regs[ port ] = 0;
      *pio_m0_regs[ port ] = 0;
      break;
      
    case PLATFORM_IO_PIN_DIR_INPUT:
      *pio_m1c_regs[ port ] = pinmask;
      *pio_m0c_regs[ port ] = pinmask;
      break;
            
    case PLATFORM_IO_PORT_GET_VALUE:
      retval = *pio_pin_regs[ port ];
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = *pio_pin_regs[ port ] & pinmask ? 1 : 0;
      break;
  }
  return retval;  
}

// ****************************************************************************
// UART

int platform_uart_exists( unsigned id )
{
  return id < 1;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return uart_init( baud, databits, parity, stopbits );
}

void platform_uart_send( unsigned id, u8 data )
{
  uart_write( data );
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  timer_data_type tmr_start, tmr_crt;
  int res;
    
  if( timeout == 0 )
  {
    // Return data only if already available
    return uart_read_nb();
  }
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
  {
    // Wait for data
    return uart_read();
  }
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START,0 );
    while( 1 )
    {
      if( ( res = uart_read_nb() ) > 0 )
        break;
      tmr_crt = platform_timer_op( timer_id, PLATFORM_TIMER_OP_READ, 0 );
      if( platform_timer_get_diff_us( timer_id, tmr_crt, tmr_start ) >= timeout )
        break;
    }
    return res;    
  }  
}

// ****************************************************************************
// Timer

static const vu_ptr tmr_load[] = { &T0LOAD, &T1LOAD };
static const vu_ptr tmr_value[] = { &T0VALUE, &T1VALUE };
static const vu_ptr tmr_ctrl[] = { &T0CTRL, &T1CTRL };
static const vu_ptr tmr_clr[] = { &T0CLR, &T1CLR };
static const unsigned tmr_prescale[] = { 1, 16, 256, 1 };

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  return MAIN_CLOCK / tmr_prescale[ ( *tmr_ctrl[ id ] >> 2 ) & 0x03 ];
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  unsigned i, mini = 0;
  
  for( i = 0; i < 3; i ++ )
    if( ABSDIFF( clock, MAIN_CLOCK / tmr_prescale[ i ] ) < ABSDIFF( clock, MAIN_CLOCK / tmr_prescale[ mini ] ) )
      mini = i;
  *tmr_ctrl[ id ] = ( *tmr_ctrl[ id ] & ~0xB ) | ( mini << 2 );
  return MAIN_CLOCK / tmr_prescale[ mini ];
}
int platform_timer_exists( unsigned id )
{
  return id < 2;
}

void platform_timer_delay( unsigned id, u32 delay_us )
{
  u32 freq;
  u64 final;
  u32 mask = ( id == 0 ) ? ( 1 << 5 ) : ( 1 << 6 );
    
  freq = platform_timer_get_clock( id );
  final = ( ( u64 )delay_us * freq ) / 1000000;
  if( final > 0xFFFFFFFF )
    final = 0xFFFFFFFF;
  *tmr_ctrl[ id ] &= 0x7F;
  *tmr_load[ id ] = final;
  *tmr_clr[ id ] = 0;
  *tmr_ctrl[ id ] |= 0x80;
  while( ( INT_PENDING & mask ) == 0 );
}
      
u32 platform_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      *tmr_ctrl[ id ] &= 0x7F;
      *tmr_load[ id ] = 0xFFFFFFFF;
      *tmr_ctrl[ id ] |= 0x80;    
      res = 0xFFFFFFFF;
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = *tmr_value[ id ];
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;      
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = platform_timer_set_clock( id, data );
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = platform_timer_get_clock( id );
      break;
  }
  return res;
}

u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start )
{
  timer_data_type temp;
  u32 freq;
    
  freq = platform_timer_get_clock( id );
  if( start < end )
  {
    temp = end;
    end = start;
    start = temp;
  }
  return ( ( u64 )( start - end ) * 1000000 ) / freq;
}

// ****************************************************************************
// Platform data functions

const char* platform_pd_get_platform_name()
{
  return "LPC288x";
}

const char* platform_pd_get_cpu_name()
{
  return "LPC2888";
}

u32 platform_pd_get_cpu_frequency()
{
  return Fcclk;
}

// ****************************************************************************
// Allocator support

extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  if( id > 1 )
    return NULL;
  else
    return id == 0 ? ( void* )end : ( void* )SDRAM_BASE_ADDR;
}

#define SRAM_ORIGIN 0x00400000
#define SRAM_SIZE 0x10000

void* platform_get_last_free_ram( unsigned id )
{
  if( id > 1 )
    return NULL;
  else
    return id == 0 ? ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) : 
                     ( void* )( SDRAM_BASE_ADDR + SDRAM_SIZE - 1 );
}

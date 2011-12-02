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
#include "common.h"
#include "platform_conf.h"

// ****************************************************************************
// Platform initialization

int platform_init()
{
   MODE1C_2 = 2;
   MODE0S_2 = 2;
                    
  // Initialize CPU
  lpc288x_init();
  
  // Initialize timers
  T0CTRL = 0;
  T1CTRL = 0;
  INT_REQ5 = ( 1 << 28 ) | ( 1 << 27 ) | ( 1 << 26 ) | ( 1 << 16 ) | 0x1;
  INT_REQ6 = ( 1 << 28 ) | ( 1 << 27 ) | ( 1 << 26 ) | ( 1 << 16 ) | 0x1;    
  
  cmn_platform_init();
  
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

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  
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
      
    default:
      retval = 0;
      break;
  }
  return retval;  
}

// ****************************************************************************
// UART

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  ( void )id;
  return uart_init( baud, databits, parity, stopbits );
}

void platform_s_uart_send( unsigned id, u8 data )
{
  uart_write( data );
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  if( timeout == 0 )
  {
    // Return data only if already available
    return uart_read_nb();
  }
  return uart_read();
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
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

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
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
      
timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
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
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = platform_timer_set_clock( id, data );
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = platform_timer_get_clock( id );
      break;

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = 0xFFFFFFFF;
      break;
  }
  return res;
}


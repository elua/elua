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
#include "common.h"
#include "platform_conf.h"
#include "buf.h"

// Platform includes
#include "mk60n512vmd100.h"
#include "sysinit.h"
#include "uart.h"

extern int core_clk_mhz;

static void timers_init();

// ****************************************************************************
// Platform initialization

int platform_init()
{
  sysinit();
  timers_init();
  
  // Common platform initialization code
  cmn_platform_init();

  return PLATFORM_OK;
} 

// *****************************************************************************
// UART functions

static UART_MemMapPtr const puarts[ NUM_UART ] = { UART0_BASE_PTR, UART1_BASE_PTR, UART2_BASE_PTR, UART3_BASE_PTR, UART4_BASE_PTR, UART5_BASE_PTR };

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return baud;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  uart_putchar( puarts[ id ], data );
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  UART_MemMapPtr uart = puarts[ id ];
  
  if( timeout == 0 )
    return uart_getchar_present( uart ) ? uart_getchar( uart ) : -1;
  return uart_getchar( uart );
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// *****************************************************************************
// Timer operations

static void timers_init()
{
  SIM_MemMapPtr sim = SIM_BASE_PTR;  

  /* Timer Init */
  sim->SCGC3 |= SIM_SCGC3_FTM2_MASK; /* FTM2 clock enablement */
  FTM2_SC = 0u;
  FTM2_CNT = 0x0000U;
  FTM2_SC |= FTM_SC_PS(7);  
  FTM2_SC |= 8;
}

static u32 timer_get_clock( unsigned id )
{
  return platform_s_cpu_get_frequency() / 128;
}

static u32 timer_set_clock( unsigned id, u32 clock )
{
  return platform_s_cpu_get_frequency() / 128;
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  FTM2_MOD = timer_get_clock( id ) / delay_us;
  FTM2_SC &= ~FTM_SC_TOF_MASK; /* Clear Flag */
  FTM2_CNT = 0;    
  while( !( FTM2_SC & FTM_SC_TOF_MASK ) );      
}

u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  data = data;
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      FTM2_CNT = 0;
      break;

    case PLATFORM_TIMER_OP_READ:
      res = FTM2_CNT;
      break;

    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFF );
      break;

    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );    
      break;

    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = timer_set_clock( id, data );
      break;

    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = timer_get_clock( id );
      break;

  }
  return res;
}

int platform_s_timer_set_match_int( unsigned id, u32 period_us, int type )
{
  return PLATFORM_TIMER_INT_INVALID_ID;
}

// *****************************************************************************
// CPU specific functions
 
u32 platform_s_cpu_get_frequency()
{
  return core_clk_mhz * 1000000;
}

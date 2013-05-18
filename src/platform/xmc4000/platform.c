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

// Platform includes
#include "DAVE3.h"
#include "UART001.h"
#include "xmc_elua.h"

#define SYSTICKHZ             10
#define SYSTICKMS             ( 1000 / SYSTICKHZ )

#define SYSTM001_SYS_CORE_CLOCK  120U 
#define SYSTM001_SYSTICK_INTERVAL SYSTICKMS

// ****************************************************************************
// Platform initialization

extern const UART001_HandleType UART001_Handle0;

int platform_init()
{
  DAVE_Init();
  cmn_platform_init();

  // Setup system timer
  cmn_systimer_set_base_freq( SYSTM001_SYS_CORE_CLOCK * 1000000 );
  cmn_systimer_set_interrupt_freq( SYSTICKHZ );

  SysTick_Config((uint32_t)(SYSTM001_SYSTICK_INTERVAL * SYSTM001_SYS_CORE_CLOCK * 1000U));
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),10,0));

  return PLATFORM_OK;
}


// ****************************************************************************
// PIO
static PORT_Type* const pio_port[] = { PORT_0, PORT_1, PORT_2, PORT_3, PORT_4, PORT_5, PORT_6, PORT_14, PORT_15 };

static void platformh_setup_pins( unsigned port, pio_type pinmask, u8 mask )
{
  unsigned i;
  u8 shift;
  PORT_Type* base = pio_port[ port ];

  for( i = 0; i < platform_pio_get_num_pins( port ); i ++ )
    if( pinmask & ( 1 << i ) )
    {
      shift = ( i & 3 ) << 3;
      base->IOCRS[ i >> 2 ] = ( base->IOCRS[ i >> 2 ] & ~( 0xFF << shift ) ) | ( mask << shift );
    }
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  PORT_Type* base = pio_port[ port ];
  unsigned i;

  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:
      base->OUT = pinmask;
      break;

    case PLATFORM_IO_PIN_SET:
      base->OMR = pinmask;
      break;

    case PLATFORM_IO_PIN_CLEAR:
      base->OMR = pinmask << 16;
      break;

    case PLATFORM_IO_PORT_DIR_INPUT:
      base->IOCRS[ 0 ] = base->IOCRS[ 1 ] = 0;
      base->IOCRS[ 2 ] = base->IOCRS[ 3 ] = 0;
      break;

    case PLATFORM_IO_PIN_DIR_INPUT:
    case PLATFORM_IO_PIN_NOPULL:
      platformh_setup_pins( port, pinmask, 0 );
      break;

    case PLATFORM_IO_PORT_DIR_OUTPUT:
      base->IOCRS[ 0 ] = base->IOCRS[ 1 ] = 0x80808080;
      base->IOCRS[ 2 ] = base->IOCRS[ 3 ] = 0x80808080;
      break;

    case PLATFORM_IO_PIN_DIR_OUTPUT:
      platformh_setup_pins( port, pinmask, 0x80 );
      break;

    case PLATFORM_IO_PORT_GET_VALUE:
      retval = base->IN;
      break;

    case PLATFORM_IO_PIN_GET:
      retval = ( base->IN & pinmask ) ? 1 : 0;
      break;

    case PLATFORM_IO_PIN_PULLUP:
      platformh_setup_pins( port, pinmask, 0x10 );
      break;

    case PLATFORM_IO_PIN_PULLDOWN:
      platformh_setup_pins( port, pinmask, 0x08 );
      break;

    default:
      retval = 0;
      break;
  }
  return retval;
}

void SysTick_Handler( void )
{
  // Handle system timer call
  cmn_systimer_periodic();
}

u64 platform_timer_sys_raw_read()
{
  return SysTick->LOAD - SysTick->VAL;
}

void platform_timer_sys_disable_int()
{
  SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void platform_timer_sys_enable_int()
{
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

timer_data_type platform_timer_read_sys()
{
  return cmn_systimer_get();
}


timer_data_type platform_s_timer_op( unsigned id, int op,timer_data_type data )
{
  return 0;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  u16 temp = ( u16 )data;

  UART001_WriteDataMultiple( &UART001_Handle0, &temp, 1 );
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  return UART001_ReadChar( &UART001_Handle0, timeout != 0 );
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return 0;
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
}


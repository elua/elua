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
#include "dwt.h"

// Platform includes
#include "DAVE.h"

#if defined ( XMC4500_F144k1024 )
# include "XMC4500.h"
#endif

#if defined ( XMC4700_F144x2048 )
# include "XMC4700.h"
#endif

#if defined ( XMC4300_F100x256 )
# include "XMC4300.h"
#endif

// Peripheral includes
#include "xmc_dac.h"

// Handles uart receive
uint8_t recv_byte;

// ****************************************************************************
// Function references

extern void ebu_main( void );

// Cleanup later: This should in theory be in inc/platform.h
void dacs_init( void );

// ****************************************************************************
// Platform initialization

/*
 * Don't know why - the compiler complains that _init doesn't exist. I define the
 * symbol now so he stops nagging me.
 */
void _init (void)
{
}

/* Main platform initialization */
int platform_init()
{
  DAVE_Init();

  cmn_platform_init();

  /* Initialize the EBU if we're using the XMC45 SDRAM kit */
#if defined ( XMC4500_E144x1024 )
  ebu_main();
#endif

  /* DAC */
  dacs_init();

  return PLATFORM_OK;
}

// ****************************************************************************
// PIO

#if defined ( ELUA_CPU_XMC4700F144K2048 ) ||\
    defined ( ELUA_CPU_XMC4500F144K1024 ) ||\
    defined ( ELUA_CPU_XMC4500E144K1024 )

static XMC_GPIO_PORT_t *const pio_port[] =
{
  XMC_GPIO_PORT0,
  XMC_GPIO_PORT1,
  XMC_GPIO_PORT2,
  XMC_GPIO_PORT3,
  XMC_GPIO_PORT4,
  XMC_GPIO_PORT5,
  XMC_GPIO_PORT6,
  XMC_GPIO_PORT14,
  XMC_GPIO_PORT15
};

#elif defined ( ELUA_CPU_XMC4300F100K256 )

static XMC_GPIO_PORT_t *const pio_port[] =
{
  XMC_GPIO_PORT0,
  XMC_GPIO_PORT1,
  XMC_GPIO_PORT2,
  XMC_GPIO_PORT3,
  XMC_GPIO_PORT4,
  XMC_GPIO_PORT5,
  XMC_GPIO_PORT14,
  XMC_GPIO_PORT15
};

#endif

static void platformh_setup_pins( unsigned port, pio_type pinmask, u8 mask )
{
  unsigned i;
  u8 shift;
  XMC_GPIO_PORT_t *base = pio_port[ port ];

  for( i = 0; i < platform_pio_get_num_pins( port ); i ++ )
    if( pinmask & ( 1 << i ) )
    {
      shift = ( i & 3 ) << 3;
      base->IOCR[ i >> 2 ] = ( base->IOCR[ i >> 2 ] & ~( 0xFF << shift ) ) | ( mask << shift );
    }
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  XMC_GPIO_PORT_t *base = pio_port[ port ];

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
      base->IOCR[ 0 ] = base->IOCR[ 1 ] = 0;
      base->IOCR[ 2 ] = base->IOCR[ 3 ] = 0;
      break;

    case PLATFORM_IO_PIN_DIR_INPUT:
    case PLATFORM_IO_PIN_NOPULL:
      platformh_setup_pins( port, pinmask, 0 );
      break;

    case PLATFORM_IO_PORT_DIR_OUTPUT:
      base->IOCR[ 0 ] = base->IOCR[ 1 ] = 0x80808080;
      base->IOCR[ 2 ] = base->IOCR[ 3 ] = 0x80808080;
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

// ****************************************************************************
// Timer

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

// ****************************************************************************
// UART

void elua_uart_rx_callback( void )
{
  // Empty for now.
}

void elua_uart_tx_callback( void )
{
  // Empty for now.
}

void platform_s_uart_send( unsigned id, u8 data )
{
  // Internally invokes DMA for transmit
  UART_Transmit( &UART_0, &data, 1 );
  while( UART_0.runtime->tx_busy );
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  // Internally invokes DMA for receive
  UART_Receive( &UART_0, &recv_byte, 1 );
  while( UART_0.runtime->rx_busy );
  return recv_byte;
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return 0;
}

// ****************************************************************************
// DAC

void dacs_init( void )
{
  /* SCU: Enable the DAC peripheral */
  XMC_DAC_Enable( (XMC_DAC_t  *)(void *)DAC );
}


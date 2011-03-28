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
extern int core_clk_khz;
extern int periph_clk_khz;

static void uarts_init();
static void timers_init();
static void gpios_init();

// ****************************************************************************
// Platform initialization

int platform_init()
{
  sysinit();
  
  gpios_init();
  uarts_init();
  timers_init();
  
  // Common platform initialization code
  cmn_platform_init();

  return PLATFORM_OK;
} 

// *****************************************************************************
// PIO functions

static GPIO_MemMapPtr const pgpios[ NUM_PIO ] = { PTA_BASE_PTR, PTB_BASE_PTR, PTC_BASE_PTR, PTD_BASE_PTR, PTE_BASE_PTR };
static PORT_MemMapPtr const ports[ NUM_PIO ] = { PORTA_BASE_PTR, PORTB_BASE_PTR, PORTC_BASE_PTR, PORTD_BASE_PTR, PORTE_BASE_PTR };

void gpios_init()
{
  unsigned i, j;
  PORT_MemMapPtr port;
  
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;  
  SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
  // Set all pins to GPIO
  for( i = 0; i < NUM_PIO; i ++ )
  {
    port = ports[ i ];
    for( j = 0; j < 32; j ++ )
      PORT_PCR_REG( port, j ) |= PORT_PCR_MUX( 1 );
  }      
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  GPIO_MemMapPtr base = pgpios[ port ];

  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:
      GPIO_PDOR_REG( base ) = pinmask;
      break;

    case PLATFORM_IO_PIN_SET:
      GPIO_PSOR_REG( base ) = pinmask;
      break;

    case PLATFORM_IO_PIN_CLEAR:
      GPIO_PCOR_REG( base ) = pinmask;
      break;

    case PLATFORM_IO_PORT_DIR_INPUT:
      GPIO_PDDR_REG( base ) = 0;
      break;
      
    case PLATFORM_IO_PIN_DIR_INPUT:
      GPIO_PDDR_REG( base ) &= ~pinmask;
      break;

    case PLATFORM_IO_PORT_DIR_OUTPUT:
      GPIO_PDDR_REG( base ) = 0xFFFFFFFF;
      break;
      
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      GPIO_PDDR_REG( base ) |= pinmask;
      break;

    case PLATFORM_IO_PORT_GET_VALUE:
      retval = GPIO_PDIR_REG( base );
      break;

    case PLATFORM_IO_PIN_GET:
      retval = ( GPIO_PDIR_REG( base ) & pinmask ) ? 1 : 0;
      break;

    default:
      retval = 0;
      break;
  }
  return retval;
}

// *****************************************************************************
// UART functions

static UART_MemMapPtr const puarts[ NUM_UART ] = { UART0_BASE_PTR, UART1_BASE_PTR, UART2_BASE_PTR, UART3_BASE_PTR, UART4_BASE_PTR, UART5_BASE_PTR };
static const u8 uart_ports[ NUM_UART ] = { 3, 2, 3, 2, 4 };
static const u8 uart_tx_pins[ NUM_UART ] = { 6, 4, 3, 17, 8 };
static const u8 uart_rx_pins[ NUM_UART ] = { 7, 3, 2, 16, 9 }; 

// Initialize UART clocks
static void uarts_init()
{
  SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;
  SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
  SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;
  SIM_SCGC1 |= SIM_SCGC1_UART4_MASK;
  SIM_SCGC1 |= SIM_SCGC1_UART5_MASK;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  PORT_MemMapPtr uart_port = ports[ uart_ports[ id ] ]; 
  UART_MemMapPtr puart = puarts[ id ];
  u16 ubd;
  int sysclk = id == 0 || id == 1 ? core_clk_khz * 1000 : periph_clk_khz * 1000;
  
  // Setup pins first
  uart_port->PCR[ uart_tx_pins[ id ] ] = PORT_PCR_MUX( 0x03 );
  uart_port->PCR[ uart_rx_pins[ id ] ] = PORT_PCR_MUX( 0x03 );
      
  // Disable receiver and transmitter
  UART_C2_REG( puart ) &= ~( UART_C2_TE_MASK | UART_C2_RE_MASK );

  // Configure the UART for 8-bit mode, no parity
  UART_C1_REG( puart ) = 0;
  
  // Check parity
  if( parity != PLATFORM_UART_PARITY_NONE )
  {
    UART_C1_REG( puart ) |= UART_C1_PE_MASK;
    if( parity == PLATFORM_UART_PARITY_ODD )
      UART_C1_REG( puart ) |= UART_C1_PT_MASK;
  }
  
  // Check data bits
  if( databits == 9 )
    UART_C1_REG( puart ) |= UART_C1_M_MASK;
    
  // Calculate baud settings
  ubd = ( u16 )( sysclk / ( baud * 16 ) );   
  UART_BDH_REG( puart ) |= UART_BDH_SBR( ( ubd & 0x1F00 ) >> 8 );
  UART_BDL_REG( puart ) = ( u8 )( ubd & UART_BDL_SBR_MASK );

  // Enable receiver and transmitter
	UART_C2_REG( puart ) |= ( UART_C2_TE_MASK | UART_C2_RE_MASK );    
      
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
// On Kinetis these are implemented using the PITs, not the FTMs

static void timers_init()
{
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
  PIT_MCR_REG( PIT_BASE_PTR ) &= ~PIT_MCR_MDIS_MASK;
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  // Disable timer first
  PIT_TCTRL_REG( PIT_BASE_PTR, id ) &= ~PIT_TCTRL_TEN_MASK;
  // Clear interrupt flag
  PIT_TFLG_REG( PIT_BASE_PTR, id ) |= PIT_TFLG_TIF_MASK;
  // Compute timeout and load it into LDVAL
  PIT_LDVAL_REG( PIT_BASE_PTR, id ) = ( ( u64 )delay_us * periph_clk_khz ) / 1000;
  // Enable timer and wait for interrupt flag
  PIT_TCTRL_REG( PIT_BASE_PTR, id ) |= PIT_TCTRL_TEN_MASK;
  while( ( PIT_TFLG_REG( PIT_BASE_PTR, id ) & PIT_TFLG_TIF_MASK ) == 0 );  
}

u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  data = data;
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      PIT_TCTRL_REG( PIT_BASE_PTR, id ) &= ~PIT_TCTRL_TEN_MASK;
      PIT_LDVAL_REG( PIT_BASE_PTR, id ) = 0xFFFFFFFF;
      PIT_TCTRL_REG( PIT_BASE_PTR, id ) |= PIT_TCTRL_TEN_MASK;
      res = 0xFFFFFFFF;
      break;

    case PLATFORM_TIMER_OP_READ:
      res = PIT_CVAL_REG( PIT_BASE_PTR, id );
      break;

    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;

    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );    
      break;

    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:    
      res = periph_clk_khz * 1000;
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

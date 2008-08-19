// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "pio.h"
#include "usart.h"
#include "devman.h"
#include "pmc.h"
#include "genstd.h"
#include "tc.h"
#include "stacks.h"
#include "pwmc.h"
#include "board.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "utils.h"

// *****************************************************************************
// std functions

static void uart_send( int fd, char c )
{
  fd = fd;
  USART_Write( AT91C_BASE_US0, c, 0 );  
}

static int uart_recv()
{
  return USART_Read( AT91C_BASE_US0, 0 );
}

// ****************************************************************************
// Platform initialization

static const Pin platform_uart_pins[ 2 ][ 2 ] = 
{
  { PIN_USART0_RXD, PIN_USART0_TXD },
  { PIN_USART1_RXD, PIN_USART1_TXD }
};
static const AT91S_TC* timer_base[] = { AT91C_BASE_TC0, AT91C_BASE_TC1, AT91C_BASE_TC2 };

int platform_init()
{
  int i;
   
  unsigned int mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | 
      AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT | AT91C_US_CHMODE_NORMAL;
      
  // Enable the peripherals we use in the PMC
  PMC_EnablePeripheral( AT91C_ID_US0 );  
  PMC_EnablePeripheral( AT91C_ID_US1 );
  PMC_EnablePeripheral( AT91C_ID_PIOA );
  PMC_EnablePeripheral( AT91C_ID_PIOB );
  PMC_EnablePeripheral( AT91C_ID_TC0 );
  PMC_EnablePeripheral( AT91C_ID_TC1 );
  PMC_EnablePeripheral( AT91C_ID_TC2 );
  PMC_EnablePeripheral( AT91C_ID_PWMC );
  
  // Configure pins
  PIO_Configure(platform_uart_pins[ 0 ], PIO_LISTSIZE(platform_uart_pins[ 0 ]));
    
  // Configure the USART in the desired mode @115200 bauds
  USART_Configure(AT91C_BASE_US0, mode, 115200, BOARD_MCK);
  
  // Enable receiver & transmitter
  USART_SetTransmitterEnabled(AT91C_BASE_US0, 1);
  USART_SetReceiverEnabled(AT91C_BASE_US0, 1);  
  
  // Configure the timers
  AT91C_BASE_TCB->TCB_BMR = 7;
  for( i = 0; i < 3; i ++ )
    TC_Configure( ( AT91S_TC* )timer_base[ i ], AT91C_TC_CLKS_TIMER_DIV5_CLOCK | AT91C_TC_WAVE );
        
  // PWM setup (only the clocks are set at this point)
  PWMC_ConfigureClocks( BOARD_MCK, BOARD_MCK, BOARD_MCK );
  PWMC_ConfigureChannel( 0, AT91C_PWMC_CPRE_MCKA, 0, 0 );
  PWMC_ConfigureChannel( 1, AT91C_PWMC_CPRE_MCKA, 0, 0 );  
  PWMC_ConfigureChannel( 2, AT91C_PWMC_CPRE_MCKB, 0, 0 );
  PWMC_ConfigureChannel( 3, AT91C_PWMC_CPRE_MCKB, 0, 0 );    
  
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );
    
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO functions

static Pin pio_port_desc[] = 
{
  { 0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT },
  { 0, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT }
};

int platform_pio_has_port( unsigned port )
{
  return port <= 1;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%c", ( char )( port + 'A' ) );
  return c;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
  return port <= 1 && pin <= 30;
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  Pin* pin;
  pio_type retval = 0;
  
  pin = pio_port_desc + port;
  pin->mask = pinmask;
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:    
    case PLATFORM_IO_PIN_SET:
      PIO_Set( pin );
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      PIO_Clear( pin );
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      pin->mask = 0x7FFFFFFF;      
    case PLATFORM_IO_PIN_DIR_INPUT:
      pin->type = PIO_INPUT;
      PIO_Configure( pin, 1 );
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:      
      pin->mask = 0x7FFFFFFF;      
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      pin->type = PIO_OUTPUT_0;
      PIO_Configure( pin, 1 );
      break;      
            
    case PLATFORM_IO_PORT_GET_VALUE:
      pin->mask = 0x7FFFFFFF;
      retval = PIO_Get( pin );
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = PIO_Get( pin ) ? 1 : 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// UART functions

// The platform UART functions
int platform_uart_exists( unsigned id )
{
  return id <= 1;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  unsigned int mode;
  AT91S_USART* base = id == 0 ? AT91C_BASE_US0 : AT91C_BASE_US1;

  // Setup mode
  mode = AT91C_US_USMODE_NORMAL | AT91C_US_CHMODE_NORMAL | AT91C_US_CLKS_CLOCK;  
  switch( databits )
  {
    case 5:
      mode |= AT91C_US_CHRL_5_BITS;
      break;      
    case 6:
      mode |= AT91C_US_CHRL_6_BITS;
      break;      
    case 7:
      mode |= AT91C_US_CHRL_7_BITS;
      break;      
    case 8:
      mode |= AT91C_US_CHRL_8_BITS;
      break;
  }
  if( parity == PLATFORM_UART_PARITY_EVEN )
    mode |= AT91C_US_PAR_EVEN;
  else if( parity == PLATFORM_UART_PARITY_ODD )
    mode |= AT91C_US_PAR_ODD;
  else
    mode |= AT91C_US_PAR_NONE;
  if( stopbits == PLATFORM_UART_STOPBITS_1 )
    mode |= AT91C_US_NBSTOP_1_BIT;
  else if( stopbits == PLATFORM_UART_STOPBITS_1_5 )
    mode |= AT91C_US_NBSTOP_15_BIT;
  else
    mode |= AT91C_US_NBSTOP_2_BIT;
      
  // Configure pins
  PIO_Configure(platform_uart_pins[ id ], PIO_LISTSIZE(platform_uart_pins[ id ]));
    
  // Configure the USART in the desired mode @115200 bauds
  baud = USART_Configure( base, mode, baud, BOARD_MCK );
  
  // Enable receiver & transmitter
  USART_SetTransmitterEnabled(base, 1);
  USART_SetReceiverEnabled(base, 1);
  
  // Return actual baud    
  return baud;
}

void platform_uart_send( unsigned id, u8 data )
{
  AT91S_USART* base = id == 0 ? AT91C_BASE_US0 : AT91C_BASE_US1;  
  
  USART_Write( base, data, 0 );
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  AT91S_USART* base = id == 0 ? AT91C_BASE_US0 : AT91C_BASE_US1;  
  timer_data_type tmr_start, tmr_crt;
  int res;
    
  if( timeout == 0 )
  {
    // Return data only if already available
    if( USART_IsDataAvailable( base ) )
      return USART_Read( base, 0 );
    else
      return -1;
  }
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
  {
    // Wait for data
    return USART_Read( base, 0 );
  }
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START,0 );
    while( 1 )
    {
      if( USART_IsDataAvailable( base ) )
      {
        res = USART_Read( base, 0 );
        break;
      }
      else
        res = -1;
      tmr_crt = platform_timer_op( timer_id, PLATFORM_TIMER_OP_READ, 0 );
      if( platform_timer_get_diff_us( timer_id, tmr_crt, tmr_start ) >= timeout )
        break;
    }
    return res;    
  }
}

// ****************************************************************************
// Timer functions

static const unsigned clkdivs[] = { 2, 8, 32, 128, 1024 };

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  u32 div;
  
  div = timer_base[ id ]->TC_CMR & 7;
  if( div > 4 )
    div = 4;
  return BOARD_MCK / clkdivs[ div ];
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  unsigned i, mini = 0;
  
  for( i = 0; i < 5; i ++ )
    if( ABSDIFF( clock, BOARD_MCK / clkdivs[ i ] ) < ABSDIFF( clock, BOARD_MCK / clkdivs[ mini ] ) )
      mini = i;
  TC_Configure( ( AT91S_TC* )timer_base[ id ], mini | AT91C_TC_WAVE );  
  return BOARD_MCK / clkdivs[ mini ];
}

int platform_timer_exists( unsigned id )
{
  return id < 3;
}

void platform_timer_delay( unsigned id, u32 delay_us )
{
  AT91S_TC* base = ( AT91S_TC* )timer_base[ id ];  
  u32 freq;
  timer_data_type final;
  volatile int i;
    
  freq = platform_timer_get_clock( id );
  final = ( ( u64 )delay_us * freq ) / 1000000;
  if( final > 0xFFFF )
    final = 0xFFFF;
  TC_Start( base );  
  for( i = 0; i < 200; i ++ );
  while( base->TC_CV < final );  
}

u32 platform_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  AT91S_TC* base = ( AT91S_TC* )timer_base[ id ];
  volatile int i;
  
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      res = 0;
      TC_Start( base );
      for( i = 0; i < 200; i ++ );      
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = base->TC_CV;
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFF );
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
// PWMs

// PWM0, PWM1 -> they can modify CLKA and are statically assigned to CLKA
// PWM2, PWM3 -> they can modify CLKB and are statically assigned to CLKB

#define PLATFORM_NUM_PWMS               4

// PWM pins
static const Pin pwm_pins[] = { PIN_PWMC_PWM0, PIN_PWMC_PWM1, PIN_PWMC_PWM2, PIN_PWMC_PWM3 };

// Helper function: return the PWM clock
static u32 platform_pwm_get_clock( unsigned id )
{
  u32 cfg = AT91C_BASE_PWMC->PWMC_CH[ id ].PWMC_CMR;
  u16 clkdata;
  
  clkdata = cfg & 0x0F;
  if( clkdata < 11 )
    return BOARD_MCK / ( 1 << clkdata );
  else
  {
    // clka / clkb
    cfg = AT91C_BASE_PWMC->PWMC_MR;
    if( clkdata == 12 ) // clkb
      cfg >>= 16;
    clkdata = cfg & 0x0FFF;
    return BOARD_MCK / ( ( clkdata & 0xFF ) * ( 1 << ( clkdata >> 8 ) ) );
  }
}

// Helper function: set the PWM clock
static u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  if( id < 2 )
    PWMC_ConfigureClocks( clock, 0, BOARD_MCK );
  else
    PWMC_ConfigureClocks( 0, clock, BOARD_MCK );
  return platform_pwm_get_clock( id );
}

int platform_pwm_exists( unsigned id )
{
  return id < PLATFORM_NUM_PWMS; 
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk = platform_pwm_get_clock( id );
  u32 period;  

  // Compute period
  period = pwmclk / frequency;
  // Set the period
  PWMC_SetPeriod( id, period );
  // Set duty cycle
  PWMC_SetDutyCycle( id, ( period * duty ) / 100 );
  // Return actual frequency
  return pwmclk / period;
}

u32 platform_pwm_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  
  switch( op )
  {
    case PLATFORM_PWM_OP_SET_CLOCK:
      res = platform_pwm_set_clock( id, data );
      break;
      
    case PLATFORM_PWM_OP_GET_CLOCK:
      res = platform_pwm_get_clock( id );
      break;
      
    case PLATFORM_PWM_OP_START:
      PIO_Configure( pwm_pins + id, 1 );    
      PWMC_EnableChannel( id );
      break;
      
    case PLATFORM_PWM_OP_STOP:
      PWMC_DisableChannel( id );
      platform_pio_op( 1, 1 << ( 19 + id ), PLATFORM_IO_PIN_DIR_INPUT );
      break;
  }
  
  return res;
}

// ****************************************************************************
// Platform data functions

const char* platform_pd_get_platform_name()
{
  return "AT91SAM7X";
}

const char* platform_pd_get_cpu_name()
{
#ifdef at91sam7x256
  return "AT91SAM7X256";
#else
  return "AT91SAM7X512";
#endif
}

u32 platform_pd_get_cpu_frequency()
{
  return BOARD_MCK;
}

// ****************************************************************************
// Allocator support
extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )end;
}

#define SRAM_ORIGIN 0x200000
#ifdef at91sam7x256
  #define SRAM_SIZE 0x10000
#else
  #define SRAM_SIZE 0x20000
#endif

void* platform_get_last_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 );
}

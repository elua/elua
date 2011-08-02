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
#include "common.h"
#include "aic.h"
#include "platform_conf.h"
#include "buf.h"

// "Stubs" used for our interrupt handlers
// Just a trick to avoid interworking and some other complications

#define INT_STUB( func )\
  asm volatile(\
  "push {lr}\n\t"\
  "bl   " #func "\n\t"\
  "pop  {r0}\n\t"\
  "bx   r0\n\t"\
 )\

// ****************************************************************************
// Platform initialization

static const Pin platform_uart_pins[ 2 ][ 2 ] = 
{
  { PIN_USART0_RXD, PIN_USART0_TXD },
  { PIN_USART1_RXD, PIN_USART1_TXD }
};
static const AT91S_TC* timer_base[] = { AT91C_BASE_TC0, AT91C_BASE_TC1, AT91C_BASE_TC2 };

#if VTMR_NUM_TIMERS > 0
void __isr_tc2_helper()
{
  cmn_virtual_timer_cb();
  AT91C_BASE_TC2->TC_SR;
}

static void __attribute__((naked)) ISR_Tc2()
{
  INT_STUB( __isr_tc2_helper );
}
#endif

int platform_init()
{
  int i;
   
  // Enable the peripherals we use in the PMC
  PMC_EnablePeripheral( AT91C_ID_US0 );  
  PMC_EnablePeripheral( AT91C_ID_US1 );
  PMC_EnablePeripheral( AT91C_ID_PIOA );
  PMC_EnablePeripheral( AT91C_ID_PIOB );
  PMC_EnablePeripheral( AT91C_ID_TC0 );
  PMC_EnablePeripheral( AT91C_ID_TC1 );
  PMC_EnablePeripheral( AT91C_ID_TC2 );
  PMC_EnablePeripheral( AT91C_ID_PWMC );  
  
  // Configure the timers
  AT91C_BASE_TCB->TCB_BMR = 0x15;
  for( i = 0; i < 3; i ++ )
    TC_Configure( ( AT91S_TC* )timer_base[ i ], AT91C_TC_CLKS_TIMER_DIV5_CLOCK | AT91C_TC_WAVE );
        
  // PWM setup (only the clocks are set at this point)
  PWMC_ConfigureClocks( BOARD_MCK, BOARD_MCK, BOARD_MCK );
  PWMC_ConfigureChannel( 0, AT91C_PWMC_CPRE_MCKA, 0, 0 );
  PWMC_ConfigureChannel( 1, AT91C_PWMC_CPRE_MCKA, 0, 0 );  
  PWMC_ConfigureChannel( 2, AT91C_PWMC_CPRE_MCKB, 0, 0 );
  PWMC_ConfigureChannel( 3, AT91C_PWMC_CPRE_MCKB, 0, 0 );    
  for( i = 0; i < 4; i ++ )
  {
    PWMC_EnableChannel( i );
    PWMC_EnableChannelIt( i );
  }

  cmn_platform_init();
  
#if VTMR_NUM_TIMERS > 0
  // Virtual timer initialization
  TC_Configure( AT91C_BASE_TC2, AT91C_TC_CLKS_TIMER_DIV5_CLOCK | AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO );
  AT91C_BASE_TC2->TC_RC = ( BOARD_MCK / 1024 ) / VTMR_FREQ_HZ;
  AIC_DisableIT( AT91C_ID_TC2 );
  AIC_ConfigureIT( AT91C_ID_TC2, 0, ISR_Tc2 );
  AT91C_BASE_TC2->TC_IER = AT91C_TC_CPCS;
  AIC_EnableIT( AT91C_ID_TC2 );  
  TC_Start( AT91C_BASE_TC2 );
#endif
    
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO functions

static Pin pio_port_desc[] = 
{
  { 0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT },
  { 0, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT }
};

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  Pin* pin;
  pio_type retval = 1;
  
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
      pin->type = PIO_INPUT;
      retval = PIO_Get( pin );
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = PIO_Get( pin ) & pinmask ? 1 : 0;
      break;
      
    case PLATFORM_IO_PIN_PULLUP:
      pin->pio->PIO_PPUER = pinmask;
      break;
      
    case PLATFORM_IO_PIN_NOPULL:
      pin->pio->PIO_PPUDR = pinmask;
      break;
      
    default:
      retval = 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// UART functions

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

void platform_s_uart_send( unsigned id, u8 data )
{
  AT91S_USART* base = id == 0 ? AT91C_BASE_US0 : AT91C_BASE_US1;  
  
  USART_Write( base, data, 0 );
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  AT91S_USART* base = id == 0 ? AT91C_BASE_US0 : AT91C_BASE_US1;  
    
  if( timeout == 0 )
  {
    // Return data only if already available
    if( USART_IsDataAvailable( base ) )
      return USART_Read( base, 0 );
    else
      return -1;
  }
  return USART_Read( base, 0 );
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
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

void platform_s_timer_delay( unsigned id, u32 delay_us )
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

u32 platform_s_timer_op( unsigned id, int op, u32 data )
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

// ****************************************************************************
// PWMs

// PWM0, PWM1 -> they can modify CLKA and are statically assigned to CLKA
// PWM2, PWM3 -> they can modify CLKB and are statically assigned to CLKB

// PWM pins
static const Pin pwm_pins[] = { PIN_PWMC_PWM0, PIN_PWMC_PWM1, PIN_PWMC_PWM2, PIN_PWMC_PWM3 };

// Return the PWM clock
u32 platform_pwm_get_clock( unsigned id )
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

// Set the PWM clock
u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  if( id < 2 )
    PWMC_ConfigureClocks( clock, 0, BOARD_MCK );
  else
    PWMC_ConfigureClocks( 0, clock, BOARD_MCK );
  return platform_pwm_get_clock( id );
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk = platform_pwm_get_clock( id );
  u32 period;  
  volatile u32 dummy;
  
  // Compute period
  period = pwmclk / frequency;
  // Set the period
  dummy = AT91C_BASE_PWMC->PWMC_ISR;
  PWMC_SetPeriod( id, period );
  while( ( AT91C_BASE_PWMC->PWMC_ISR & ( 1 << id ) ) == 0 );
  // Set duty cycle
  PWMC_SetDutyCycle( id, ( period * duty ) / 100 );
  // Return actual frequency
  return pwmclk / period;
}

void platform_pwm_start( unsigned id )
{
  volatile u32 dummy;
  PIO_Configure( pwm_pins + id, 1 );    
  dummy = AT91C_BASE_PWMC->PWMC_ISR;
}

void platform_pwm_stop( unsigned id )
{
  platform_pio_op( 1, 1 << ( 19 + id ), PLATFORM_IO_PIN_DIR_INPUT );
}

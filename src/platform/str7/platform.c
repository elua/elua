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
#include "71x_lib.h"

// ****************************************************************************
// Platform initialization

// Clock initialization
static void clock_init()
{
  // Configure MCLK clock for the CPU, RCCU_DEFAULT = RCLK /1
  RCCU_MCLKConfig( RCCU_DEFAULT );
   
  // Configure the PLL1 ( * 20 , / 1 )
  RCCU_PLL1Config( RCCU_PLL1_Mul_20, RCCU_Div_1 );

  // Wait PLL to lock
  while(RCCU_FlagStatus( RCCU_PLL1_LOCK ) == RESET );

  // Select PLL1_Output as RCLK clock
  RCCU_RCLKSourceConfig( RCCU_PLL1_Output );  
  
  // Configure PCLK1 = RCLK / 2
  RCCU_PCLK1Config( RCCU_RCLK_2 );

  // Configure PCLK2 = RCLK / 4
  RCCU_PCLK2Config( RCCU_RCLK_4 );  
  
  // At this step the MCLK = 40MHz, PCLK1 = 20MHz and PCLK2 = 10MHz with an external oscilator equal to 4MHz  
}

int platform_init()
{
  // Initialize clocks
  clock_init();
  
  // Initialize Timer 0 for XMODEM
  platform_timer_op( 0, PLATFORM_TIMER_OP_SET_CLOCK, 39000 ); 
  
  cmn_platform_init();

  // If interrupts are needed, uncomment the line below
  // EIC->ICR |= 0x03;
      
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO functions

static const GPIO_TypeDef *gpio_periph[ NUM_PIO ] = { GPIO0, GPIO1 };

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  GPIO_TypeDef* base = ( GPIO_TypeDef* )gpio_periph[ port ];
  pio_type retval = 1;
  
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:    
      GPIO_WordWrite( base, ( u16 )pinmask );
      break;
      
    case PLATFORM_IO_PIN_SET:
      GPIO_BitWrite_Mask( base, ( u16 )pinmask, 1 );
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      GPIO_BitWrite_Mask( base, ( u16 )pinmask, 0 );
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:
      pinmask = 0xFFFF;     
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      GPIO_Config( base, ( u16 )pinmask, GPIO_OUT_PP );
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      pinmask = 0xFFFF;     
    case PLATFORM_IO_PIN_DIR_INPUT:
      GPIO_Config( base, ( u16 )pinmask, GPIO_IN_TRI_CMOS );
      break;    
            
    case PLATFORM_IO_PORT_GET_VALUE:
      retval = GPIO_WordRead( base );
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = GPIO_BitRead_Mask( base, ( u16 )pinmask );
      break;
      
    default:
      retval = 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// UART

static const u16 uart_rx_pins[ NUM_UART ] = { 0x0001 << 8, 0x0001 << 10, 0x0001 << 13, 0x0001 << 1 };
static const u16 uart_tx_pins[ NUM_UART ] = { 0x0001 << 9, 0x0001 << 11, 0x0001 << 14, 0x0001 << 0 };
static const UART_TypeDef *uart_periph[ NUM_UART ] = { UART0, UART1, UART2, UART3 };

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{      
  UART_TypeDef* pport = ( UART_TypeDef* )uart_periph[ id ];
  UARTParity_TypeDef u_parity;
  UARTStopBits_TypeDef u_stopbits;
  UARTMode_TypeDef u_mode;
  
  // Configure GPIO
  GPIO_Config( GPIO0, uart_tx_pins[ id ], GPIO_AF_PP );
  GPIO_Config( GPIO0, uart_rx_pins[ id ], GPIO_IN_TRI_CMOS );  
  
  UART_Init( pport );
  
  // Enable port
  UART_OnOffConfig( pport, ENABLE ); 
  
  //  Disable FIFOs, reset FIFOs, disable loopback     
  UART_FifoConfig( pport, ENABLE ); 
  UART_FifoReset( pport, UART_RxFIFO ); 
  UART_FifoReset( pport, UART_TxFIFO ); 
  UART_LoopBackConfig( pport, DISABLE );
  
  // Check parity first
  if( parity == PLATFORM_UART_PARITY_EVEN )
    u_parity = UART_EVEN_PARITY;
  else if( parity == PLATFORM_UART_PARITY_ODD )
    u_parity = UART_ODD_PARITY;
  else
    u_parity = UART_NO_PARITY;
    
  // Then stop bits
  if( stopbits == PLATFORM_UART_STOPBITS_1 )
    u_stopbits = UART_1_StopBits;    
  else if( stopbits == PLATFORM_UART_STOPBITS_1 )
    u_stopbits = UART_1_5_StopBits;    
  else
    u_stopbits = UART_2_StopBits;
    
  // And finally databits
  if( databits == 7 && u_parity != UART_NO_PARITY )
    u_mode = UARTM_7D_P;
  else if( databits == 8 && u_parity != UART_NO_PARITY )
    u_mode = UARTM_8D_P;
  else
    u_mode = UARTM_8D;
    
  // Configure port and enable Rx
  UART_Config( pport, baud, u_parity, u_stopbits, u_mode );
  UART_RxConfig( pport, ENABLE );   
  
  // Return actual baud
  // [TODO] COMPUTE actual baud...
  return baud;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  UART_TypeDef* pport = ( UART_TypeDef* )uart_periph[ id ];
  
  UART_ByteSend( pport, &data );
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  UART_TypeDef* pport = ( UART_TypeDef* )uart_periph[ id ];    
  
  if( timeout == 0 )
  {
    // Return data only if already available
    if( UART_FlagStatus( pport ) & UART_RxBufNotEmpty )
      return UART_ByteReceive( pport );
    else
      return -1;
  }
  return UART_ByteReceive( pport );
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// ****************************************************************************
// Timer

#define NUM_TIMERS      4
            
static const TIM_TypeDef *tim_periph[ NUM_TIMER ] = { TIM0, TIM1, TIM2, TIM3 };

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  TIM_TypeDef* ptimer = ( TIM_TypeDef* )tim_periph[ id ];
  
  return RCCU_FrequencyValue( RCCU_PCLK2 ) / ( ( int )TIM_PrescalerValue( ptimer ) + 1 );
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  TIM_TypeDef* ptimer = ( TIM_TypeDef* )tim_periph[ id ];
  u32 baseclk = RCCU_FrequencyValue( RCCU_PCLK2 );
  u64 bestdiv;
  
  bestdiv = ( ( u64 )baseclk << 16 ) / clock;
  if( bestdiv & 0x8000 )
    bestdiv += 0x10000;
  bestdiv >>= 16;
  if( bestdiv > 256 )
    bestdiv = 256;
  TIM_PrescalerConfig( ptimer, ( u8 )( bestdiv - 1 ) );
  return baseclk / bestdiv;
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
  TIM_TypeDef* ptimer = ( TIM_TypeDef* )tim_periph[ id ];  
  u32 freq;
  timer_data_type final;
  
  freq = platform_timer_get_clock( id );
  final = ( ( u64 )delay_us * freq ) / 1000000;
  if( final > 2 )
    final -= 2;
  else
    final = 0;
  if( final > 0xFFFF )
    final = 0xFFFF;
  TIM_CounterConfig( ptimer, TIM_STOP );
  TIM_CounterConfig( ptimer, TIM_CLEAR );  
  TIM_CounterConfig( ptimer, TIM_START );  
  while( TIM_CounterValue( ptimer ) >= 0xFFFC );
  while( TIM_CounterValue( ptimer ) < final );  
}
      
timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
{
  u32 res = 0;
  TIM_TypeDef* ptimer = ( TIM_TypeDef* )tim_periph[ id ];  
  u8 pre;
  
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      pre = TIM_PrescalerValue( ptimer );
      TIM_Init( ptimer );
      TIM_PrescalerConfig( ptimer, pre );
      TIM_CounterConfig( ptimer, TIM_STOP );
      TIM_CounterConfig( ptimer, TIM_CLEAR );  
      TIM_CounterConfig( ptimer, TIM_START );  
      while( TIM_CounterValue( ptimer ) >= 0xFFFC );        
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = TIM_CounterValue( ptimer );
      break;
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = platform_timer_set_clock( id, data );
      break;
      
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = platform_timer_get_clock( id );
      break;

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = 0xFFFF;
      break;
  }
  return res;
}

// ****************************************************************************
// PWM functions

static const u16 pwm_pins[ NUM_PWM ] = { 1 << 7, 1 << 13, 1 << 2 };
static const u8 pwm_ports[ NUM_PWM ] = { 1, 0, 1 };
 
u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk = platform_timer_get_clock( id + 1 );
  u32 period;
  GPIO_TypeDef *pport = ( GPIO_TypeDef* )gpio_periph[ pwm_ports[ id ] ];
  TIM_TypeDef *ptimer = ( TIM_TypeDef* )tim_periph[ id + 1 ];
  
  // Set pin as PWM
  GPIO_Config( pport, pwm_pins[ id ], GPIO_AF_PP );
  // Compute period
  period = pwmclk / frequency;
  if( period > 0xFFFF )
    period = 0xFFFF;
  // Set the period and duty cycle
  TIM_PWMOModeConfig( ptimer, ( duty * period ) / 100, TIM_HIGH, period, TIM_LOW );
  // Return actual frequency
  return pwmclk / period;
}

u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  return platform_timer_set_clock( id + 1, clock );
}

u32 platform_pwm_get_clock( unsigned id )
{
  return platform_timer_get_clock( id + 1 );
}

void platform_pwm_start( unsigned id )
{
  TIM_TypeDef *ptimer = ( TIM_TypeDef* )tim_periph[ id + 1 ];

  TIM_CounterConfig( ptimer, TIM_START );
}

void platform_pwm_stop( unsigned id )
{
  platform_pio_op( pwm_ports[ id ], pwm_pins[ id ], PLATFORM_IO_PIN_DIR_INPUT );
}

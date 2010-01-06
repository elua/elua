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
#include "lpc17xx_gpio.h"
#include "lpc17xx_libcfg.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"

// ****************************************************************************
// Platform initialization

static void platform_setup_timers();
static void platform_setup_pwm();

int platform_init()
{
  // Set up microcontroller system and SystemCoreClock variable
  SystemInit();
  
  // DeInit NVIC and SCBNVIC
	NVIC_DeInit();
	NVIC_SCBDeInit();
	
  // Configure the NVIC Preemption Priority Bits:
  // two (2) bits of preemption priority, six (6) bits of sub-priority.
  // Since the Number of Bits used for Priority Levels is five (5), so the
  // actual bit number of sub-priority is three (3)
	NVIC_SetPriorityGrouping(0x05);
	
  	//  Set Vector table offset value
  #if (__RAM_MODE__==1)
  	NVIC_SetVTOR(0x10000000);
  #else
  	NVIC_SetVTOR(0x00000000);
  #endif

  // Setup peripherals
  platform_setup_timers();
  //platform_setup_pwm();

  // Initialize console UART
  platform_uart_setup( CON_UART_ID, CON_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );

  // Common platform initialization code
  cmn_platform_init();

  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO section

// The platform I/O functions
pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
   
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:   
      GPIO_SetValue(port, pinmask);
      break;
      
    case PLATFORM_IO_PIN_SET:
      GPIO_SetValue(port, pinmask);
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      GPIO_ClearValue(port, pinmask);
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:
      GPIO_SetDir(port, 0xFFFFFFFF, 1);
      break;    

    case PLATFORM_IO_PIN_DIR_OUTPUT:
      GPIO_SetDir(port, pinmask, 1);
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      GPIO_SetDir(port, 0xFFFFFFFF, 0);
      break;

    case PLATFORM_IO_PIN_DIR_INPUT:
      GPIO_SetDir(port, pinmask, 0);
      break;    
            
    case PLATFORM_IO_PORT_GET_VALUE:
      retval = GPIO_ReadValue(port);
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = ( GPIO_ReadValue(port) & pinmask ) ? 1 : 0;
      break;
      
    default:
      retval = 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// UART section

// UART0: Rx = P0.3, Tx = P0.2
// The other UARTs have assignable Rx/Tx pins and thus have to be configured
// by the user

static const UART_TypeDef *uart[] = { UART0, UART1, UART2, UART3 };

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 temp;
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfg;
	
	// UART0 Pin Config
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);
	
  UARTConfigStruct.Baud_rate = ( uint32_t )baud;
  
  switch( databits )
  {
    case 5:
      UARTConfigStruct.Databits = UART_DATABIT_5;
      break;

    case 6:
      UARTConfigStruct.Databits = UART_DATABIT_6;
      break;

    case 7:
      UARTConfigStruct.Databits = UART_DATABIT_7;
      break;

    case 8:
      UARTConfigStruct.Databits = UART_DATABIT_8;
      break;
  }
  
  if( stopbits == PLATFORM_UART_STOPBITS_2 )
    UARTConfigStruct.Stopbits = UART_STOPBIT_2;
  else
    UARTConfigStruct.Stopbits = UART_STOPBIT_1;
  
  switch( parity )
  {
    case PLATFORM_UART_PARITY_NONE:
      UARTConfigStruct.Parity = UART_PARITY_NONE;
      break;
    
    case PLATFORM_UART_PARITY_ODD:
      UARTConfigStruct.Parity = UART_PARITY_ODD;
      break;
    
    case PLATFORM_UART_PARITY_EVEN:
      UARTConfigStruct.Parity = UART_PARITY_EVEN;
      break;
  }
	
	UART_Init(uart[ id ], &UARTConfigStruct);
	
	// Get default FIFO config and initialize
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(uart[ id ], &UARTFIFOConfigStruct);
	
	// Enable Transmit
	UART_TxCmd(uart[ id ], ENABLE);
	
  return baud; // FIXME: find a way to actually get baud
}

void platform_uart_send( unsigned id, u8 data )
{
  UART_Send(uart[ id ], &data, 1, BLOCKING);
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  u8 buffer;
  
  if( timeout == 0 )
  {
    if ( UART_Receive(uart[ id ], &buffer, 1, NONE_BLOCKING) == 0 )
      return -1;
    else
      return ( int )buffer;
  }
  
  UART_Receive(uart[ id ], &buffer, 1, BLOCKING);
  return ( int )buffer;
}

// ****************************************************************************
// Timer section

static const TIM_TypeDef *tmr[] = { TIM0, TIM1, TIM2, TIM3 };
static const u32 tmr_pclk[] = { CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_TIMER1, CLKPWR_PCLKSEL_TIMER2, CLKPWR_PCLKSEL_TIMER3 };

// Helper function: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  return CLKPWR_GetPCLK( tmr_pclk[ id ] ) / ( tmr[ id ]->PR + 1 );
}

// Helper function: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  TIM_TIMERCFG_Type TIM_ConfigStruct;

  TIM_Cmd( tmr[ id ], DISABLE );

  // Initialize timer 0, prescale count time of 1uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1000000ULL / clock;
	
  TIM_Init( tmr[ id ], TIM_TIMER_MODE, &TIM_ConfigStruct );
	TIM_Cmd( tmr[ id ], ENABLE );
	TIM_ResetCounter( tmr[ id ] );
  
  return clock;
}

// Helper function: setup timers
static void platform_setup_timers()
{
  unsigned i;
  
  // Power on clocks on APB1
  CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, ENABLE);
  CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM3, ENABLE);
  
  // Set base frequency to 1MHz, as we can't use a better resolution anyway
  for( i = 0; i < 4; i ++ )
  {
    platform_timer_set_clock( i, 1000000ULL );
  }
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  u32 last;

  last = ( ( u64 )delay_us * platform_timer_get_clock( id ) ) / 1000000;
  TIM_Cmd( tmr[ id ], ENABLE );
  TIM_ResetCounter( tmr[ id ] );
  while( tmr[ id ]->TC < last );
}
      
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      TIM_Cmd( tmr[ id ], ENABLE );
      TIM_ResetCounter( tmr[ id ] );
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = tmr[ id ]->TC;
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

// ****************************************************************************
// CPU functions

void platform_cpu_enable_interrupts()
{
  __enable_irq();  
}

void platform_cpu_disable_interrupts()
{
  __disable_irq();
}
/*
// ****************************************************************************
// PWM functions

static const u32 pwm_tcr[] = { ( u32 )&PWM0TCR, ( u32 )&PWM1TCR };
static const u32 pwm_pr[] = { ( u32 )&PWM0PR, ( u32 )&PWM1PR };
static const u32 pwm_pc[] = { ( u32 )&PWM0PC, ( u32 )&PWM1PC };
static const u32 pwm_pcr[] = { ( u32 )&PWM0PCR, ( u32 )&PWM1PCR };
static const u32 pwm_mcr[] = { ( u32 )&PWM0MCR, ( u32 )&PWM1MCR };
static const u32 pwm_ler[] = { ( u32 )&PWM0LER, ( u32 )&PWM1LER };
static const u32 pwm_channels[ 2 ][ 6 ] = 
{
  { ( u32 )&PWM0MR1, ( u32 )&PWM0MR2, ( u32 )&PWM0MR3, ( u32 )&PWM0MR4, ( u32 )&PWM0MR5, ( u32 )&PWM0MR6 },
  { ( u32 )&PWM1MR1, ( u32 )&PWM1MR2, ( u32 )&PWM1MR3, ( u32 )&PWM1MR4, ( u32 )&PWM1MR5, ( u32 )&PWM1MR6 }, 
};

// Timer register definitions
enum
{
  PWM_ENABLE = 1,
  PWM_RESET = 2,
  PWM_MODE = 8,
  PWM_ENABLE_1 = 1 << 9,
  PWM_ENABLE_2 = 1 << 10,
  PWM_ENABLE_3 = 1 << 11,
  PWM_ENABLE_4 = 1 << 12,
  PWM_ENABLE_5 = 1 << 13,
  PWM_ENABLE_6 = 1 << 14,
};

// Helper function: get timer clock
static u32 platform_pwm_get_clock( unsigned id )
{
  unsigned pwmid = id / 6;
  PREG PWMxPR = ( PREG )pwm_pr[ pwmid ];

  return Fpclk / ( *PWMxPR + 1 );
}

// Helper function: set timer clock
static u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  u32 div = Fpclk / clock, prevtc;
  unsigned pwmid = id / 6;
  PREG PWMxPR = ( PREG )pwm_pr[ pwmid ];  
  PREG PWMxPC = ( PREG )pwm_pc[ pwmid ];
  PREG PWMxTCR = ( PREG )pwm_tcr[ pwmid ]; 
  
  prevtc = *PWMxTCR;
  *PWMxTCR = 0;
  *PWMxPC = 0; 
  *PWMxPR = div - 1;
  *PWMxTCR = prevtc;
  return Fpclk / div;
}

// Setup all PWM channels
static void platform_setup_pwm()
{
  unsigned i;
  PREG temp;

  for( i = 0; i < 2; i ++ )
  {
    // Keep clock in reset, set PWM code
    temp = ( PREG )pwm_tcr[ i ];
    *temp = PWM_RESET;
    // Set match mode (reset on MR0 match)
    temp = ( PREG )pwm_mcr[ i ];
    *temp = 0x02; 
    // Set base frequency to 1MHz
    platform_pwm_set_clock( i * 6, 1000000 );
  }
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  unsigned pwmid = id / 6, chid = id % 6;
  PREG PWMxMR0 = pwmid == 0 ? ( PREG )&PWM0MR0 : ( PREG )&PWM1MR0;
  PREG PWMxMRc = ( PREG )pwm_channels[ pwmid ][ chid ];
  PREG PWMxLER = ( PREG )pwm_ler[ pwmid ];
  u32 divisor;

  divisor = platform_pwm_get_clock( id ) / frequency - 1;
  *PWMxMR0 = divisor;
  *PWMxMRc = ( divisor * duty ) / 100;
  *PWMxLER = 1 | ( 1 << ( chid + 1 ) );

  return platform_pwm_get_clock( id ) / divisor;
}

u32 platform_pwm_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  unsigned pwmid = id / 6;
  PREG PWMxTCR = ( PREG )pwm_tcr[ pwmid ];
  PREG PWMxPCR = ( PREG )pwm_pcr[ pwmid ];

  switch( op )
  {
    case PLATFORM_PWM_OP_START:
      *PWMxPCR = PWM_ENABLE_1 | PWM_ENABLE_2 | PWM_ENABLE_3 | PWM_ENABLE_4 | PWM_ENABLE_5 | PWM_ENABLE_6;
      *PWMxTCR = PWM_ENABLE | PWM_MODE;
      break;

    case PLATFORM_PWM_OP_STOP:
      *PWMxPCR = 0;   
      *PWMxTCR = PWM_RESET;
      break;

    case PLATFORM_PWM_OP_SET_CLOCK:
      res = platform_pwm_set_clock( id, data );
      break;

    case PLATFORM_PWM_OP_GET_CLOCK:
      res = platform_pwm_get_clock( id );
      break;
  }

  return res;
}

*/
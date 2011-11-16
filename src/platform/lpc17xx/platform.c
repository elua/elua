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
#include "elua_adc.h"
#include "platform_conf.h"
#include "lrotable.h"
#include "buf.h"

// Platform includes
#include "lpc17xx_gpio.h"
#include "lpc17xx_libcfg.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_adc.h"

#define SYSTICKHZ             10

// ****************************************************************************
// Platform initialization

static void platform_setup_timers();
static void platform_setup_pwm();
static void platform_setup_adcs();

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

#ifdef BUILD_ADC
  // Setup ADCs
  platform_setup_adcs();
#endif

  // System timer setup
  cmn_systimer_set_base_freq( mbed_get_cpu_frequency() );
  cmn_systimer_set_interrupt_freq( SYSTICKHZ );

  // Enable SysTick
  SysTick_Config( mbed_get_cpu_frequency() / SYSTICKHZ );

  // Common platform initialization code
  cmn_platform_init();

  return PLATFORM_OK;
} 

extern u32 SystemCoreClock;
u32 mbed_get_cpu_frequency()
{
  return SystemCoreClock;
}

// SysTick interrupt handler
void SysTick_Handler()
{
  // Handle virtual timers
  cmn_virtual_timer_cb();

  // Handle system timer call
  cmn_systimer_periodic();
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

static LPC_UART_TypeDef* const uart[] = { LPC_UART0, LPC_UART1, LPC_UART2, LPC_UART3 };

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
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

void platform_s_uart_send( unsigned id, u8 data )
{
  UART_Send(uart[ id ], &data, 1, BLOCKING);
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
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

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// ****************************************************************************
// Timer section

static LPC_TIM_TypeDef *tmr[] = { LPC_TIM0, LPC_TIM1, LPC_TIM2, LPC_TIM3 };
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
  TIM_ConfigStruct.PrescaleValue  = 1000000ULL / clock;

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
    platform_timer_set_clock( i, 1000000ULL );
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
  u32 last;

  last = ( ( u64 )delay_us * platform_timer_get_clock( id ) ) / 1000000;
  TIM_Cmd( tmr[ id ], ENABLE );
  TIM_ResetCounter( tmr[ id ] );
  while( tmr[ id ]->TC < last );
}
      
timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
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

    case PLATFORM_TIMER_OP_GET_MAX_CNT:
      res = 0xFFFFFFFF;
      break;
  }
  return res;
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

// *****************************************************************************
// ADC specific functions and variables

#ifdef BUILD_ADC


// Match trigger sources for timer 1 & timer 3
static const u32 adc_trig[] = { 0, ADC_START_ON_MAT11, 0, 0 };

int platform_adc_check_timer_id( unsigned id, unsigned timer_id )
{
  return ( ( timer_id == 1 ) );
}

void platform_adc_stop( unsigned id )
{  
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
    
  s->op_pending = 0;
  INACTIVATE_CHANNEL( d, id );
  
  // If there are no more active channels, stop the sequencer
  if( d->ch_active == 0 && d->running == 1 )
  {
    d->running = 0;
    NVIC_DisableIRQ( ADC_IRQn );
  }
}

// Handle ADC interrupts
// NOTE: This could probably be less complicated...
void ADC_IRQHandler(void)
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  elua_adc_ch_state *s = d->ch_state[ d->seq_ctr ];
  int i;
  
  // Disable sampling & current sequence channel
  ADC_StartCmd( LPC_ADC, 0 );
  ADC_ChannelCmd( LPC_ADC, s->id, DISABLE );
  ADC_IntConfig( LPC_ADC, s->id, DISABLE );

  if ( ADC_ChannelGetStatus( LPC_ADC, s->id, ADC_DATA_DONE ) )
  { 
    d->sample_buf[ d->seq_ctr ] = ( u16 )ADC_ChannelGetData( LPC_ADC, s->id );
    s->value_fresh = 1;
            
    if ( s->logsmoothlen > 0 && s->smooth_ready == 0)
      adc_smooth_data( s->id );
#if defined( BUF_ENABLE_ADC )
    else if ( s->reqsamples > 1 )
    {
      buf_write( BUF_ID_ADC, s->id, ( t_buf_data* )s->value_ptr );
      s->value_fresh = 0;
    }
#endif
        
    if ( adc_samples_available( s->id ) >= s->reqsamples && s->freerunning == 0 )
      platform_adc_stop( s->id );      
  }
    
  // Set up for next channel acquisition if we're still running
  if( d->running == 1 )
  {
    // Prep next channel in sequence, if applicable
    if( d->seq_ctr < ( d->seq_len - 1 ) )
      d->seq_ctr++;
    else if( d->seq_ctr == ( d->seq_len - 1 ) )
    { 
      adc_update_dev_sequence( 0 );
      d->seq_ctr = 0; // reset sequence counter if on last sequence entry
    }
          
    ADC_ChannelCmd( LPC_ADC, d->ch_state[ d->seq_ctr ]->id, ENABLE );
    ADC_IntConfig( LPC_ADC, d->ch_state[ d->seq_ctr ]->id, ENABLE );
         
    if( d->clocked == 1  && d->seq_ctr == 0 ) // always use clock for first in clocked sequence
      ADC_StartCmd( LPC_ADC, adc_trig[ d->timer_id ] );

    // Start next conversion if unclocked or if clocked and sequence index > 0
    if( ( d->clocked == 1 && d->seq_ctr > 0 ) || d->clocked == 0 )
      ADC_StartCmd( LPC_ADC, ADC_START_NOW );
  }
}


static void platform_setup_adcs()
{
  unsigned id;
  
  for( id = 0; id < NUM_ADC; id ++ )
    adc_init_ch_state( id );
  
  NVIC_SetPriority(ADC_IRQn, ((0x01<<3)|0x01));

  ADC_Init(LPC_ADC, 13000000);
  
  // Default enables CH0, disable channel
  ADC_ChannelCmd( LPC_ADC, 0, DISABLE );
  
  // Default enables ADC interrupt only on global, switch to per-channel
  ADC_IntConfig( LPC_ADC, ADC_ADGINTEN, DISABLE );
    
  platform_adc_set_clock( 0, 0 );
}


// NOTE: On this platform, there is only one ADC, clock settings apply to the whole device
u32 platform_adc_set_clock( unsigned id, u32 frequency )
{
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
  elua_adc_dev_state *d = adc_get_dev_state( 0 );

  if ( frequency > 0 )
  {
    d->clocked = 1;
    
    // Max Sampling Rate on LPC1768 is 200 kS/s
    if ( frequency > 200000 )
      frequency = 200000;
        
    // Run timer at 1MHz
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue     = 1;
    
    TIM_MatchConfigStruct.MatchChannel = 1;
    TIM_MatchConfigStruct.IntOnMatch   = FALSE;
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    TIM_MatchConfigStruct.StopOnMatch  = FALSE;
    TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    // Set match value to period (in uS) associated with frequency
    TIM_MatchConfigStruct.MatchValue   = ( 1000000ULL / ( frequency * 2 ) ) - 1;
        
    frequency = 1000000ULL / (TIM_MatchConfigStruct.MatchValue + 1);
  
    // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init( tmr[ d->timer_id ], TIM_TIMER_MODE, &TIM_ConfigStruct );
    TIM_ConfigMatch( tmr[ d->timer_id ], &TIM_MatchConfigStruct );
    TIM_ResetCounter( tmr[ d->timer_id ] );
  }
  else
    d->clocked = 0;
    
  return frequency;
}

static const u8 adc_ports[] = {  0, 0,   0,  0,  1,  1, 0, 0 };
static const u8 adc_pins[] =  { 23, 24, 25, 26, 30, 31, 3, 2 };
static const u8 adc_funcs[] = {  1,  1,  1,  1,  3,  3, 2, 2 };

// Prepare Hardware Channel
int platform_adc_update_sequence( )
{ 
  elua_adc_dev_state *d = adc_get_dev_state( 0 ); 
  PINSEL_CFG_Type PinCfg;
  u8 seq_tmp;
  unsigned id;
  
  // Enable Needed Pins
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  
  for( seq_tmp = 0; seq_tmp < d->seq_len; seq_tmp++ )
  {
    id = d->ch_state[ seq_tmp ]->id;
       
    PinCfg.Funcnum = adc_funcs[ id ];
    PinCfg.Pinnum = adc_pins[ id ];
    PinCfg.Portnum = adc_ports[ id ];
    PINSEL_ConfigPin(&PinCfg);
  }
  
  return PLATFORM_OK;
}


int platform_adc_start_sequence()
{ 
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  if( d->running != 1 )
  {
    adc_update_dev_sequence( 0 );
    
    // Start sampling on first channel
    d->seq_ctr = 0;
    ADC_ChannelCmd( LPC_ADC, d->ch_state[ d->seq_ctr ]->id, ENABLE );
    ADC_IntConfig( LPC_ADC, d->ch_state[ d->seq_ctr ]->id, ENABLE );

    d->running = 1;
    NVIC_EnableIRQ( ADC_IRQn );
    
    if( d->clocked == 1 )
    {
      ADC_StartCmd( LPC_ADC, adc_trig[ d->timer_id ] );
      TIM_ResetCounter( tmr[ d->timer_id ] );
      TIM_Cmd( tmr[ d->timer_id ], ENABLE );
    }
    else
      ADC_StartCmd( LPC_ADC, ADC_START_NOW );
  }
  
  return PLATFORM_OK;
}

#endif // ifdef BUILD_ADC


// ****************************************************************************
// PWM functions


// Helper function: get timer clock
u32 platform_pwm_get_clock( unsigned id )
{
  return CLKPWR_GetPCLK( CLKPWR_PCLKSEL_PWM1 ) / ( LPC_PWM1->PR + 1 );
}

// Helper function: set timer clock
u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  PWM_TIMERCFG_Type PWMCfgDat;
  
  PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_USVAL;
  PWMCfgDat.PrescaleValue = 1000000ULL / clock;
  PWM_Init( LPC_PWM1, PWM_MODE_TIMER, &PWMCfgDat );

  return clock;
}

// Setup all PWM channels
static void platform_setup_pwm()
{
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  
  // Keep clock in reset, set PWM code
  PWM_ResetCounter( LPC_PWM1 );
  
  // Set match mode (reset on MR0 match)
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = 0;
  PWMMatchCfgDat.ResetOnMatch = ENABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch( LPC_PWM1, &PWMMatchCfgDat );

  // Set base frequency to 1MHz
  platform_pwm_set_clock( 0, 1000000 );
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  u32 divisor = platform_pwm_get_clock( id ) / frequency - 1;
    
  PWM_MatchUpdate( LPC_PWM1, 0, divisor, PWM_MATCH_UPDATE_NOW ); // PWM1 cycle rate
  PWM_MatchUpdate( LPC_PWM1, id, ( divisor * duty ) / 100, PWM_MATCH_UPDATE_NOW ); // PWM1 channel edge position
  
  if ( id > 1 ) // Channel one is permanently single-edge
    PWM_ChannelConfig( LPC_PWM1, id, PWM_CHANNEL_SINGLE_EDGE );
  
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = id;
  PWMMatchCfgDat.ResetOnMatch = DISABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);

  PWM_ResetCounter(LPC_PWM1);
  PWM_CounterCmd(LPC_PWM1, ENABLE);

  PWM_ChannelCmd(LPC_PWM1, id, ENABLE);

  return platform_pwm_get_clock( id ) / divisor;
}

void platform_pwm_start( unsigned id )
{
  PWM_Cmd(LPC_PWM1, ENABLE);
}

void platform_pwm_stop( unsigned id )
{
  PWM_Cmd(LPC_PWM1, DISABLE);
}

// ****************************************************************************
// Platform specific modules go here

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
extern const LUA_REG_TYPE mbed_pio_map[];

const LUA_REG_TYPE platform_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "pio" ), LROVAL( mbed_pio_map ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_platform( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, PS_LIB_TABLE_NAME, platform_map );
  
  // Setup the new tables inside platform table
  lua_newtable( L );
  luaL_register( L, NULL, mbed_pio_map );
  lua_setfield( L, -2, "pio" );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

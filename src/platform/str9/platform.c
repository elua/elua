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
#include "91x_scu.h"
#include "91x_fmi.h"
#include "91x_gpio.h"
#include "91x_uart.h"
#include "91x_tim.h"
#include "common.h"
#include "platform_conf.h"
#include "91x_vic.h"
#include "lrotable.h"
#include "91x_i2c.h"
#include "91x_wiu.h"
#include "buf.h"
#include "elua_adc.h"
#include "91x_adc.h"
#include "91x_ssp.h"
#include "91x_can.h"
#include "utils.h"

// ****************************************************************************
// Platform initialization
const GPIO_TypeDef* port_data[] = { GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, GPIO9 };
#ifndef VTMR_TIMER_ID
#error Define VTMR_TIMER_ID to the ID of the timer used for the system timer
#endif
const TIM_TypeDef* str9_timer_data[] = { TIM0, TIM1, TIM2, TIM3 };

// System timer implementation on STR9 uses one of the physical timers (defined by
// VTMR_TIMER_ID). This is the same timer used for VTMR implementation. Its base
// frequency is set to 1MHz in platform_s_timer_set_match_int. It runs at 16Hz
// since this gives an exact number of microseconds (62500) before its overflow.

static void platform_setup_adcs();

static void cans_init();

static void platform_config_scu()
{     
  volatile u16 i = 0xFFFF;
  while (i-- > 0);  
  
   // SCU initialization
  SCU_MCLKSourceConfig(SCU_MCLK_OSC);
  SCU_PLLFactorsConfig(192,25,2);            /* PLL = 96 MHz */
  SCU_PLLCmd(ENABLE);                        /* PLL Enabled  */
  SCU_MCLKSourceConfig(SCU_MCLK_PLL);        /* MCLK = PLL   */  
  
  SCU_PFQBCCmd( ENABLE );

  /* Set the RCLK Clock divider to max speed*/
  SCU_RCLKDivisorConfig(SCU_RCLK_Div1);
  /* Set the PCLK Clock to MCLK/2 */
  SCU_PCLKDivisorConfig(SCU_PCLK_Div2);
  /* Set the HCLK Clock to MCLK */
  SCU_HCLKDivisorConfig(SCU_HCLK_Div1);
  /* Set the BRCLK Clock to MCLK */
  SCU_BRCLKDivisorConfig(SCU_BRCLK_Div1);
  
  // Enable VIC clock
  SCU_AHBPeriphClockConfig(__VIC, ENABLE);
  SCU_AHBPeriphReset(__VIC, DISABLE);
                 
  // Enable the UART clocks
  SCU_APBPeriphClockConfig(__UART_ALL, ENABLE);

  // Enable the timer clocks
  SCU_APBPeriphClockConfig(__TIM01, ENABLE);
  SCU_APBPeriphReset(__TIM01, DISABLE);
  SCU_APBPeriphClockConfig(__TIM23, ENABLE);
  SCU_APBPeriphReset(__TIM23, DISABLE);

  // Enable the GPIO clocks  
  SCU_APBPeriphClockConfig(__GPIO_ALL, ENABLE);  

  // Enable the WIU clock
  SCU_APBPeriphClockConfig(__WIU, ENABLE);
  SCU_APBPeriphReset(__WIU, DISABLE);

  // Enable the I2C clocks
  SCU_APBPeriphClockConfig(__I2C0, ENABLE);
  SCU_APBPeriphReset(__I2C0, DISABLE);
  SCU_APBPeriphClockConfig(__I2C1, ENABLE);
  SCU_APBPeriphReset(__I2C1, DISABLE);
  
  // Enable the ADC clocks
  SCU_APBPeriphClockConfig(__ADC, ENABLE);

  // Enable the SSP clocks
  SCU_APBPeriphClockConfig(__SSP0,ENABLE);
  SCU_APBPeriphReset(__SSP0,DISABLE);
  SCU_APBPeriphClockConfig(__SSP1,ENABLE);
  SCU_APBPeriphReset(__SSP1,DISABLE);
}

// Port/pin definitions of the eLua UART connection for different boards
#define UART_RX_IDX   0
#define UART_TX_IDX   1

#ifdef ELUA_BOARD_STRE912
static const GPIO_TypeDef* uart_port_data[] = { GPIO5, GPIO5 };
static const u8 uart_pin_data[] = { GPIO_Pin_1, GPIO_Pin_0 };
#else // STR9-comStick
static const GPIO_TypeDef* uart_port_data[] = { GPIO3, GPIO3 };
static const u8 uart_pin_data[] = { GPIO_Pin_2, GPIO_Pin_3 };
#endif


// Plaform specific GPIO UART setup
static void platform_gpio_uart_setup()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // RX
  GPIO_StructInit( &GPIO_InitStructure );
  GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
  GPIO_InitStructure.GPIO_Pin = uart_pin_data[ UART_RX_IDX ]; 
  GPIO_InitStructure.GPIO_IPConnected = GPIO_IPConnected_Enable;
  GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1  ;
  GPIO_Init( ( GPIO_TypeDef* )uart_port_data[ UART_RX_IDX ], &GPIO_InitStructure );
  // TX
  GPIO_StructInit( &GPIO_InitStructure );
  GPIO_InitStructure.GPIO_Direction=GPIO_PinOutput;
  GPIO_InitStructure.GPIO_Pin = uart_pin_data[ UART_TX_IDX ];
  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
  GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt3  ;
  GPIO_Init( ( GPIO_TypeDef* )uart_port_data[ UART_TX_IDX ], &GPIO_InitStructure );
}

int platform_init()
{
  unsigned i;
  TIM_InitTypeDef tim;
  TIM_TypeDef* base;  
        
  // System configuration
  platform_config_scu();
  
  // PIO setup
  for( i = 0; i < 10; i ++ )
    GPIO_DeInit( ( GPIO_TypeDef* )port_data[ i ] );
    
  // UART setup
  platform_gpio_uart_setup();

  // Initialize timers
  for( i = 0; i < NUM_PHYS_TIMER; i ++ )
  {
    base = ( TIM_TypeDef* )str9_timer_data[ i ];
    TIM_DeInit( base );
    TIM_StructInit( &tim );
    tim.TIM_Clock_Source = TIM_CLK_APB;
    tim.TIM_Prescaler = 255;      
    TIM_Init( base, &tim );    
    TIM_CounterCmd( base, TIM_START );
  }
  
 cmn_platform_init();

#ifdef BUILD_ADC
  // Setup ADCs
  platform_setup_adcs();
#endif

#ifdef BUILD_CAN
  // Setup CANs
  cans_init();
#endif

  // Initialize system timer
  cmn_systimer_set_base_freq( 1000000 );
  cmn_systimer_set_interrupt_freq( VTMR_FREQ_HZ );
  platform_s_timer_set_match_int( VTMR_TIMER_ID, 1000000 / VTMR_FREQ_HZ, PLATFORM_TIMER_INT_CYCLIC );
  return PLATFORM_OK;
}

// ****************************************************************************
// PIO functions

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  GPIO_TypeDef* base = ( GPIO_TypeDef* )port_data[ port ];
  GPIO_InitTypeDef data;
  pio_type retval = 1;
  
  GPIO_StructInit( &data );
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:    
      GPIO_Write( base, ( u8 )pinmask );
      break;
      
    case PLATFORM_IO_PIN_SET:
      GPIO_WriteBit( base, ( u8 )pinmask, Bit_SET );
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      GPIO_WriteBit( base, ( u8 )pinmask, Bit_RESET );
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:
      pinmask = 0xFF;     
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      data.GPIO_Direction = GPIO_PinOutput;
      data.GPIO_Type = GPIO_Type_PushPull ;
      data.GPIO_Alternate=GPIO_OutputAlt1;
      data.GPIO_Pin = ( u8 )pinmask;
      GPIO_Init(base, &data);
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      pinmask = 0xFF;     
    case PLATFORM_IO_PIN_DIR_INPUT:
      data.GPIO_Pin = ( u8 )pinmask;
      GPIO_Init(base, &data);
      break;    
            
    case PLATFORM_IO_PORT_GET_VALUE:
      retval = GPIO_Read( base );
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = GPIO_ReadBit( base, ( u8 )pinmask );
      break;
      
    default:
      retval = 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// UART

static const UART_TypeDef* uarts[] = { UART0, UART1, UART2 };

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  UART_InitTypeDef UART_InitStructure;
  UART_TypeDef* p_uart = ( UART_TypeDef* )uarts[ id ];
    
  // Then configure UART parameters
  switch( databits )
  {
    case 5:
      UART_InitStructure.UART_WordLength = UART_WordLength_5D;
      break;      
    case 6:
      UART_InitStructure.UART_WordLength = UART_WordLength_6D;
      break;      
    case 7:
      UART_InitStructure.UART_WordLength = UART_WordLength_7D;
      break;      
    case 8:
      UART_InitStructure.UART_WordLength = UART_WordLength_8D;
      break;
  }
  if( stopbits == PLATFORM_UART_STOPBITS_1 )
    UART_InitStructure.UART_StopBits = UART_StopBits_1;    
  else
    UART_InitStructure.UART_StopBits = UART_StopBits_2;
  if( parity == PLATFORM_UART_PARITY_EVEN )
    UART_InitStructure.UART_Parity = UART_Parity_Even;
  else if( parity == PLATFORM_UART_PARITY_ODD )
    UART_InitStructure.UART_Parity = UART_Parity_Odd;
  else
    UART_InitStructure.UART_Parity = UART_Parity_No;
  UART_InitStructure.UART_BaudRate = baud;
  UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
  UART_InitStructure.UART_Mode = UART_Mode_Tx_Rx;
  UART_InitStructure.UART_FIFO = UART_FIFO_Enable;
  UART_InitStructure.UART_TxFIFOLevel = UART_FIFOLevel_1_2; /* FIFO size 16 bytes, FIFO level 8 bytes */
  UART_InitStructure.UART_RxFIFOLevel = UART_FIFOLevel_1_2; /* FIFO size 16 bytes, FIFO level 8 bytes */

  UART_DeInit( p_uart );
  UART_Init( p_uart , &UART_InitStructure );
  UART_Cmd( p_uart, ENABLE );
  
  return baud;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  UART_TypeDef* p_uart = ( UART_TypeDef* )uarts[ id ];

//  while( UART_GetFlagStatus( STR9_UART, UART_FLAG_TxFIFOFull ) == SET );
  UART_SendData( p_uart, data );
  while( UART_GetFlagStatus( p_uart, UART_FLAG_TxFIFOFull ) != RESET );  
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  UART_TypeDef* p_uart = ( UART_TypeDef* )uarts[ id ];

  if( timeout == 0 )
  {
    // Return data only if already available
    if( UART_GetFlagStatus( p_uart, UART_FLAG_RxFIFOEmpty ) != SET )
      return UART_ReceiveData( p_uart );
    else
      return -1;
  }
  while( UART_GetFlagStatus( p_uart, UART_FLAG_RxFIFOEmpty ) == SET );
  return UART_ReceiveData( p_uart ); 
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

// ****************************************************************************
// Timer

u8 str9_timer_int_periodic_flag[ NUM_PHYS_TIMER ];

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  return ( SCU_GetPCLKFreqValue() * 1000 ) / ( TIM_GetPrescalerValue( ( TIM_TypeDef* )str9_timer_data[ id ] ) + 1 );
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  u32 baseclk = SCU_GetPCLKFreqValue() * 1000;
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ id ];      
  u64 bestdiv;
  
  bestdiv = ( ( u64 )baseclk << 16 ) / clock;
  if( bestdiv & 0x8000 )
    bestdiv += 0x10000;
  bestdiv >>= 16;
  if( bestdiv > 256 )
    bestdiv = 256;
  TIM_PrescalerConfig( base, bestdiv - 1 );
  return baseclk / bestdiv;
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ id ];  
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
  TIM_CounterCmd( base, TIM_STOP );
  TIM_CounterCmd( base, TIM_CLEAR );  
  TIM_CounterCmd( base, TIM_START );  
  while( TIM_GetCounterValue( base ) >= 0xFFFC );
  while( TIM_GetCounterValue( base ) < final );  
}
      
timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data )
{
  u32 res = 0;
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ id ];  

  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      TIM_CounterCmd( base, TIM_STOP );
      TIM_CounterCmd( base, TIM_CLEAR );  
      TIM_CounterCmd( base, TIM_START );  
      while( TIM_GetCounterValue( base ) >= 0xFFFC );        
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = TIM_GetCounterValue( base );
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

int platform_s_timer_set_match_int( unsigned id, timer_data_type period_us, int type )
{
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ id ];  
  u32 freq;
  u64 final;
  TIM_InitTypeDef TIM_InitStructure;

  if( period_us == 0 )
  {
    TIM_ITConfig( base, TIM_IT_OC1, DISABLE );
    base->CR1 = 0;
    base->CR2 = 0;
    return PLATFORM_TIMER_INT_OK; 
  }
  platform_timer_set_clock( id, 1000000 );
  freq = platform_timer_get_clock( id );
  final = ( ( u64 )period_us * freq ) / 1000000;
  if( final == 0 )
    return PLATFORM_TIMER_INT_TOO_SHORT;
  if( final > 0xFFFF )
    return PLATFORM_TIMER_INT_TOO_LONG;

  TIM_CounterCmd( base, TIM_STOP );
  TIM_StructInit( &TIM_InitStructure );
  TIM_InitStructure.TIM_Mode = TIM_OCM_CHANNEL_1;                           
  TIM_InitStructure.TIM_OC1_Modes = TIM_TIMING;               
  TIM_InitStructure.TIM_Clock_Source = TIM_CLK_APB;         
  TIM_InitStructure.TIM_Clock_Edge = TIM_CLK_EDGE_FALLING;  
  TIM_InitStructure.TIM_Prescaler = TIM_GetPrescalerValue( base );
  TIM_InitStructure.TIM_Pulse_Length_1 = ( u16 )final;          
  TIM_Init( base, &TIM_InitStructure );
  str9_timer_int_periodic_flag[ id ] = type;

  TIM_CounterCmd( base, TIM_CLEAR );  
  TIM_CounterCmd( base, TIM_START );  
  while( TIM_GetCounterValue( base ) >= 0xFFFC );
  TIM_ITConfig( base, TIM_IT_OC1, ENABLE );

  return PLATFORM_TIMER_INT_OK;
}

u64 platform_timer_sys_raw_read()
{
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ VTMR_TIMER_ID ];

  return TIM_GetCounterValue( base );
}

void platform_timer_sys_enable_int()
{
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ VTMR_TIMER_ID ];

  TIM_ITConfig( base, TIM_IT_OC1, ENABLE );
}

void platform_timer_sys_disable_int()
{
  TIM_TypeDef* base = ( TIM_TypeDef* )str9_timer_data[ VTMR_TIMER_ID ];

  TIM_ITConfig( base, TIM_IT_OC1, DISABLE );
}

timer_data_type platform_timer_read_sys()
{
  return cmn_systimer_get();
}

// *****************************************************************************
// ADC specific functions and variables

#ifdef BUILD_ADC

ADC_InitTypeDef ADC_InitStructure;

int platform_adc_check_timer_id( unsigned id, unsigned timer_id )
{
  return 0; // This platform does not support direct timer triggering
}

void platform_adc_stop( unsigned id )
{  
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  s->op_pending = 0;
  INACTIVATE_CHANNEL( d, id );

  // If there are no more active channels, stop the sequencer
  if( d->ch_active == 0 )
    d->running = 0;
}


void ADC_IRQHandler( void )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  elua_adc_ch_state *s;

  if ( ADC_GetFlagStatus( ADC_FLAG_ECV ) )
  {
    d->seq_ctr = 0;
    while( d->seq_ctr < d->seq_len )
    {
      s = d->ch_state[ d->seq_ctr ];
      d->sample_buf[ s->id ] = ( u16 )ADC_GetConversionValue( s->id );
      s->value_fresh = 1;
    
      // Fill in smoothing buffer until warmed up
      if ( s->logsmoothlen > 0 && s->smooth_ready == 0)
        adc_smooth_data( s->id );
#if defined( BUF_ENABLE_ADC )
      else if ( s->reqsamples > 1 )
      {
        buf_write( BUF_ID_ADC, s->id, ( t_buf_data* )s->value_ptr );
        s->value_fresh = 0;
      }
#endif

      // If we have the number of requested samples, stop sampling
      if ( adc_samples_available( s->id ) >= s->reqsamples && s->freerunning == 0 )
        platform_adc_stop( s->id );

      d->seq_ctr++;
    }
    d->seq_ctr = 0;
    ADC_ClearFlag( ADC_FLAG_ECV );
  }
  
  if( d->running == 1 )
    adc_update_dev_sequence( 0 );

  if ( d->clocked == 0 && d->running == 1 )
  {
    ADC_ConversionCmd( ADC_Conversion_Start );
  }

  VIC0->VAR = 0xFF;
}

static void platform_setup_adcs()
{
  unsigned id;
  
  for( id = 0; id < NUM_ADC; id ++ )
    adc_init_ch_state( id );
  
  VIC_Config(ADC_ITLine, VIC_IRQ, 0);
  VIC_ITCmd(ADC_ITLine, ENABLE);
  
  ADC_StructInit(&ADC_InitStructure);

  /* Configure the ADC  structure in continuous mode conversion */
  ADC_DeInit();             /* ADC Deinitialization */
  ADC_InitStructure.ADC_Channel_0_Mode = ADC_NoThreshold_Conversion;
  ADC_InitStructure.ADC_Scan_Mode = ENABLE;
  ADC_InitStructure.ADC_Conversion_Mode = ADC_Single_Mode;
  
  ADC_Cmd( ENABLE );
  ADC_PrescalerConfig( 0x2 );
  ADC_Init( &ADC_InitStructure );

  ADC_ITConfig(ADC_IT_ECV, ENABLE);

  platform_adc_set_clock( 0, 0 );
}


// NOTE: On this platform, there is only one ADC, clock settings apply to the whole device
u32 platform_adc_set_clock( unsigned id, u32 frequency )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  // No clocked conversions supported
  d->clocked = 0;

  return 0;
}

const int adc_gpio_chan[] = { GPIO_ANAChannel0, GPIO_ANAChannel1, GPIO_ANAChannel2, GPIO_ANAChannel3, GPIO_ANAChannel4, GPIO_ANAChannel5, GPIO_ANAChannel6, GPIO_ANAChannel7 };

// Prepare Hardware Channel
int platform_adc_update_sequence( )
{ 
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
    
  ADC_Cmd( DISABLE );
  ADC_DeInit();
  
  ADC_InitStructure.ADC_Channel_0_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_1_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_2_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_3_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_4_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_5_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_6_Mode = ADC_No_Conversion;
  ADC_InitStructure.ADC_Channel_7_Mode = ADC_No_Conversion;

  d->seq_ctr = 0;
  while( d->seq_ctr < d->seq_len )
  {
    GPIO_ANAPinConfig( adc_gpio_chan[ d->ch_state[ d->seq_ctr ]->id ], ENABLE );

    // This is somewhat terrible, but the API doesn't provide an alternative
    switch( d->ch_state[ d->seq_ctr ]->id )
    {
      case 0:
        ADC_InitStructure.ADC_Channel_0_Mode = ADC_NoThreshold_Conversion;
        break;
      case 1:
        ADC_InitStructure.ADC_Channel_1_Mode = ADC_NoThreshold_Conversion;
        break;
      case 2:
        ADC_InitStructure.ADC_Channel_2_Mode = ADC_NoThreshold_Conversion;
        break;
      case 3:
        ADC_InitStructure.ADC_Channel_3_Mode = ADC_NoThreshold_Conversion;
        break;
      case 4:
        ADC_InitStructure.ADC_Channel_4_Mode = ADC_NoThreshold_Conversion;
        break;
      case 5:
        ADC_InitStructure.ADC_Channel_5_Mode = ADC_NoThreshold_Conversion;
        break;
      case 6:
        ADC_InitStructure.ADC_Channel_6_Mode = ADC_NoThreshold_Conversion;
        break;
      case 7:
        ADC_InitStructure.ADC_Channel_7_Mode = ADC_NoThreshold_Conversion;
        break;
    }
    d->seq_ctr++;
  }
  d->seq_ctr = 0;
  
  ADC_Cmd( ENABLE );
  ADC_PrescalerConfig( 0x2 );
  ADC_Init( &ADC_InitStructure );

  ADC_ITConfig( ADC_IT_ECV, ENABLE );

  return PLATFORM_OK;
}


int platform_adc_start_sequence()
{ 
    elua_adc_dev_state *d = adc_get_dev_state( 0 );

    // Only force update and initiate if we weren't already running
    // changes will get picked up during next interrupt cycle
    if ( d->running != 1 )
    {
      // Bail if we somehow were trying to set up clocked conversion
      if( d->clocked == 1 )
        return PLATFORM_ERR;

      adc_update_dev_sequence( 0 );

      d->running = 1;

      ADC_ClearFlag( ADC_FLAG_ECV );

      ADC_ITConfig( ADC_IT_ECV, ENABLE );

      ADC_ConversionCmd( ADC_Conversion_Start );
    }

    return PLATFORM_OK;
  }


#endif // ifdef BUILD_ADC


// ****************************************************************************
// PWM functions

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  TIM_TypeDef* p_timer = ( TIM_TypeDef* )str9_timer_data[ id ];
  u32 base = SCU_GetPCLKFreqValue() * 1000;
  u32 div = ( base / 256 ) / frequency;
  TIM_InitTypeDef tim;

  TIM_DeInit( p_timer );
  tim.TIM_Mode = TIM_PWM;
  tim.TIM_Clock_Source = TIM_CLK_APB;       
  tim.TIM_Prescaler = 0xFF;       
  tim.TIM_Pulse_Level_1 = TIM_HIGH;   
  tim.TIM_Period_Level = TIM_LOW;    
  tim.TIM_Full_Period = div;
  tim.TIM_Pulse_Length_1 = ( div * duty ) / 100;
  TIM_Init( p_timer, &tim );

  return base / div;
}

u32 platform_pwm_set_clock( unsigned id, u32 clock )
{
  TIM_TypeDef* p_timer = ( TIM_TypeDef* )str9_timer_data[ id ];
  u32 base = ( SCU_GetPCLKFreqValue() * 1000 );
  u32 div = base / clock;

  TIM_PrescalerConfig( p_timer, ( u8 )div - 1 );
  return base / div;
}

u32 platform_pwm_get_clock( unsigned id )
{
  TIM_TypeDef* p_timer = ( TIM_TypeDef* )str9_timer_data[ id ];

  return ( SCU_GetPCLKFreqValue() * 1000 ) / ( TIM_GetPrescalerValue( p_timer ) + 1 );
}

void platform_pwm_start( unsigned id )
{
  TIM_TypeDef* p_timer = ( TIM_TypeDef* )str9_timer_data[ id ];

  TIM_CounterCmd( p_timer, TIM_START );
}

void platform_pwm_stop( unsigned id )
{
  TIM_TypeDef* p_timer = ( TIM_TypeDef* )str9_timer_data[ id ];

  TIM_CounterCmd( p_timer, TIM_STOP );
}

// ****************************************************************************
// I2C support
static const GPIO_TypeDef* i2c_port_data[] = { GPIO1, GPIO2 };
static const I2C_TypeDef* i2cs[] = { I2C0, I2C1 };
static const u8 i2c_clock_pin[] = { GPIO_Pin_4, GPIO_Pin_2 };
static const u8 i2c_data_pin[] = { GPIO_Pin_6, GPIO_Pin_3 };

u32 platform_i2c_setup( unsigned id, u32 speed )
{
  GPIO_InitTypeDef GPIO_InitStructure;
  I2C_InitTypeDef I2C_InitStructure;

  // Setup PIO
  GPIO_StructInit( &GPIO_InitStructure );
  GPIO_InitStructure.GPIO_Pin = i2c_clock_pin[ id ] | i2c_data_pin[ id ]; 
  GPIO_InitStructure.GPIO_Type = GPIO_Type_OpenCollector;
  GPIO_InitStructure.GPIO_IPConnected = GPIO_IPConnected_Enable;
  GPIO_InitStructure.GPIO_Alternate = id == 0 ? GPIO_OutputAlt3 : GPIO_OutputAlt2;
  GPIO_Init( ( GPIO_TypeDef* )i2c_port_data[ id ], &GPIO_InitStructure );
 
  // Setup and interface
  I2C_StructInit( &I2C_InitStructure );
  I2C_InitStructure.I2C_GeneralCall = I2C_GeneralCall_Disable;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_CLKSpeed = speed;
  I2C_InitStructure.I2C_OwnAddress = 0XA0 + id; // dummy, shouldn't matter
  I2C_Init( ( I2C_TypeDef* )i2cs[ id ], &I2C_InitStructure );

  // Return actual speed
  return speed;
}

void platform_i2c_send_start( unsigned id )
{
  I2C_TypeDef *pi2c = ( I2C_TypeDef* )i2cs[ id ];

  //while( I2C_GetFlagStatus( pi2c, I2C_FLAG_BUSY ) );
  I2C_GenerateStart( pi2c, ENABLE );
  while( I2C_CheckEvent( pi2c, I2C_EVENT_MASTER_MODE_SELECT ) != SUCCESS );
}

void platform_i2c_send_stop( unsigned id )
{
  I2C_TypeDef *pi2c = ( I2C_TypeDef* )i2cs[ id ];

  I2C_GenerateSTOP( pi2c, ENABLE );
  while( I2C_GetFlagStatus( pi2c, I2C_FLAG_BUSY ) );
}

int platform_i2c_send_address( unsigned id, u16 address, int direction )
{
  I2C_TypeDef *pi2c = ( I2C_TypeDef* )i2cs[ id ];
  u16 flags;

  I2C_Send7bitAddress( pi2c, address, direction == PLATFORM_I2C_DIRECTION_TRANSMITTER ? I2C_MODE_TRANSMITTER : I2C_MODE_RECEIVER );
  while( 1 )
  {
    flags = I2C_GetLastEvent( pi2c );
    if( flags & I2C_FLAG_AF )
      return 0;
    if( flags == I2C_EVENT_MASTER_MODE_SELECTED )
      break;
  }
  I2C_ClearFlag( pi2c, I2C_FLAG_ENDAD );
  if( direction == PLATFORM_I2C_DIRECTION_TRANSMITTER )
  {
    while( I2C_CheckEvent( pi2c, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) != SUCCESS );
    I2C_ClearFlag( pi2c, I2C_FLAG_BTF );
  }
  return 1;
}

int platform_i2c_send_byte( unsigned id, u8 data )
{
  I2C_TypeDef *pi2c = ( I2C_TypeDef* )i2cs[ id ];
  u16 flags;

  I2C_SendData( pi2c, data ); 
  while( 1 )
  {
    flags = I2C_GetLastEvent( pi2c );
    if( flags & I2C_FLAG_AF )
      return 0;
    if( flags == I2C_EVENT_MASTER_BYTE_TRANSMITTED )
      break;
  }
  I2C_ClearFlag( pi2c, I2C_FLAG_BTF );
  return 1;
}

int platform_i2c_recv_byte( unsigned id, int ack )
{
  I2C_TypeDef *pi2c = ( I2C_TypeDef* )i2cs[ id ];

  I2C_AcknowledgeConfig( pi2c, ack ? ENABLE : DISABLE );
  if( !ack )
    I2C_GenerateSTOP( pi2c, ENABLE );
  while( I2C_CheckEvent( pi2c, I2C_EVENT_MASTER_BYTE_RECEIVED ) != SUCCESS );
  return I2C_ReceiveData( pi2c );
}

// ****************************************************************************
// CAN
#if defined( BUILD_CAN )
static canmsg RxCanMsg;
static canmsg TxCanMsg;
static GPIO_InitTypeDef    GPIO_InitStructure;
static CAN_InitTypeDef     CAN_InitStructure;

static vu32 frame_received_flag = 0;

/* used message object numbers */
enum {
  CAN_TX_STD_MSGOBJ = 0,
  CAN_TX_EXT_MSGOBJ = 1,
  CAN_RX_STD_MSGOBJ = 2,
  CAN_RX_EXT_MSGOBJ = 3
};

/*******************************************************************************
* Function Name  : CAN_IRQHandler
* Description    : This function handles the CAN interrupt request
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_IRQHandler(void)
{
  u32 msgobj = 0;

  if(CAN->IDR == 0x8000)	/* status interrupt */
    (void)CAN->SR;	/* read the status register to clear*/
  else if(CAN->IDR >= 1 && CAN->IDR <= 32)
  {
    /* get the message object number that caused the interrupt to occur */
    switch(msgobj = CAN->IDR - 1)
    {
      case  0: case 1:/* CAN_TX_MSGOBJ */
        CAN_ReleaseTxMessage(msgobj);
      	break;

      default:
        CAN_ReceiveMessage(msgobj, FALSE, &RxCanMsg);
      	CAN_ReleaseRxMessage(msgobj);
      	frame_received_flag = 1;
        break;
    }
  }
  /*write any value to VIC0 VAR*/  
  VIC0->VAR = 0xFF;
}

void cans_init( void )
{
  SCU_APBPeriphClockConfig(__CAN, ENABLE);
  SCU_APBPeriphReset(__CAN, DISABLE); 

#ifdef ELUA_BOARD_STRE912
 /* P3.3 alternate input 1, CAN_RX pin 61*/
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Direction=GPIO_PinInput;
  GPIO_InitStructure.GPIO_IPConnected=GPIO_IPConnected_Enable;
  GPIO_InitStructure.GPIO_Alternate=GPIO_InputAlt1;
  GPIO_Init(GPIO3,&GPIO_InitStructure);

  /* P3.2 alternate output 2, CAN_TX pin 60*/
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Direction=GPIO_PinOutput;
  GPIO_InitStructure.GPIO_Type=GPIO_Type_PushPull;
  GPIO_InitStructure.GPIO_Alternate=GPIO_OutputAlt2;
  GPIO_Init(GPIO3,&GPIO_InitStructure);
#else // STR9-comStick
 /* P5.0 alternate input 1, CAN_RX pin 12*/
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Direction=GPIO_PinInput;
  GPIO_InitStructure.GPIO_IPConnected=GPIO_IPConnected_Enable;
  GPIO_InitStructure.GPIO_Alternate=GPIO_InputAlt1;
  GPIO_Init(GPIO5,&GPIO_InitStructure);

  /* P5.1 alternate output 2, CAN_TX pin 18*/
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Direction=GPIO_PinOutput;
  GPIO_InitStructure.GPIO_Type=GPIO_Type_PushPull;
  GPIO_InitStructure.GPIO_Alternate=GPIO_OutputAlt2;
  GPIO_Init(GPIO5,&GPIO_InitStructure);
#endif
}

u32 platform_can_setup( unsigned id, u32 clock )
{
  //Timing input values
  const u8 tseg1 = 2;
  const u8 tseg2 = 1;
  const u8 sjw = 4;

  //PCLK clock
  u32 fclk = SCU_GetPCLKFreqValue();

  //Baud Rate Prescaler
  //Check 'The insider's guide to the STR91x ARM 9'
  //Section 4.13.2.1 Bit Timing
  u32 brp = 1000 * fclk / clock / (1 + tseg1 + tseg2);

  /* initialize the CAN, interrupts enabled */
  CAN_InitStructure.CAN_ConfigParameters=CAN_CR_IE;
  CAN_Init(&CAN_InitStructure);

  //Set Can Timing
  CAN_EnterInitMode(CAN_CR_CCE | CAN_InitStructure.CAN_ConfigParameters);
  CAN_SetTiming(tseg1, tseg2, sjw, brp);
  CAN_LeaveInitMode();

  //Set message objects 
  CAN_SetUnusedAllMsgObj();
  CAN_SetTxMsgObj(CAN_TX_STD_MSGOBJ, CAN_STD_ID, DISABLE);
  CAN_SetTxMsgObj(CAN_TX_EXT_MSGOBJ, CAN_EXT_ID, DISABLE);
  CAN_SetRxMsgObj(CAN_RX_STD_MSGOBJ, CAN_STD_ID, 0, CAN_LAST_STD_ID, TRUE);
  CAN_SetRxMsgObj(CAN_RX_EXT_MSGOBJ, CAN_EXT_ID, 0, CAN_LAST_EXT_ID, TRUE);

  return clock;
}

int platform_can_send( unsigned id, u32 canid, u8 idtype, u8 len, const u8 *data )
{
  TxCanMsg.IdType = idtype;
  TxCanMsg.Id = canid;
  TxCanMsg.Dlc = len;
  memcpy(TxCanMsg.Data, data, len);
  if ( CAN_SendMessage((idtype & ELUA_CAN_ID_EXT) ? CAN_TX_EXT_MSGOBJ : CAN_TX_STD_MSGOBJ, &TxCanMsg) == SUCCESS )
    return PLATFORM_OK;
  return PLATFORM_ERR;
}

int platform_can_recv( unsigned id, u32 *canid, u8 *idtype, u8 *len, u8 *data )
{
  // wait for a message
  if ( frame_received_flag != 0){
    *canid = ( u32 ) RxCanMsg.Id;
    *idtype = RxCanMsg.IdType;
    *len = RxCanMsg.Dlc;
    memcpy( data, RxCanMsg.Data, *len );
    frame_received_flag = 0;
    return PLATFORM_OK;
  }
  else
    return PLATFORM_UNDERFLOW;
}
#endif

// ****************************************************************************
// SPI

#define SPI_MAX_PRESCALER     ( 254 * 256 )
#define SPI_MIN_PRESCALER     2

u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits )
{
  const u32 basefreq = CPU_FREQUENCY;
  u32 prescaler, divider = 1, temp, mindiff = 0xFFFFFFFF, minp;
  GPIO_InitTypeDef  GPIO_InitStructure;
  SSP_InitTypeDef SSP_InitStructure;

  clock = UMIN( clock, basefreq >> 1 );
  prescaler = UMIN( basefreq / clock, SPI_MAX_PRESCALER );
  if( basefreq / prescaler > clock )
    prescaler ++;
  if( prescaler & 1 )
    prescaler ++;
  if( prescaler > 254 )
  {
    temp = prescaler;
    for( prescaler = minp = 2; prescaler <= 254; prescaler += 2 )
    {
      divider = temp / prescaler;
      if( divider <= 255 )
      {
        if( ABSDIFF( divider * prescaler, temp ) < mindiff )
        {
          mindiff = ABSDIFF( divider * prescaler, temp );
          minp = prescaler;
          if( mindiff == 0 )
            break;
        }
      }
    }
    prescaler = minp;
    divider = temp / prescaler;
  }

  if ( id == 0 )
  {
    // GPIO setup
    // Fixed assignment:
    // P2.4 - SCLK
    // P2.5 - MOSI
    // P2.6 - MISO
    // P2.7 - CS
    GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;
    GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt2;
    GPIO_Init(GPIO2, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_IPConnected = GPIO_IPConnected_Enable;
    GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1  ;
    GPIO_Init(GPIO2, &GPIO_InitStructure);

    // Actual SPI setup
    SSP_DeInit(SSP0);
    SSP_InitStructure.SSP_FrameFormat = SSP_FrameFormat_Motorola;
    SSP_InitStructure.SSP_Mode = SSP_Mode_Master;
    SSP_InitStructure.SSP_CPOL = cpol == 0 ? SSP_CPOL_Low : SSP_CPOL_High;
    SSP_InitStructure.SSP_CPHA = cpha == 0 ? SSP_CPHA_1Edge : SSP_CPHA_2Edge;
    SSP_InitStructure.SSP_DataSize = databits - 1;
    SSP_InitStructure.SSP_ClockRate = divider - 1;
    SSP_InitStructure.SSP_ClockPrescaler = prescaler;
    SSP_Init(SSP0, &SSP_InitStructure);

    // Enable peripheral   
    SSP_Cmd(SSP0, ENABLE);
  }
  else if ( id == 1 )
  {
    // GPIO setup
    // Fixed assignment:
    // P1.0 - SCLK
    // P1.1 - MOSI
    // P1.2 - MISO
    // P1.3 - CS
    GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;
    GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt3;
    GPIO_Init(GPIO1, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;
    GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt3;
    GPIO_Init(GPIO1, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_IPConnected = GPIO_IPConnected_Enable;
    GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1;
    GPIO_Init(GPIO1, &GPIO_InitStructure);

    // Actual SPI setup
    SSP_DeInit(SSP1);
    SSP_InitStructure.SSP_FrameFormat = SSP_FrameFormat_Motorola;
    SSP_InitStructure.SSP_Mode = SSP_Mode_Master;
    SSP_InitStructure.SSP_CPOL = cpol == 0 ? SSP_CPOL_Low : SSP_CPOL_High;
    SSP_InitStructure.SSP_CPHA = cpha == 0 ? SSP_CPHA_1Edge : SSP_CPHA_2Edge;
    SSP_InitStructure.SSP_DataSize = databits - 1;
    SSP_InitStructure.SSP_ClockRate = divider - 1;
    SSP_InitStructure.SSP_ClockPrescaler = prescaler;
    SSP_Init(SSP1, &SSP_InitStructure);

    // Enable peripheral   
    SSP_Cmd(SSP1, ENABLE);
  }
  
  // All done
  return basefreq / ( prescaler * divider );
}

spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data )
{
  if (id == 0)
  {
    // Send byte through the SSP0 peripheral
    SSP0->DR = data;
    // Loop while Transmit FIFO is full
    while(SSP_GetFlagStatus(SSP0, SSP_FLAG_TxFifoEmpty) == RESET);
    // Loop while Receive FIFO is empty
    while(SSP_GetFlagStatus(SSP0, SSP_FLAG_RxFifoNotEmpty) == RESET); 
    // Return the byte read from the SSP bus
    return SSP0->DR;  
  }
  else
  {
    // Send byte through the SSP1 peripheral
    SSP1->DR = data;
    // Loop while Transmit FIFO is full
    while(SSP_GetFlagStatus(SSP1, SSP_FLAG_TxFifoEmpty) == RESET);
    // Loop while Receive FIFO is empty
    while(SSP_GetFlagStatus(SSP1, SSP_FLAG_RxFifoNotEmpty) == RESET); 
    // Return the byte read from the SSP bus
    return SSP1->DR;  
  }
}

void platform_spi_select( unsigned id, int is_select )
{
  id = id;
  is_select = is_select;
}


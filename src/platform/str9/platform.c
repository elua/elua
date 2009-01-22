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

// We define here the UART used by this porting layer
#define STR9_UART         UART1

// ****************************************************************************
// Platform initialization
static const GPIO_TypeDef* port_data[] = { GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, GPIO9 };
static const TIM_TypeDef* timer_data[] = { TIM0, TIM1, TIM2, TIM3 };

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
  /* Set the PCLK Clock to MCLK/8 */
  SCU_PCLKDivisorConfig(SCU_PCLK_Div8);
  /* Set the HCLK Clock to MCLK */
  SCU_HCLKDivisorConfig(SCU_HCLK_Div1);
  
  /* Enable VIC clock */
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
    
  // Initialize VIC
  VIC_DeInit();
  
  // UART setup (only STR9_UART is used in this example)
  platform_uart_setup( CON_UART_ID, CON_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );

  // Initialize timers
  for( i = 0; i < 4; i ++ )
  {
    base = ( TIM_TypeDef* )timer_data[ i ];
    TIM_DeInit( base );
    TIM_StructInit( &tim );
    tim.TIM_Clock_Source = TIM_CLK_APB;
    tim.TIM_Prescaler = 255;      
    TIM_Init( base, &tim );    
    TIM_CounterCmd( base, TIM_START );
  }
  
  cmn_platform_init();

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

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  UART_InitTypeDef UART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
      
  id = id;
  
  // First configure GPIO
  // RX: GPIO3.2
  GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
  GPIO_InitStructure.GPIO_IPConnected = GPIO_IPConnected_Enable;
  GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1  ;
  GPIO_Init (GPIO3, &GPIO_InitStructure);
  // TX: GPIO3.3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt2  ;
  GPIO_Init (GPIO3, &GPIO_InitStructure);
    
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

  UART_DeInit(STR9_UART);
  UART_Init(STR9_UART, &UART_InitStructure);
  UART_Cmd(STR9_UART, ENABLE);
  
  return baud;
}

void platform_uart_send( unsigned id, u8 data )
{
  id = id;
//  while( UART_GetFlagStatus( STR9_UART, UART_FLAG_TxFIFOFull ) == SET );
  UART_SendData( STR9_UART, data );
  while( UART_GetFlagStatus( STR9_UART, UART_FLAG_TxFIFOFull ) != RESET );  
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  if( timeout == 0 )
  {
    // Return data only if already available
    if( UART_GetFlagStatus( STR9_UART, UART_FLAG_RxFIFOEmpty ) != SET )
      return UART_ReceiveData( STR9_UART );
    else
      return -1;
  }
  while( UART_GetFlagStatus( STR9_UART, UART_FLAG_RxFIFOEmpty ) == SET );
  return UART_ReceiveData( STR9_UART );
}

// ****************************************************************************
// Timer

// Helper: get timer clock
static u32 platform_timer_get_clock( unsigned id )
{
  return ( SCU_GetPCLKFreqValue() * 1000 ) / ( TIM_GetPrescalerValue( ( TIM_TypeDef* )timer_data[ id ] ) + 1 );
}

// Helper: set timer clock
static u32 platform_timer_set_clock( unsigned id, u32 clock )
{
  u32 baseclk = SCU_GetPCLKFreqValue() * 1000;
  TIM_TypeDef* base = ( TIM_TypeDef* )timer_data[ id ];      
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

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  TIM_TypeDef* base = ( TIM_TypeDef* )timer_data[ id ];  
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
      
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  TIM_TypeDef* base = ( TIM_TypeDef* )timer_data[ id ];  

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
// CPU functions

extern void enable_ints();
extern void disable_ints();

void platform_cpu_enable_interrupts()
{
  enable_ints();
}

void platform_cpu_disable_interrupts()
{
  disable_ints();
}

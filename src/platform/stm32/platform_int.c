// STM32 interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

// Platform-specific headers
#include "stm32f10x.h"

// ****************************************************************************
// Interrupt handlers

extern USART_TypeDef *const stm32_usart[];

static void all_usart_irqhandler( int resnum )
{
  cmn_int_handler( INT_UART_RX, resnum );
  USART_ClearITPendingBit( stm32_usart[ resnum ], USART_IT_RXNE );  
}

void USART1_IRQHandler()
{
  all_usart_irqhandler( 0 );
}

void USART2_IRQHandler()
{
  all_usart_irqhandler( 1 );
}

void USART3_IRQHandler()
{
  all_usart_irqhandler( 2 );
}

void UART4_IRQHandler()
{
  all_usart_irqhandler( 3 );
}

void UART5_IRQHandler()
{
  all_usart_irqhandler( 4 );
}

// ****************************************************************************
// Interrupt: INT_TMR_MATCH

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  return PLATFORM_INT_NOT_HANDLED;
}

static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  return PLATFORM_INT_NOT_HANDLED;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
  return PLATFORM_INT_NOT_HANDLED;
}

// ****************************************************************************
// Interrupt: INT_UART_RX

static int int_uart_rx_get_status( elua_int_resnum resnum )
{
  return USART_GetITStatus( stm32_usart[ resnum ], USART_IT_RXNE ) == SET ? 1 : 0;
}

static int int_uart_rx_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_uart_rx_get_status( resnum );
  USART_ITConfig( stm32_usart[ resnum ], USART_IT_RXNE, status == PLATFORM_CPU_ENABLE ? ENABLE : DISABLE );
  return prev;
}

static int int_uart_rx_get_flag( elua_int_resnum resnum, int clear )
{
  int status = USART_GetFlagStatus( stm32_usart[ resnum ], USART_FLAG_RXNE ) == SET ? 1 : 0;
  if( clear )
    USART_ClearFlag( stm32_usart[ resnum ], USART_FLAG_RXNE );
  return status;
}

// ****************************************************************************
// Initialize interrupt subsystem

// UART IRQ table
#if defined( STM32F10X_LD )
static const u8 uart_irq_table[] = { USART1_IRQn, USART2_IRQn };
#elseif defined( STM32F10X_MD )
static const u8 uart_irq_table[] = { USART1_IRQn, USART2_IRQn, USART3_IRQn };
#else // high density devices
static const u8 uart_irq_table[] = { USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn, UART5_IRQn };
#endif

void platform_int_init()
{
  NVIC_InitTypeDef nvic_init_structure;
  unsigned i;
  
  // Enable all USART interrupts in the NVIC

  nvic_init_structure.NVIC_IRQChannelSubPriority = 2;
  nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;  
  for( i = 0; i < sizeof( uart_irq_table ) / sizeof( u8 ); i ++ )
  {
    nvic_init_structure.NVIC_IRQChannel = uart_irq_table[ i ];
    NVIC_Init( &nvic_init_structure );
  }  
}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag },
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag }  
};

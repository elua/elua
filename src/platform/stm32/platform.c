// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "uip_arp.h"
#include "elua_uip.h"
#include "uip-conf.h"
#include "platform_conf.h"
#include "common.h"

// Platform specific includes
#include "stm32f10x_lib.h"
#include "stm32f10x_map.h"
#include "stm32f10x_type.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_dbgmcu.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_systick.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_conf.h"
#include "systick.h"

// Clock data
// IMPORTANT: if you change these, make sure to modify RCC_Configuration() too!
#define HCLK        ( HSE_Value * 9 )
#define PCLK1_DIV   2
#define PCLK2_DIV   1

// ****************************************************************************
// Platform initialization

// forward dcls
static void RCC_Configuration(void);
static void NVIC_Configuration(void);

static void timers_init();
static void uarts_init();
static void pios_init();

int platform_init()
{
  // Set the clocking to run from PLL
  RCC_Configuration();

  // Setup IRQ's
  NVIC_Configuration();

  // Enable SysTick timer.
  SysTick_Config();

  // Setup PIO
  pios_init();

  // Setup UARTs
  uarts_init();
  
  // Setup timers
  timers_init();

  cmn_platform_init();

  // All done
  return PLATFORM_OK;
}

// ****************************************************************************
// Clocks
// Shared by all STM32 devices.
// TODO: Fix to handle different crystal frequencies and CPU frequencies.

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);

    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

// ****************************************************************************
// NVIC
// Shared by all STM32 devices.

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the nested vectored interrupt controller.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Configuration(void)
{
  NVIC_DeInit();

#ifdef  VECT_TAB_RAM
  /* Set the Vector Table base location at 0x20000000 */
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Configure the SysTick handler priority */
  NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 0, 0);
}

// ****************************************************************************
// PIO
// This is pretty much common code to all STM32 devices.
// todo: Needs updates to support different processor lines.
static GPIO_TypeDef * const pio_port[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG };
static const u32 pio_port_clk[]        = { RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOB, RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD, RCC_APB2Periph_GPIOE, RCC_APB2Periph_GPIOF, RCC_APB2Periph_GPIOG };

static void pios_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  int port;

  for( port = 0; port < NUM_PIO; port++ )
  {
    // Enable clock to port.
    RCC_APB2PeriphClockCmd(pio_port_clk[port], ENABLE);

    // Default all port pins to input and enable port.
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(pio_port[port], &GPIO_InitStructure);
  }
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1;
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_TypeDef * base = pio_port[ port ];

  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:
      GPIO_Write(base, pinmask);
      break;

    case PLATFORM_IO_PIN_SET:
      GPIO_SetBits(base, pinmask);
      break;

    case PLATFORM_IO_PIN_CLEAR:
      GPIO_ResetBits(base, pinmask);
      break;

    case PLATFORM_IO_PORT_DIR_INPUT:
      pinmask = GPIO_Pin_All;
    case PLATFORM_IO_PIN_DIR_INPUT:
      GPIO_InitStructure.GPIO_Pin  = pinmask;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

      GPIO_Init(base, &GPIO_InitStructure);
      break;

    case PLATFORM_IO_PORT_DIR_OUTPUT:
      pinmask = GPIO_Pin_All;
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      GPIO_InitStructure.GPIO_Pin   = pinmask;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

      GPIO_Init(base, &GPIO_InitStructure);
      break;

    case PLATFORM_IO_PORT_GET_VALUE:
      retval = pinmask == PLATFORM_IO_READ_IN_MASK ? GPIO_ReadInputData(base) : GPIO_ReadOutputData(base);
      break;

    case PLATFORM_IO_PIN_GET:
      retval = GPIO_ReadInputDataBit(base, pinmask);
      break;

    case PLATFORM_IO_PIN_PULLUP:
      GPIO_InitStructure.GPIO_Pin   = pinmask;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;

      GPIO_Init(base, &GPIO_InitStructure);
      break;

    case PLATFORM_IO_PIN_PULLDOWN:
      GPIO_InitStructure.GPIO_Pin   = pinmask;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;

      GPIO_Init(base, &GPIO_InitStructure);
      break;

    case PLATFORM_IO_PIN_NOPULL:
      GPIO_InitStructure.GPIO_Pin   = pinmask;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;

      GPIO_Init(base, &GPIO_InitStructure);
      break;

    default:
      retval = 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// UART
// TODO: Support timeouts.

// All possible STM32 uarts defs
static USART_TypeDef *const usart[] =          { USART1, USART2, USART3, UART4, UART5 };
static GPIO_TypeDef *const usart_gpio_rx_port[] = { GPIOA, GPIOA, GPIOB, GPIOC, GPIOD };
static GPIO_TypeDef *const usart_gpio_tx_port[] = { GPIOA, GPIOA, GPIOB, GPIOC, GPIOC };
static const u16 usart_gpio_rx_pin[] = { GPIO_Pin_10, GPIO_Pin_3, GPIO_Pin_11, GPIO_Pin_11, GPIO_Pin_2 };
static const u16 usart_gpio_tx_pin[] = { GPIO_Pin_9, GPIO_Pin_2, GPIO_Pin_10, GPIO_Pin_10, GPIO_Pin_12 };

static void usart_init(u32 id, USART_InitTypeDef * initVals)
{
  /* Configure USART IO */
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USART Tx Pin as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = usart_gpio_tx_pin[id];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(usart_gpio_tx_port[id], &GPIO_InitStructure);

  /* Configure USART Rx Pin as input floating */
  GPIO_InitStructure.GPIO_Pin = usart_gpio_rx_pin[id];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(usart_gpio_rx_port[id], &GPIO_InitStructure);

  /* Configure USART */
  USART_Init(usart[id], initVals);

  /* Enable USART1 Receive and Transmit interrupts */
  //USART_ITConfig(usart[id], USART_IT_RXNE, ENABLE);
  //USART_ITConfig(usart[id], USART_IT_TXE, ENABLE);

  /* Enable USART */
  USART_Cmd(usart[id], ENABLE);
}

static void uarts_init()
{
  USART_InitTypeDef USART_InitStructure;

  // Enable clocks.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

  // Configure the U(S)ART

  USART_InitStructure.USART_BaudRate = CON_UART_SPEED;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  usart_init(CON_UART_ID, &USART_InitStructure);
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate = baud;

  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  switch( databits )
  {
    case 5:
    case 6:
    case 7:
    case 8:
      USART_InitStructure.USART_WordLength = USART_WordLength_8b;
      break;
    case 9:
      USART_InitStructure.USART_WordLength = USART_WordLength_9b;
      break;
    default:
      USART_InitStructure.USART_WordLength = USART_WordLength_8b;
      break;
  }

  switch (stopbits)
  {
    case PLATFORM_UART_STOPBITS_1:
      USART_InitStructure.USART_StopBits = USART_StopBits_1;
      break;
    case PLATFORM_UART_STOPBITS_2:
      USART_InitStructure.USART_StopBits = USART_StopBits_2;
      break;
    default:
      USART_InitStructure.USART_StopBits = USART_StopBits_2;
      break;
  }

  switch (parity)
  {
    case PLATFORM_UART_PARITY_EVEN:
      USART_InitStructure.USART_Parity = USART_Parity_Even;
      break;
    case PLATFORM_UART_PARITY_ODD:
      USART_InitStructure.USART_Parity = USART_Parity_Odd;
      break;
    default:
      USART_InitStructure.USART_Parity = USART_Parity_No;
      break;
  }

  usart_init(id, &USART_InitStructure);

  return TRUE;
}

void platform_uart_send( unsigned id, u8 data )
{
  while(USART_GetFlagStatus(usart[id], USART_FLAG_TXE) == RESET)
  {
  }
  USART_SendData(usart[id], data);
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
  if( timeout == 0 )
  {
    if (USART_GetFlagStatus(usart[id], USART_FLAG_RXNE) == RESET)
      return -1;
    else
      return USART_ReceiveData(usart[id]);
  }
  // Receive char blocking
  while(USART_GetFlagStatus(usart[id], USART_FLAG_RXNE) == RESET);
  return USART_ReceiveData(usart[id]);
}

// ****************************************************************************
// Timers

// We leave out TIM6/TIM for now, as they are dedicated
static TIM_TypeDef * const timer[] = { TIM1, TIM2, TIM3, TIM4, TIM5, TIM8 };
#define TIM_GET_BASE_CLK( id ) ( ( id ) == 0 || ( id ) == 5 ? ( HCLK / PCLK2_DIV ) : ( HCLK / PCLK1_DIV ) )
#define TIM_STARTUP_CLOCK       50000

static void timers_init()
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  unsigned i;

  // Enable clocks.
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM1, ENABLE );  
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM5, ENABLE );
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM8, ENABLE );  

  // Configure timers
  for( i = 0; i < NUM_TIMER; i ++ )
  {
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_GET_BASE_CLK( i ) / TIM_STARTUP_CLOCK;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
    TIM_TimeBaseInit( timer[ i ], &TIM_TimeBaseStructure );
    TIM_Cmd( timer[ i ], ENABLE );
  }
}

static u32 timer_get_clock( unsigned id )
{
  TIM_TypeDef* ptimer = timer[ id ];

  return TIM_GET_BASE_CLK( id ) / ( TIM_GetPrescaler( ptimer ) + 1 );
}

static u32 timer_set_clock( unsigned id, u32 clock )
{
  TIM_TypeDef *ptimer = timer[ id ];
  u16 pre;
  
  pre = TIM_GET_BASE_CLK( id ) / clock;
  TIM_PrescalerConfig( ptimer, pre, TIM_PSCReloadMode_Immediate );
  return TIM_GET_BASE_CLK( id ) / pre;
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  TIM_TypeDef *ptimer = timer[ id ];
  volatile unsigned dummy;
  timer_data_type final;

  final = ( ( u64 )delay_us * timer_get_clock( id ) ) / 1000000;
  TIM_SetCounter( ptimer, 0 );
  for( dummy = 0; dummy < 200; dummy ++ );
  while( TIM_GetCounter( ptimer ) < final );
}

u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  TIM_TypeDef *ptimer = timer[ id ];
  volatile unsigned dummy;

  data = data;
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      TIM_SetCounter( ptimer, 0 );
      for( dummy = 0; dummy < 200; dummy ++ );
      break;

    case PLATFORM_TIMER_OP_READ:
      res = TIM_GetCounter( ptimer );
      break;

    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFF );
      break;

    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;

    case PLATFORM_TIMER_OP_SET_CLOCK:
      res = timer_set_clock( id, data );
      break;

    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = timer_get_clock( id );
      break;

  }
  return res;
}

// *****************************************************************************
// CPU specific functions

void platform_cpu_enable_interrupts()
{
  //IntMasterEnable();
}

void platform_cpu_disable_interrupts()
{
  //IntMasterDisable();
}

u32 platform_s_cpu_get_frequency()
{
  return HCLK;
}


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

#include "systick.h"

#define STM32_USE_PIO
#define STM32_USE_USART

void exit(int ret)
{
  while(1);
}

// ****************************************************************************
// Platform initialization

// forward dcls
static void RCC_Configuration(void);
static void NVIC_Configuration(void);

static void timers_init();
static void uarts_init();
static void spis_init();
static void pios_init();
static void pwms_init();
static void eth_init();

int platform_init()
{ 
  // Set the clocking to run from PLL
  RCC_Configuration();

  // Setup IRQ's
  NVIC_Configuration();

  // Enable SysTick timer.
  SysTick_Config();

#ifdef STM32_USE_PIO
  // Setup PIO
  pios_init();
#endif

#ifdef STM32_USE_SPI
  // Setup SPIs
  //spis_init();
#endif

#ifdef STM32_USE_USART
  // Setup UARTs
  uarts_init();
#endif

  // Setup timers
  //timers_init();        

  // Setup PWMs
  //pwms_init();                 

  // Setup ethernet (TCP/IP)
  //eth_init();

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
  NVIC_InitTypeDef NVIC_InitStructure;

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
#ifdef STM32_USE_PIO
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
      retval = GPIO_ReadInputData(base);
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
#endif

#ifdef STM32_USE_SPI
// ****************************************************************************
// SPI
// TODO: Just about everything.

static const u32 spi_base[] = { SSI0_BASE, SSI1_BASE };
static const u32 spi_sysctl[] = { SYSCTL_PERIPH_SSI0, SYSCTL_PERIPH_SSI1 };
static const u32 spi_gpio_base[] = { GPIO_PORTA_BASE | GPIO_PORTE_BASE };
static const u8 spi_gpio_pins[] = { GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
                                    GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 };
//                                  SSIxClk      SSIxFss      SSIxRx       SSIxTx
static const u8 spi_gpio_clk_pin[] = { GPIO_PIN_2, GPIO_PIN_0 };
#define SPIS_COUNT 		1

static void spis_init()
{
  unsigned i;

  for( i = 0; i < SPIS_COUNT; i ++ )
  {
    SysCtlPeripheralEnable(spi_sysctl[ i ]);
  }
}

int platform_spi_exists( unsigned id )
{
  return id < SPIS_COUNT;
}

u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits )
{
  unsigned protocol;
  
  if( cpol == 0 )
    protocol = cpha ? SSI_FRF_MOTO_MODE_1 : SSI_FRF_MOTO_MODE_0;
  else
    protocol = cpha ? SSI_FRF_MOTO_MODE_3 : SSI_FRF_MOTO_MODE_2;
  mode = mode == PLATFORM_SPI_MASTER ? SSI_MODE_MASTER : SSI_MODE_SLAVE;  
  SSIDisable( spi_base[ id ] );

  GPIOPinTypeSSI( spi_gpio_base[ id ], spi_gpio_pins[ id ] );
  
  // FIXME: not sure this is always "right"
  GPIOPadConfigSet(spi_gpio_base[ id ], spi_gpio_clk_pin[ id ], GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);    

  SSIConfigSetExpClk( spi_base[ id ], SysCtlClockGet(), protocol, mode, clock, databits );
  SSIEnable( spi_base[ id ] );
  return clock;
}

spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data )
{
  SSIDataPut( spi_base[ id ], data );
  SSIDataGet( spi_base[ id ], &data );
  return data;
}

void platform_spi_select( unsigned id, int is_select )
{
  // This platform doesn't have a hardware SS pin, so there's nothing to do here  
  id = id;
  is_select = is_select;
}
#endif

#ifdef STM32_USE_USART
// ****************************************************************************
// UART
// TODO: Support timeouts.

// All possible STM32 uarts defs
static USART_TypeDef * usart[] =          { USART1, USART2, USART3, UART4 };
static GPIO_TypeDef * usart_gpio_port[] = { GPIOA, GPIOA, GPIOB, GPIOC };
static const u16 usart_gpio_tx_pin[] = { GPIO_Pin_9, GPIO_Pin_2, GPIO_Pin_10, GPIO_Pin_10 };
static const u16 usart_gpio_rx_pin[] = { GPIO_Pin_10, GPIO_Pin_3, GPIO_Pin_11, GPIO_Pin_11 };
//static const u32 uart_sysctl[] = { SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2 };
//static const u32 uart_gpio_base[] = { _BASE, GPIO_PORTD_BASE, GPIO_PORTG_BASE };
//static const u8 uart_gpio_pins[] = { GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_0 | GPIO_PIN_1 };

static void usart_init(u32 id, USART_InitTypeDef * initVals)
{
  /* Configure USART IO */
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USART Tx Pin as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = usart_gpio_tx_pin[id];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(usart_gpio_port[id], &GPIO_InitStructure);

  /* Configure USART Rx Pin as input floating */
  GPIO_InitStructure.GPIO_Pin = usart_gpio_rx_pin[id];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(usart_gpio_port[id], &GPIO_InitStructure);

  /* Configure USART */
  USART_Init(usart[id], initVals);
  
  /* Enable USART1 Receive and Transmit interrupts */
  //USART_ITConfig(usart[id], USART_IT_RXNE, ENABLE);
  //USART_ITConfig(usart[id], USART_IT_TXE, ENABLE);

  /* Enable the USART1 */
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
  
  // Configure the U(S)ART for 115,200, 8-N-1 operation.

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

int platform_s_uart_recv( unsigned id, unsigned timer_id, int timeout )
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
#endif

#ifdef STM32_USE_TIMERS
// ****************************************************************************
// Timers
// Same on LM3S8962 and LM3S6965

// All possible LM3S timers defs
static TIM_TypeDef * timer[] = { TIM2, TIM3, TIM4, TIM5};

static void timers_init()
{
#if 0
  unsigned i;

  for( i = 0; i < NUM_TIMER; i ++ )
  {
	SysCtlPeripheralEnable(timer_sysctl[ i ]);
    TimerConfigure(timer_base[ i ], TIMER_CFG_32_BIT_PER);
    TimerEnable(timer_base[ i ], TIMER_A);
  }
#endif
}

static u32 platform_timer_get_clock(unsigned id)
{
  RCC_ClocksTypeDef Clocks;

  RCC_GetClocksFreq(&Clocks);

  return Clocks.PCLK1_Frequency / (TIM_GetPrescaler(timer[id]) + 1)
}

static u32 platform_timer_set_clock(unsigned id, u32 clock)
{
  RCC_ClocksTypeDef Clocks;
  u32 pclk, clkdiv;
  u64 tmp;

  RCC_GetClocksFreq(&Clocks);

  pclk   = Clocks.PCLK1_Frequency; // Get peripheral bus clock frequency.
  tmp    = ((u64)pclk << 16) / clock; // Convert to u32.16 fixed point and calculate prescaler divisor
  clkdiv = ((tmp & 0x8000) ? (tmp + 0x10000) : tmp) >> 16;  // Round up or down and convert back to u32.0
  if (clkdiv > 0x10000)  // Saturate to u16 (+1 for a clkdiv value of 1 is a reg value of 0)
      clkdiv = 0x10000;
  
  TIM_PrescalerConfig(timer[id], clkdiv - 1, TIM_PSCReloadMode_Immediate); // Update timer prescaler immediately
  
  return pclk / clkdiv; // Return actual clock rate used.
}

void platform_s_timer_delay( unsigned id, u32 delay_us )
{
  timer_data_type final;
  u32 base = timer_base[ id ];
  
  final = 0xFFFFFFFF - ( ( ( u64 )delay_us * SysCtlClockGet() ) / 1000000 );
  TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
  while( TimerValueGet( base, TIMER_A ) > final );
}
      
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  u32 base = timer_base[ id ]; 
  
  data = data;
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      res = 0xFFFFFFFF;
      TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = TimerValueGet( base, TIMER_A );
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = SysCtlClockGet();
      break;
      
  }
  return res;
}
#else
u32 platform_s_timer_op( unsigned id, int op, u32 data )
{
  return 0;
}
#endif

#ifdef STM32_USE_PWM
// ****************************************************************************
// PWMs
// TODO: Everything.

#define PLATFORM_NUM_PWMS               6

// SYSCTL div data and actual div factors
const static u32 pwm_div_ctl[] = { SYSCTL_PWMDIV_1, SYSCTL_PWMDIV_2, SYSCTL_PWMDIV_4, SYSCTL_PWMDIV_8, SYSCTL_PWMDIV_16, SYSCTL_PWMDIV_32, SYSCTL_PWMDIV_64 };
const static u8 pwm_div_data[] = { 1, 2, 4, 8, 16, 32, 64 };
// Port/pin information for all channels
const static u32 pwm_ports[] =  { GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE };
const static u8 pwm_pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1 };
// PWM generators
const static u16 pwm_gens[] = { PWM_GEN_0, PWM_GEN_1, PWM_GEN_2 };
// PWM outputs
const static u16 pwm_outs[] = { PWM_OUT_0, PWM_OUT_1, PWM_OUT_2, PWM_OUT_3, PWM_OUT_4, PWM_OUT_5 };

static void pwms_init()
{
  SysCtlPeripheralEnable( SYSCTL_PERIPH_PWM );
  SysCtlPWMClockSet( SYSCTL_PWMDIV_1 );
}

// Helper function: return the PWM clock
static u32 platform_pwm_get_clock()
{
  unsigned i;
  u32 clk;
  
  clk = SysCtlPWMClockGet();
  for( i = 0; i < sizeof( pwm_div_ctl ) / sizeof( u32 ); i ++ )
    if( clk == pwm_div_ctl[ i ] )
      break;
  return SysCtlClockGet() / pwm_div_data[ i ];
}

// Helper function: set the PWM clock
static u32 platform_pwm_set_clock( u32 clock )
{
  unsigned i, min_i;
  u32 sysclk;
  
  sysclk = SysCtlClockGet();
  for( i = min_i = 0; i < sizeof( pwm_div_data ) / sizeof( u8 ); i ++ )
    if( ABSDIFF( clock, sysclk / pwm_div_data[ i ] ) < ABSDIFF( clock, sysclk / pwm_div_data[ min_i ] ) )
      min_i = i;
  SysCtlPWMClockSet( pwm_div_ctl[ min_i ] );
  return sysclk / pwm_div_data[ min_i ];
}

int platform_pwm_exists( unsigned id )
{
  return id < PLATFORM_NUM_PWMS; 
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk = platform_pwm_get_clock();
  u32 period;
  
  // Set pin as PWM
  GPIOPinTypePWM( pwm_ports[ id ], pwm_pins[ id ] );
  // Compute period
  period = pwmclk / frequency;
  // Set the period
  PWMGenConfigure( PWM_BASE, pwm_gens[ id >> 1 ], PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC );
  PWMGenPeriodSet( PWM_BASE, pwm_gens[ id >> 1 ], period );
  // Set duty cycle
  PWMPulseWidthSet( PWM_BASE, pwm_outs[ id ], ( period * duty ) / 100 );
  // Return actual frequency
  return pwmclk / period;
}

u32 platform_pwm_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  
  switch( op )
  {
    case PLATFORM_PWM_OP_SET_CLOCK:
      res = platform_pwm_set_clock( data );
      break;
            
    case PLATFORM_PWM_OP_GET_CLOCK:
      res = platform_pwm_get_clock();
      break;
      
    case PLATFORM_PWM_OP_START:
      PWMOutputState( PWM_BASE, 1 << id, true );
      PWMGenEnable( PWM_BASE, pwm_gens[ id >> 1 ] );
      break;
      
    case PLATFORM_PWM_OP_STOP:
      PWMOutputState( PWM_BASE, 1 << id, false );
      PWMGenDisable( PWM_BASE, pwm_gens[ id >> 1 ] );
      break;
  }
  
  return res;
}
#endif

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
  RCC_ClocksTypeDef clocks;

  RCC_GetClocksFreq(&clocks);

  return clocks.HCLK_Frequency;
}

u32 platform_pclk1_get_frequency()
{
  RCC_ClocksTypeDef clocks;

  RCC_GetClocksFreq(&clocks);

  return clocks.PCLK1_Frequency;
}

u32 platform_pclk2_get_frequency()
{
  RCC_ClocksTypeDef clocks;

  RCC_GetClocksFreq(&clocks);

  return clocks.PCLK2_Frequency;
}

// ****************************************************************************
// Ethernet functions

static void eth_init()
{
#ifdef BUILD_UIP
  u32 user0, user1, temp;
  static struct uip_eth_addr sTempAddr;     
  
  // Enable and reset the controller
  SysCtlPeripheralEnable( SYSCTL_PERIPH_ETH );
  SysCtlPeripheralReset( SYSCTL_PERIPH_ETH );
  
  // Enable Ethernet LEDs
  GPIODirModeSet( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_HW );
  GPIOPadConfigSet( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );

  // Configure SysTick for a periodic interrupt.
  SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
  SysTickEnable();
  SysTickIntEnable();
  
  // Intialize the Ethernet Controller and disable all Ethernet Controller interrupt sources.
  EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                     ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
  temp = EthernetIntStatus(ETH_BASE, false);
  EthernetIntClear(ETH_BASE, temp);  
  
  // Initialize the Ethernet Controller for operation.
  EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

  // Configure the Ethernet Controller for normal operation.
  // - Full Duplex
  // - TX CRC Auto Generation
  // - TX Padding Enabled
  EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                               ETH_CFG_TX_PADEN));

  // Enable the Ethernet Controller.
  EthernetEnable(ETH_BASE);

  // Enable the Ethernet interrupt.
  IntEnable(INT_ETH);

  // Enable the Ethernet RX Packet interrupt source.
  EthernetIntEnable(ETH_BASE, ETH_INT_RX);  

  // Enable all processor interrupts.
  IntMasterEnable();

  // Configure the hardware MAC address for Ethernet Controller filtering of
  // incoming packets.
  //
  // For the Ethernet Eval Kits, the MAC address will be stored in the
  // non-volatile USER0 and USER1 registers.  These registers can be read
  // using the FlashUserGet function, as illustrated below.
  FlashUserGet(&user0, &user1);
  
  // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
  // address needed to program the hardware registers, then program the MAC
  // address into the Ethernet Controller registers.
  sTempAddr.addr[0] = ((user0 >>  0) & 0xff);
  sTempAddr.addr[1] = ((user0 >>  8) & 0xff);
  sTempAddr.addr[2] = ((user0 >> 16) & 0xff);
  sTempAddr.addr[3] = ((user1 >>  0) & 0xff);
  sTempAddr.addr[4] = ((user1 >>  8) & 0xff);
  sTempAddr.addr[5] = ((user1 >> 16) & 0xff);  

  // Program the hardware with it's MAC address (for filtering).
  EthernetMACAddrSet(ETH_BASE, (unsigned char *)&sTempAddr);  

  // Initialize the eLua uIP layer
  elua_uip_init( &sTempAddr );
#endif
}

#ifdef BUILD_UIP
static int eth_timer_fired;

void platform_eth_send_packet( const void* src, u32 size )
{
  EthernetPacketPut( ETH_BASE, uip_buf, uip_len );    
}

u32 platform_eth_get_packet_nb( void* buf, u32 maxlen )
{
  return EthernetPacketGetNonBlocking( ETH_BASE, uip_buf, sizeof( uip_buf ) );
}

void platform_eth_force_interrupt()
{
  HWREG( NVIC_SW_TRIG) |= INT_ETH - 16;  
}

u32 platform_eth_get_elapsed_time()
{
  if( eth_timer_fired )
  {
    eth_timer_fired = 0;
    return SYSTICKMS;
  }
  else
    return 0;
}

#if 0
void SysTickHandler(void)
{
  // Indicate that a SysTick interrupt has occurred.
  eth_timer_fired = 1;

  // Generate a fake Ethernet interrupt.  This will perform the actual work
  // of incrementing the timers and taking the appropriate actions.
  platform_eth_force_interrupt();
}
#endif

void EthernetIntHandler()
{
  u32 temp;
  
  // Read and Clear the interrupt.
  temp = EthernetIntStatus( ETH_BASE, false );
  EthernetIntClear( ETH_BASE, temp );

  // Call the UIP main loop  
  elua_uip_mainloop();
}

#else  // #ifdef ELUA_UIP

#if 0
void SysTickHandler()
{
}
#endif

void EthernetIntHandler()
{
}
#endif // #ifdef ELUA_UIP

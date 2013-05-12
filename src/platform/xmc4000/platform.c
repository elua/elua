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
#include "DAVE3.h"
#include "UART001.h"

#define SYSTICKHZ             10
#define SYSTICKMS             ( 1000 / SYSTICKHZ )

#define SYSTM001_SYS_CORE_CLOCK  120U 
#define SYSTM001_SYSTICK_INTERVAL SYSTICKMS
// ****************************************************************************
// Platform initialization

extern const UART001_HandleType UART001_Handle0;

int platform_init()
{
  DAVE_Init();
  cmn_platform_init();

  // Setup system timer
  cmn_systimer_set_base_freq( SYSTM001_SYS_CORE_CLOCK * 1000000 );
  cmn_systimer_set_interrupt_freq( SYSTICKHZ );

  SysTick_Config((uint32_t)(SYSTM001_SYSTICK_INTERVAL * SYSTM001_SYS_CORE_CLOCK * 1000U));
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),10,0));

  return PLATFORM_OK;
}

void SysTick_Handler( void )
{
  // Handle system timer call
  cmn_systimer_periodic();
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


timer_data_type platform_s_timer_op( unsigned id, int op,timer_data_type data )
{
  return 0;
}

void platform_s_uart_send( unsigned id, u8 data )
{
  u16 temp = ( u16 )data;

  UART001_WriteDataMultiple( &UART001_Handle0, &temp, 1 );
}

int platform_s_uart_recv( unsigned id, timer_data_type timeout )
{
  return UART001_ReadChar( &UART001_Handle0, timeout != 0 );
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  return 0;
}

void platform_s_timer_delay( unsigned id, timer_data_type delay_us )
{
}


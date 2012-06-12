// MBED interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

#include <stdio.h>

// Platform-specific headers
#include "LPC17xx.h"
#include "LPC17xx_timer.h"
#include "mbed_rtc.h"
#include "core_cm3.h"

// RTC Interrupt
void RTC_IRQHandler(void)
{
  // Add an elua interrupt
  cmn_int_handler( INT_RTC_ALARM, 0 );

  // Clear interrupt flag
  LPC_RTC->ILR = 2;
}

static int int_rtc_alarm_get_status( elua_int_resnum resnum )
{
  return NVIC_GetActive( RTC_IRQn );
}

static int int_rtc_alarm_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_rtc_alarm_get_status( resnum ); 

  if (status == PLATFORM_CPU_ENABLE)
  {
    // Enable alarm interrupt
    NVIC_EnableIRQ( RTC_IRQn );

    // Clear interrupt flag
    LPC_RTC->ILR = 2;
  }
  else
  {
    // Enable alarm interrupt
    NVIC_DisableIRQ( RTC_IRQn );
  }
  
  return prev;
}

static int int_rtc_alarm_get_flag( elua_int_resnum resnum, int clear )
{
  int status = (LPC_RTC->ILR & 2) != 0;

  if( clear )
    // Clear interrupt flag
    LPC_RTC->ILR = 2;

  return status;
}

// ****************************************************************************
// Timer interrupts
static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  switch (resnum)
  {
    case 0: return NVIC_GetActive( TIMER0_IRQn ); break;
    case 1: return NVIC_GetActive( TIMER1_IRQn ); break;
    case 2: return NVIC_GetActive( TIMER2_IRQn ); break;
    case 3: return NVIC_GetActive( TIMER3_IRQn ); break;
  }
}

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_tmr_match_get_status( resnum ); 

  if (status == PLATFORM_CPU_ENABLE)
  {
    // Enable timer interrupt & clear interrupt flag
    switch (resnum)
    {
      case 0: {
                LPC_TIM0->IR |= 1;
                NVIC_EnableIRQ( TIMER0_IRQn );
                break;
              }
      case 1: {
                LPC_TIM1->IR |= 1;
                NVIC_EnableIRQ( TIMER1_IRQn );
                break;
              }

      case 2: {
                LPC_TIM2->IR |= 1;
                NVIC_EnableIRQ( TIMER2_IRQn );
                break;
              }
      case 3: {
                LPC_TIM3->IR |= 1;
                NVIC_EnableIRQ( TIMER3_IRQn );
                break;
              }
    }

  }
  else
  {
    // Disable timer interrupt
    switch (resnum)
    {
      case 0: NVIC_DisableIRQ( TIMER0_IRQn ); break;
      case 1: NVIC_DisableIRQ( TIMER1_IRQn ); break;
      case 2: NVIC_DisableIRQ( TIMER2_IRQn ); break;
      case 3: NVIC_DisableIRQ( TIMER3_IRQn ); break;
    }
  }
  
  return prev;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
    switch (resnum)
    {
      case 0: return LPC_TIM0->IR & 1; break;
      case 1: return LPC_TIM1->IR & 1; break;
      case 2: return LPC_TIM2->IR & 1; break;
      case 3: return LPC_TIM3->IR & 1; break;
    }

    if (clear)
      switch (resnum)
      {
        case 0: LPC_TIM0->IR = 1; break;
        case 1: LPC_TIM1->IR = 1; break;
        case 2: LPC_TIM2->IR = 1; break;
        case 3: LPC_TIM3->IR = 1; break;
      }
}

static void tmr_int_handler( int id )
{
  /*
  TIM_ClearFlag( base, TIM_FLAG_OC1 );
  TIM_CounterCmd( base, TIM_CLEAR );
  if( id == VTMR_TIMER_ID )
  {
    cmn_virtual_timer_cb();
    cmn_systimer_periodic();
  }
  else
  */

  cmn_int_handler( INT_TMR_MATCH, id );

//  if( str9_timer_int_periodic_flag[ id ] != PLATFORM_TIMER_INT_CYCLIC )
//    TIM_ITConfig( base, TIM_IT_OC1, DISABLE );    
}

void TIMER0_IRQHandler(void)
{
  if ((LPC_TIM0->IR & 0x01) == 0x01) // if MR0 interrupt
  {
    LPC_TIM0->IR |= 1 << 0; // Clear MR0 interrupt flag
    tmr_int_handler( 0 );
  }
}

void TIMER1_IRQHandler(void)
{
  if ((LPC_TIM1->IR & 0x01) == 0x01) // if MR0 interrupt
  {
    LPC_TIM1->IR |= 1 << 0; // Clear MR0 interrupt flag
    tmr_int_handler( 1 );
  }
}

void TIMER2_IRQHandler(void)
{
  if ((LPC_TIM2->IR & 0x01) == 0x01) // if MR0 interrupt
  {
    LPC_TIM2->IR |= 1 << 0; // Clear MR0 interrupt flag
    tmr_int_handler( 2 );
  }
}

void TIMER3_IRQHandler(void)
{
  if ((LPC_TIM3->IR & 0x01) == 0x01) // if MR0 interrupt
  {
    LPC_TIM3->IR |= 1 << 0; // Clear MR0 interrupt flag
    tmr_int_handler( 3 );
  }
}


// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_rtc_alarm_set_status, int_rtc_alarm_get_status, int_rtc_alarm_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag }
};

// ****************************************************************************
// Interrupt initialization

void platform_int_init()
{
  // Clear RTC interrupt flag
  LPC_RTC->ILR = 2;
  
  // Clear TIMER interrupt flags
  TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
  TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
  TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
  TIM_ClearIntPending(LPC_TIM3, TIM_MR0_INT);
}


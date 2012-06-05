// MBED interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

#include <stdio.h>

// Platform-specific headers
#include "LPC17xx.h"
#include "mbed_rtc.h"

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

// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_rtc_alarm_set_status, int_rtc_alarm_get_status, int_rtc_alarm_get_flag }
};

// ****************************************************************************
// Interrupt initialization

void platform_int_init()
{
  // Clear RTC interrupt flag
  LPC_RTC->ILR = 2;
}


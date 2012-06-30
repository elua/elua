// Generic platform-wide header

#ifndef __PLATFORM_GENERIC_H__
#define __PLATFORM_GENERIC_H__

#include "type.h"
#include "stacks.h"
#include "stm32f10x.h"
#include "sermux.h"

#define PLATFORM_HAS_SYSTIMER

// *****************************************************************************
// Configuration data

// TODO: replace with a component
// ADC Configuration Params
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        4

#endif // #ifndef __PLATFORM_GENERIC_H__


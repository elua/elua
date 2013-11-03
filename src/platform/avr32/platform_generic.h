// Generic platform-wide header

#ifndef __PLATFORM_GENERIC_H__
#define __PLATFORM_GENERIC_H__

#define VTMR_CH               2    // Which hardware timer to use for VTMR

// If virtual timers are enabled, the last timer will be used only for them
#if VTMR_NUM_TIMERS > 0
#undef NUM_TIMER
#define NUM_TIMER             2
#endif

#define PLATFORM_HAS_SYSTIMER

#endif // #ifndef __PLATFORM_GENERIC_H__


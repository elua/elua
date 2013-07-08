// Platform customization header

#ifndef __PLATFORM_GENERIC_H__
#define __PLATFORM_GENERIC_H__

#define PLATFORM_HAS_SYSTIMER

#if NUM_CAN > 0
#define BUILD_CAN
#endif

// ID of the hardware timer used to implement physical timers
#define VTMR_TIMER_ID         3

#endif // #ifndef __PLATFORM_GENERIC_H__


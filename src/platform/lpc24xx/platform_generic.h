// Platform-wide configuration file, included by platform_conf.h

#ifndef __PLATFORM_GENERIC_H__
#define __PLATFORM_GENERIC_H__

#define PLATFORM_HAS_SYSTIMER

// If virtual timers are enabled, the last timer will be used only for them
#if VTMR_NUM_TIMERS > 0
#undef NUM_TIMER
#define NUM_TIMER             3
#endif

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#define PINSEL_BASE_ADDR      0xE002C000
#define IO_PINSEL0            ( PINSEL_BASE_ADDR + 0x00 )
#define IO_PINSEL1            ( PINSEL_BASE_ADDR + 0x04 )
#define IO_PINSEL2            ( PINSEL_BASE_ADDR + 0x08 )
#define IO_PINSEL3            ( PINSEL_BASE_ADDR + 0x0C )
#define IO_PINSEL4            ( PINSEL_BASE_ADDR + 0x10 )
#define IO_PINSEL5            ( PINSEL_BASE_ADDR + 0x14 )
#define IO_PINSEL6            ( PINSEL_BASE_ADDR + 0x18 )
#define IO_PINSEL7            ( PINSEL_BASE_ADDR + 0x1C )
#define IO_PINSEL8            ( PINSEL_BASE_ADDR + 0x20 )
#define IO_PINSEL9            ( PINSEL_BASE_ADDR + 0x24 )
#define IO_PINSEL10           ( PINSEL_BASE_ADDR + 0x28 )

#define PLATFORM_CPU_CONSTANTS_PLATFORM\
 _C( IO_PINSEL0 ),\
 _C( IO_PINSEL1 ),\
 _C( IO_PINSEL2 ),\
 _C( IO_PINSEL3 ),\
 _C( IO_PINSEL4 ),\
 _C( IO_PINSEL5 ),\
 _C( IO_PINSEL6 ),\
 _C( IO_PINSEL7 ),\
 _C( IO_PINSEL8 ),\
 _C( IO_PINSEL9 ),\
 _C( IO_PINSEL10 ),

#endif // #ifndef __PLATFORM_GENERIC_H__


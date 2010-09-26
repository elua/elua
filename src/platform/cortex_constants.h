// ARM/Thumb constants

#ifndef __CORTEX_CONSTANTS_H__
#define __CORTEX_CONSTANTS_H__

#define PM_BIT                  0x01

#ifdef CORTEX_M3
#define INTERRUPT_MASK_BIT      ( PM_BIT )
// Define next constant as 0 if the interrupt flag is active on logic 0, or as INTERRUPT_MASK_BIT otherwise
#define INTERRUPT_ACTIVE        ( 0 )
#endif // #ifdef CORTEX_M3

#endif


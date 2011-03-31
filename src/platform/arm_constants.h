// ARM/Thumb constants

#ifndef __ARM_CONSTANTS_H__
#define __ARM_CONSTANTS_H__

#define I_BIT                   0x80
#define F_BIT                   0x40

#ifndef CORTEX_M3
#define INTERRUPT_MASK_BIT      ( I_BIT )
// Define next constant as 0 if the interrupt flag is active on logic 0, or as INTERRUPT_MASK_BIT otherwise
#define INTERRUPT_ACTIVE        ( 0 )
#endif // #ifndef CORTEX_M3

#endif


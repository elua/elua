#include "stacks.h"

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

#define ARM_MODE_ABT     0x17
#define ARM_MODE_FIQ     0x11
#define ARM_MODE_IRQ     0x12
#define ARM_MODE_SVC     0x13

#define I_BIT            0x80
#define F_BIT            0x40

//------------------------------------------------------------------------------
//         Startup routine
//------------------------------------------------------------------------------

            .align      4
            .arm
        
/* Exception vectors
 *******************/
            .section    .vectors, "a"

resetVector:
        ldr     pc, =resetHandler       /* Reset */
undefVector:
        b       undefVector             /* Undefined instruction */
swiVector:
        b       swiVector               /* Software interrupt */
prefetchAbortVector:
        b       prefetchAbortVector     /* Prefetch abort */
dataAbortVector:
        b       dataAbortVector         /* Data abort */
reservedVector:
        b       reservedVector          /* Reserved for future use */
irqVector:
        b       irqVector               /* Interrupt */
fiqVector:
        b       fiqVector               /* Fast interrupt */

//------------------------------------------------------------------------------
/// Initializes the chip and branches to the main() function.
//------------------------------------------------------------------------------
            .section    .text
            .global     entry
            .extern     main

entry:
resetHandler:

// Wait for OSC stabilization

        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop

/* Perform low-level initialization of the chip using LowLevelInit() */
/* Initialize the relocate segment */

        ldr     r0, =_efixed
        ldr     r1, =_srelocate
        ldr     r2, =_erelocate
CopyROMtoRAM:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     CopyROMtoRAM

/* Clear the zero segment */
   	    ldr     r0, =_szero
        ldr     r1, =_ezero
        mov     r2, #0
ZeroBSS:
        cmp     r0, r1
        strcc   r2, [r0], #4
        bcc     ZeroBSS

/* Setup stacks
 **************/
 
/* Supervisor mode */
        msr     CPSR_c, #ARM_MODE_SVC
        ldr     r4, =_sstack        
        mov     sp, r4

/* Branch to main()
 ******************/
        ldr     r0, =main
        mov     lr, pc
        bx      r0

/* Loop indefinitely when program is finished */
forever:
        b       forever

#include "arm_constants.h"

        .section .text
        .align 4

        .macro prologue
#ifdef CPUMODE_THUMB
        mov   r1, pc
        bx    r1
        .arm
#endif
        .endm

        .macro functype
#ifdef CPUMODE_THUMB
        .thumb
        .thumb_func
#else
        .arm
#endif
        .endm

# enable interrupts
        .global    arm_enable_ints
        functype
arm_enable_ints:
        prologue
        mrs     r0, CPSR
        bic     r0, r0, #I_BIT
        msr     CPSR_c, r0
        bx      lr

# disable interrupts
       .global arm_disable_ints
       functype
arm_disable_ints:
       prologue
       mrs      r0, CPSR
       orr      r0, r0, #I_BIT
       msr      CPSR_c, r0
       bx       lr

# Get interrupt status
      .global arm_get_int_status
      functype
arm_get_int_status:
      prologue
      mrs     r0, CPSR
      bx      lr

      .end


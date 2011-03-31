#include "cortex_constants.h"

        .section .text
        .align 2

# enable interrupts
        .global    arm_enable_ints
        .thumb
        .thumb_func
arm_enable_ints:
        cpsie   i   
        bx      lr

# disable interrupts
       .global arm_disable_ints
       .thumb
       .thumb_func
arm_disable_ints:
       cpsid    i
       bx       lr

# Get interrupt status
      .global arm_get_int_status
      .thumb
      .thumb_func
arm_get_int_status:
      mrs       r0, PRIMASK
      bx        lr

      .end


#ifdef BUILD_UMON

        .section .text
        .align 2
        .syntax unified

# enable interrupts
        .global    umon_enable_ints
        .thumb
        .thumb_func
umon_enable_ints:
        cpsie   i   
        bx      lr

# disable interrupts
       .global umon_disable_ints
       .thumb
       .thumb_func
umon_disable_ints:
       cpsid    i
       bx       lr

# Get interrupt status
      .global   umon_get_int_stat_and_disable
      .thumb
      .thumb_func
umon_get_int_stat_and_disable:
      mrs       r0, PRIMASK
      and       r0, r0, #1
      eor       r0, r0, #1
      cpsid     i
      bx        lr

# Intercept setjmp calls
      .global   __wrap_setjmp
      .thumb
      .thumb_func
      .extern   __real_setjmp
      .extern   umon_handle_setjmp
__wrap_setjmp:
      push      { r0, lr }
      bl        umon_handle_setjmp
      pop       { r0, lr }
      b         __real_setjmp

# Intercept longjmp calls
      .global   __wrap_longjmp
      .thumb
      .thumb_func
      .extern   __real_longjmp
      .extern   umon_handle_longjmp
__wrap_longjmp:
      push      { r0, r1, lr }
      bl        umon_handle_longjmp
      pop       { r0, r1, lr }
      b         __real_longjmp

      .end

#endif // #ifdef BUILD_UMON


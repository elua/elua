[BITS 32]
[GLOBAL umon_enable_ints]
[GLOBAL umon_disable_ints]
[GLOBAL umon_get_int_stat_and_disable]
[GLOBAL umon_get_current_int]
[EXTERN __real_setjmp]
[EXTERN __real_longjmp]
[EXTERN umon_handle_setjmp]
[EXTERN umon_handle_longjmp]
[GLOBAL __wrap_setjmp]
[GLOBAL __wrap_longjmp]

[SECTION .text]
           
umon_enable_ints:
umon_disable_ints:
      ret

umon_get_int_stat_and_disable:
umon_get_current_int:
      mov       eax, 0
      ret

__wrap_setjmp:
      push      eax
      call      umon_handle_setjmp
      pop       eax
      jmp       __real_setjmp

__wrap_longjmp:
      push      ebx
      push      eax
      call      umon_handle_longjmp
      pop       eax
      pop       ebx
      jmp       __real_longjmp


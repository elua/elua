// Cortex M3 'get id' function

#define UDL_GETID_ADDRESS     0x20000004

  .section    .text
  .align      2
  .thumb
  .syntax     unified
  .global     udl_get_id
  .thumb_func

udl_get_id:
  push        {lr}
  ldr         r1, =UDL_GETID_ADDRESS
  ldr         r1, [r1]
  mov         r0, pc
  blx         r1
  pop         {pc}

  .end
  

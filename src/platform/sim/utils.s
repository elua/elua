;
; boot.s -- Kernel start location. Also defines multiboot header.
;           Based on Bran's kernel development tutorial file start.asm
;

[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL longjmp]                 
[SECTION .text]

longjmp:
  push  ebp
  mov   ebp, esp

  mov   edi, [ebp+8]            ; get jump buffer
  mov   eax, [ebp+12]           ; store retval in j->eax
  mov   [edi], eax

  mov   ebp, [edi+24]

  mov   esp, [edi+28]
  
  push dword  [edi+32]

  mov   eax, [edi]
  mov   ebx, [edi+4]
  mov   ecx, [edi+8]
  mov   edx, [edi+12]
  mov   esi, [edi+16]
  mov   edi, [edi+20]

  ret


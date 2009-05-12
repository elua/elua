;
; boot.s -- Kernel start location. Also defines multiboot header.
;           Based on Bran's kernel development tutorial file start.asm
;

[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL start]                  ; Kernel entry point.
[EXTERN main]                   ; This is the entry point of our C code
[EXTERN platform_ll_init]       ; Low level initializatin function
[SECTION .text]
    
start:

    push ebx
    call platform_ll_init

    ; Execute the kernel:
    call main                   ; call our main() function.
    jmp $                       ; Enter an infinite loop, to stop the processor
                                ; executing whatever rubbish is in the memory
                                ; after our kernel!

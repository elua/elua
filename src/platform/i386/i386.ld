/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

ENTRY(start)
SECTIONS
{

    .text 0x100000 :
    {
        code = .; _code = .; __code = .;
        PROVIDE(stext = .);
        *(.text .text.*)
        *(.rodata .rodata.*)        
        *(.gnu.linkonce.r.*)    
        *(.gnu.linkonce.t.*)                    
        . = ALIGN(4096);
        PROVIDE(etext = .);
    }

    .data :
    {
        data = .; _data = .; __data = .;
        *(.data .data.*)
        *(.gnu.linkonce.d.*)        
        . = ALIGN(4096);
    }

    .bss :
    {
        bss = .; _bss = .; __bss = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)        
        *(COMMON)        
        . = ALIGN(4096);
    }

    end = .; _end = .; __end = .;
}

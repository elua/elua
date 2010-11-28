MEMORY
{
    flash (RX) : ORIGIN = 0x0, LENGTH = 1M
}

SECTIONS
{  
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.modtext .modtext.*))
        *(.text .text.*)
        KEEP(*(.modltrmap .modltrmap.*))
        *(.rodata .rodata.*)    
        . = ALIGN(4);
    } >flash
}


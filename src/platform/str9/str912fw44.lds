OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
    sram (W!RX) : ORIGIN = 0x40000000, LENGTH = 96k
    flash (RX) : ORIGIN = 0x0, LENGTH = 512k
}

SECTIONS
{  
    .fixed :
    {
        . = ALIGN(4);
        _sfixed = .;
        PROVIDE(stext = .);
        KEEP(*(.vectors))
        KEEP(*(.dlsym))
        *(.text .text.*)        
        . = ALIGN(4);
        *(.rodata.romfiles_fs)
        *(.rodata .rodata.*)        
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        PROVIDE(etext = .);        
        _fini = .;
        *(.fini)
        . = ALIGN(4);
        _efixed = .;
    } >flash
    
    firstsram = .;
    .relocate : AT (_efixed)
    {
        . = ALIGN(4);
        _srelocate = .;
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _erelocate = .;
    } >sram
    
    .ARM.extab :
    {
        *(.ARM.extab*)
    } >sram

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } >sram
     __exidx_end = .;

    PROVIDE(symaddr = _efixed + . - _srelocate );

    .bss (NOLOAD) : {
        _szero = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        _ezero = .;
    } >sram
    
    end = .;
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(entry)

MEMORY
{
    sram (W!RX) : ORIGIN = 0x20000000, LENGTH = 0x10000
    flash (RX) : ORIGIN = 0x0, LENGTH = 0x40000
}

SECTIONS
{  
    .fixed :
    {
        . = ALIGN(4);
        _sfixed = .;
        PROVIDE(stext = .);
        KEEP(*(.vectors))
        KEEP(*(.init))
        *(.text .text.*)        
        *(.rodata .rodata.*)        
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        _efixed = .;
        PROVIDE(etext = .);        
         _fini = .;
        *(.fini)
    } >flash

    .relocate : AT (_efixed)
    {
        . = ALIGN(4);
        _srelocate = .;
        *(.ramfunc .ramfunc.* .fastrun .fastrun.*)
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

    .bss (NOLOAD) : {
        _szero = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        _ezero = .;
    } >sram
    
    end = .;
    _sstack = 0x20010000;
}

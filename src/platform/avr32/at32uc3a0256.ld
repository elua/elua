MEMORY
{
    sram (W!RX) : ORIGIN = 0x00000004, LENGTH = 0x0000FFFC
    flash (RX) : ORIGIN = 0x80002000, LENGTH = 0x00040000 - 0x2000
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _stext = .;
        PROVIDE(stext = .);
        KEEP(*(.start))
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        *(.exception .exception.*)
        . = ALIGN(4);
        _etext = .;
        PROVIDE(etext = .);
    } >flash

    .data : AT (_etext)
    {
        . = ALIGN(4);
        _data = .;
        *(.ramfunc .ramfunc.* .fastrun .fastrun.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
    } >sram

    .bss (NOLOAD) : {
        _bss = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >sram

    end = .;
    _sstack = 0x00010000;
}

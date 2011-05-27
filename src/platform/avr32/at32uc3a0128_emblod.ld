MEMORY
{
    sram (W!RX) : ORIGIN = 0x00000004, LENGTH = 0x00007FFC
    sdram (WRX) : ORIGIN = 0xd0000000, LENGTH = 0x02000000
    flash (RX) : ORIGIN = 0x80002000, LENGTH = 0x00020000 - 0x2000
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
    } >sdram

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
    _sstack = 0x00008000;
}

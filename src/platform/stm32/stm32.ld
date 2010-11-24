MEMORY
{
    sram (W!RX) : ORIGIN = 0x20000000, LENGTH = 64k
    flash (RX) : ORIGIN = 0x08000000, LENGTH = 512k
}

SECTIONS
{  
    .text :
    {
        . = ALIGN(4);
        _text = .;
        PROVIDE(stext = .);
        KEEP(*(.isr_vector))
        KEEP(*(.init))
        *(.text .text.*)        
        *(.rodata .rodata.*)        
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        _etext = .;
		_sidata = _etext;
        PROVIDE(etext = .);   
     		_fini = . ;
				*(.fini)

    } >flash
    PROVIDE( uld_endflash = . );

    .data : AT (_etext)
    {
        . = ALIGN(4);
        /* Dynamic loader data is always at the beginning of RAM */
        *(.uld .uld.*)
        _sdata = .;
        *(.ramfunc .ramfunc.* .fastrun .fastrun.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
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
		. = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);        
        _ebss = .;
    } >sram
    
    end = .;
    PROVIDE( _estack = 0x20010000 );
}


/* Common linker file used by GNU for ARM CORTEX-M3 - LPC1700 platform
 * Used in ROM mode
 * HieuNguyen - NXP Vietnam lab
 * 2009 Feb 11: Preliminary */

/* MEMORY command : Define allowed memory regions */
/* This part define various memory regions that the */
/* linker is allowed to put data into. This is an */
/* optional feature, but useful because the linker can */
/* warn you when your program is too big to fit. */
MEMORY
{
	/* On-chip ROM is a readable (r), executable region (x) */
	IROM (rx) : ORIGIN = 0x00000000, LENGTH = 512k
	/* On-chip SRAM is a readable (r), writable (w) and */
	/* executable region (x) */
	IRAM0 (rwx) : ORIGIN = 0x10000000, LENGTH = 32k
	/*
	 * Used for Ethernet RAM section
	 * IRAM1 (rwx) : ORIGIN = 0x2007C000, LENGTH = 16k
	 */
	/*
	 * Used for USB RAM section
	 *
	 */
	 IRAM2 (rwx) : ORIGIN = 0x20080000, LENGTH = 16k
}

/* SECTION command : Define mapping of input sections */
/* into output sections. */

SECTIONS
{
	/******************************************/
	/* code section */
	.text :
	{
	  PROVIDE(stext = .);
		KEEP(*(.isr_vector .isr_vector.*))
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
		*(.glue_7)
		*(.glue_7t)
		*(.gcc_except_table)
		*(.rodata .rodata*)
		*(.gnu.linkonce.r.*)
		. = ALIGN(4);
	} >IROM

	/******************************************/
	/* .ctors .dtors are used for c++ constructors/destructors */
	.ctors :
	{
		. = ALIGN(4);
		PROVIDE(__ctors_start = .);
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		PROVIDE(__ctors_end = .);
	} >IROM

	.dtors :
	{
		. = ALIGN(4);
		PROVIDE(__dtors_start = .);
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		PROVIDE(__dtors_end = .);

		. = ALIGN(4);
		/* End Of .text section */
		etext = .;
                _fini = .;
		_sidata = .;
	} >IROM

	/******************************************/
	/* This used for USB RAM section */
	.usb_ram (NOLOAD):
	{
		*.o (USB_RAM)
	} > IRAM2

	/* data section */
	.data : AT (_sidata)
  	{
  		. = ALIGN(4);
		_sdata = .;

		*(vtable vtable.*)
		*(.data .data.*)
		*(.gnu.linkonce.d*)

		. = ALIGN(4);
		_edata = . ;
	} >IRAM0

	/******************************************/
	/* For no-init variables section */
	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		_sbss = . ;

		*(.bss .bss.*)
		*(.gnu.linkonce.b*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = . ;
	} >IRAM0

	/******************************************/
	/* For stack section */
  	.stackarea (NOLOAD) :
  	{
	    . = ALIGN(8);
	    _sstack = .;

		*(.stackarea .stackarea.*)

		. = ALIGN(8);
		_estack = .;

		. = ALIGN(4);
		_end = . ;
		PROVIDE (end = .);

  	} > IRAM0

	/******************************************/
	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	/* .comment       0 : { *(.comment) } */
	/* DWARF debug sections.
	 Symbols in the DWARF debugging sections are relative to the beginning
	 of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}

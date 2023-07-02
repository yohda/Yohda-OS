SECTIONS
{
	. = 0x100000;
	.multiboot_header : 
	{
		KEEP(*(.multiboot_header))	
	}

	. = ALIGN(0x8000);
	.text :
	{
		pbl.o (.text)	
		main.o (.text)
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
    {
		pbl.o (.data)
		paging.o (.data)
        *(.data)
    }

	.bss :
    {
        *(.bss)
		*(COMMON)
    }
}


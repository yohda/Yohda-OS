SECTIONS
{
	. = 0x10000;
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
        *(.data)
    }

	.bss :
    {
        *(.bss)
		*(COMMON)
    }
}

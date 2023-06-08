SECTIONS
{
	.text 0x7C00 :
	{
		fbl.o (.text) 
		sbl.o (.text)
		. = 0x100000;
		pbl.o (.text)
		/**(.text)*/
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
	
/*
	.text 0x7E00:
	{
		sbl.o (.sbl.text)
	}
	ebl_text = .;
	. = 0xC0100000;
	.text ALIGN(0x1000): AT(ADDR(.text) - 0xC0000000)
	{
		main.o(.text)
		*(.text)
	}

	.rodata ALIGN(0x1000) : AT(ADDR(.rodata) - 0xC0000000)
   	{
		*(.rodata)
	}	

	.data ALIGN(0x1000): AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

	.bss ALIGN(0x1000): AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
		*(COMMON)
    }
			*/
}

SECTIONS
{
	.text 0x7C00 :
	{
		fbl.o (.text)
		sbl.o (.text)
		*(.text)
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

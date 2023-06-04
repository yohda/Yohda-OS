SECTIONS
{
	.text 0x7C00 :
	{
		pbl.o (.text)
		sbl.o (.text)
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

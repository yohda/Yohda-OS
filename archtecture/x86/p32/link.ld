VIRT_BASE = 0xC0000000;
SECTIONS
{
	. = 0x100000; 
	sboot_size = .;
	.text.boot :
	{
		pbl.o (.text.boot)
	}

	.data.boot :
    {
		pbl.o (.data.boot)
    }

	eboot_size = .;
	. = eboot_size + 0xC0000000;

	skernel_size = .;	
	.text : AT(ADDR(.text) - VIRT_BASE)
	{
		paging.o (.text)
		main.o (.text)
		*(.text)
	}

	.rodata : AT(ADDR(.rodata) - VIRT_BASE)
	{
		*(.rodata)
	}

	.data : AT(ADDR(.data) - VIRT_BASE)
    {
        *(.data)
    }

	.bss : AT(ADDR(.bss) - VIRT_BASE)
    {
        *(.bss)
		*(COMMON)
    }
	ekernel_size = .;
}

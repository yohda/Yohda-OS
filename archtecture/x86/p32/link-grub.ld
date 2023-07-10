SECTIONS
{
	. = 0x100000;
	sboot_size = .;
	.multiboot_header : 
	{
		KEEP(*(.multiboot_header))	
	}

	. = ALIGN(0x8000);
	.text.boot :
	{
		paging.o (.text.boot)	
	}

	.data.boot :
    {
		paging.o (.data.boot)
    }
	eboot_size = .;
	
	_virt_base = 0xC0000000;
	. = _virt_base;
	. = eboot_size + 0xC0000000;

	_kernel32_start = .;	
	.text : AT(ADDR(.text) - _virt_base)
	{
		pbl.o (.text)
		main.o (.text)
		*(.text)
	}

	.rodata : AT(ADDR(.rodata) - _virt_base)
	{
		*(.rodata)
	}

	.data : AT(ADDR(.data) - _virt_base)
    {
        *(.data)
    }

	.bss : AT(ADDR(.bss) - _virt_base)
    {
        *(.bss)
		*(COMMON)
    }
	_kernel32_end = .;
	_kernel32_size = _kernel32_end - _kernel32_start;}


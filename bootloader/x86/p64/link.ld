HH_BASE = 0xFFFF800000000000;

ENTRY(_start_64)
SECTIONS
{
	. = 0x100000;
	smode64 = .;
	.text.mode64 :
	{
		*(.text.mode64)
	}

	.data.page64 :
	{
		*(.data.page64)		
	}

	/* it`s needed to plus end address of `.text.mode64` to 0xFFFF_8000_0000_0000. 
	 * Becuase, if don`t plus, it`s happended that there is duplicating he first 0x1XXXXX regions.
	 * 
	 * */
	emode64 = .;
	. = 0xFFFF800000000000 + emode64; 
	
	kern_virt_start_addr = .;
	kern_phy_start_addr = kern_virt_start_addr - HH_BASE;	
	
	.text ALIGN(0x1000): AT(ADDR(.text) - HH_BASE)
	{
		*(.text)
	}

	.rodata ALIGN(0x1000) : AT(ADDR(.rodata) - HH_BASE)
   	{
		*(.rodata)
	}	

	.data ALIGN(0x1000): AT(ADDR(.data) - HH_BASE)
    {
        *(.data)
    }

	.bss ALIGN(0x1000): AT(ADDR(.bss) - HH_BASE)
    {
        *(.bss)
		*(COMMON)
    }

	kern_virt_end_addr = .;
	kern_phy_end_addr = kern_virt_end_addr - HH_BASE;
}

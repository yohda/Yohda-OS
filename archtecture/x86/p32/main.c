#include "vga.h"
#include "interrupt.h"
#include "cpu.h"
#include "ata.h"
#include "pic.h"
#include "io.h"
#include "mm/mm.h"
#include "block/ahci.h"
#include "debug.h"

extern void load_higher_half(void);
u8 *start64_addr = 0x180000;

int main();

int main(unsigned int start_sector)
{
	int i;
	u16 sector = start_sector;
	// you must first initialize VGA other than.
	// Without that, there is nothing to see the debug log.
	vga_text_init();
	//vga_text_write("Enter Protected mode\n");
	debug("Enter Protected mode\n");
	
	cpu_init();
	if(CPUID_64) {
		debug("CPUID#0x%x, X64#0x%x, 1GB#0x%x STD#0x%x EXT#0x%x\n", CPUID, CPUID_64, CPUID_PAGE_1GB, CPUID_STD_FUNCS, CPUID_EXT_FUNCS);
		debug("x86-64 supported\n");	
		mode64();
	} else {
		debug("x86-64 not supported\n");
	}
	/*
	 * PIC remap some interrupts to prevent from hanppening the double fault.
	 * So, PIC was initialized first than interrupt enable.
	 * */
	pic_init();
	interrupt_init();	

	mm_init(0x100000 * 512); // 512MB
	
	pci_init();
	ahci_init();
	//ata_init();

	//ahci_read(sector, 4, start64_addr);
	//sector += 48;
	//start64_addr += 512*48;

	/*
	 * AHCI Read 16 sectors at once. i`d like to read more counts, but, it`s impossible to read more. So, shiled codes. Read 4MB. 
	 * */

	// Read 4MB
	/*
	for(i=0; i<512; i++) {
		ahci_read(sector, 16, start64_addr);
		io_wait();
		sector += 16;
		start64_addr += 512*16;
	}	
	*/
	while(1);
}

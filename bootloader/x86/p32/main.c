#include "vga.h"
#include "interrupt.h"
#include "ata.h"
#include "pic.h"
//#include "block/ahci.h"

extern void load_higher_half(void);
u8 *buf = 0x100000;

int main();

int main()
{
	int a, b;
	int c[3];
	vga_text_init();
	vga_text_write("Protected mode\n");
	vga_text_write("Protected mode2");
	vga_text_write("");
	vga_text_write("Pr\node2");

	interrupt_init();	
	pic_init();
	//load_higher_half();
	//b = a/0;
	pci_init();
	ata_init();

	//ahci_read(0, 1, buf);

	while(1);
}

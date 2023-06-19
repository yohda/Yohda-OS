#include "vga.h"
#include "interrupt.h"
#include "ata.h"
#include "pic.h"
#include "io.h"

extern void load_higher_half(void);
u8 *buf = 0x100000;

int main();

int main()
{
	int a, b;
	int c[3];
	vga_text_init();
	vga_text_write("Protected mode\n");

	kprintf("0x%x\n0x%x\n0x%x\n0x%x\n0x%x\n", -1, 2254, 0, 0xFF, 0x22e213fd);	
	kprintf("%d\n%d\nw\n%d\n%s\n", 1238728, 0, -232, "123124");	
	kprintf("YohdaOS\n");	
	kprintf("");	
	
	interrupt_init();	
	pic_init();
	//load_higher_half();
	//b = a/0;
	pci_init();
	ata_init();

	//ahci_read(0, 1, buf);

	while(1);
}

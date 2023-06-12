#include "vga.h"
#include "interrupt.h"

extern void load_higher_half(void);

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
	load_higher_half();
	//b = a/0;

	while(1);
}

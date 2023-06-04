#include "vga.h"

int main()
{
	vga_text_init();
	vga_text_write('P');
	vga_text_write('r');
	vga_text_write('o');
	vga_text_write('t');
	vga_text_write('e');
	vga_text_write('d');

	while(1);
}

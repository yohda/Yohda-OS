#include "vga.h"

int main()
{
	vga_text_init();
	vga_text_write("Protected mode\n");
	vga_text_write("Protected mode2");
	vga_text_write("");
	vga_text_write("Pr\node2");

	while(1);
}

#include "debug.h"
//#include"io.h"

int main();
int main()
{
	vga_text_init();

	debug("64bit-kernel start\n");	
	//kprintf("%s", "efef");	

	while(1);	
}

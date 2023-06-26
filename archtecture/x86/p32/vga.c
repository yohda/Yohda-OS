#include "vga.h"
#include "string.h"

#define VGA_TEXT_BASE 0xB8000;

struct vga_text *vga_text_bp;
struct vga_text *vga_text_cp;
int cl;

static int _vga_text_write(const char *c, const u8 fg, const u8 bg)
{
	struct vga_text vt;
	
	vt.attr = (bg<<4)|fg;
	while(*c) {
		if(*c == '\n') {
			cl++;
			vga_text_cp = vga_text_bp + (VT_COLUMNS*cl);	
			c++;
			continue; 
		}

		vt.text = *c;
		
		*vga_text_cp = vt;
	
		vga_text_cp++;
		c++;
	}	

	return 0;
}

int vga_next_line()
{

}

int vga_text_write(const char *c)
{
	_vga_text_write(c, VT_WHITE, VT_BLACK);
	
	return 0;
}

int vga_text_init()
{
	vga_text_bp = (struct vga_text *)VGA_TEXT_BASE;
	vga_text_cp = vga_text_bp;
	
	/**
	 * For represent a character, it needs two bytes. first is color attribute, second is a character.  
	 */
	memset(vga_text_bp, 0x00, VT_ROWS*VT_COLUMNS*2);
	
	return 0;
}


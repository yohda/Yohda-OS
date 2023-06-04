#include "vga.h"

#define VGA_TEXT_BASE 0xB8000;

struct vga_text *vga_text_bp;
struct vga_text *vga_text_cp;

static int _vga_text_write(const char c, const u8 fg, const u8 bg)
{
	struct vga_text vt;

	vt.text = c;
	vt.attr = (bg<<4)|fg;
		
	*vga_text_cp = vt;

	vga_text_cp++;
}

int vga_text_write(const char c)
{
	_vga_text_write(c, VT_WHITE, VT_BLACK);
}

int vga_text_init()
{
	vga_text_bp = VGA_TEXT_BASE;
	vga_text_cp = vga_text_bp;
}


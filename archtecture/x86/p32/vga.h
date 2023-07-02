#ifndef _VGA_H_
#define _VGA_H_

#include "type.h"

// Size
#define VT_ROWS			(25)
#define VT_COLUMNS		(80)
#define VT_SIZE			(VT_COLUMNS*VT_ROWS)

// Attribute
#define VT_BLACK 		(0x0)
#define VT_BLUE			(0x1)
#define VT_GREEN		(0x2)
#define VT_CYAN			(0x3)
#define VT_RED			(0x4)
#define VT_MAGENTA		(0x5)
#define VT_BROWN		(0x6)
#define VT_LGRAY		(0x7)
#define VT_DARK_GRAY	(0x8)
#define VT_LBLUE		(0x9)
#define VT_LGREEN		(0xA)
#define VT_LCYAN		(0xB)
#define VT_LRED			(0xC)
#define VT_LMAGENTA		(0xD)
#define VT_YELLOW		(0xE)
#define VT_WHITE		(0xF)

struct vga_text {
	char text;
	u8 attr;
} __attribute__((packed));

int vga_text_init();
int vga_text_write(const char *c);
int vt_new_line(const char* buf);
int vt_flush(void);

#endif

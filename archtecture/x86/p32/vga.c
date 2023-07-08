#include "vga.h"
#include "string.h"

#define VT_BASE (0xB8000) 
#define VT_MONS (0x10)		// Monitor count
#define VT_BUF_SIZE (VT_SIZE*VT_MONS)

#define VT_EXCEED	((vga.curr_row)>=(VT_ROWS))

struct dev_vga {
	struct vga_text *bp;
	struct vga_text *cp;
	struct vga_text buffer[VT_MONS*VT_ROWS][VT_COLUMNS] __attribute__((packed));
	uint32_t curr_row;
	uint32_t curr_col;
	uint32_t pre_row;
	uint32_t pre_col;
	uint8_t inited;
};

struct dev_vga vga;

static int _vt_flush(void)
{
	int32_t i = 0, j = 0, k = 0, rows = 0;

	// Scroll	
	if(VT_EXCEED) {
		vga.pre_row = vga.curr_row - (VT_ROWS-1);
		memcpy(vga.bp, vga.buffer[vga.pre_row], VT_SIZE*2);	
	} else {
		rows = vga.curr_row+1;
		for(i=vga.pre_row; i<rows; i++) {
			for(j=vga.pre_col; vga.buffer[i][j].text && j<VT_COLUMNS; j++) {
				*(vga.bp+((i*80)+j)) = vga.buffer[i][j];
			}
		}
	}	
		
	vga.pre_row = vga.curr_row;
	vga.pre_col = vga.curr_col;
}

int vt_flush(void)
{
	if(!vga.inited)
		return -1;
	
	_vt_flush();
}

static int _vt_new_line(void)
{
	vga.curr_row++;
	vga.curr_col = 0;
	vga.pre_row = 0;
	vga.pre_col = 0;
}

static int _vga_text_write(const char *c, const u8 fg, const u8 bg)
{
	while(*c) {
		if(*c == '\n' || vga.curr_col >= VT_COLUMNS) {
			if(vga.curr_row >= VT_MONS*VT_ROWS) {
				// Screen Buffer Overflow
				(*(vga.bp)).text = 'F';
				(*(vga.bp+1)).text = 'u';
				(*(vga.bp+1)).text = 'c';
				(*(vga.bp+1)).text = 'k';
			}

			_vt_new_line();

			c++;
			continue;
		}
	
		vga.buffer[vga.curr_row][(vga.curr_col)++].text = *c;	

		c++;
	}	
	
	return 0;
}

int vt_flush_with_buf(const char *buf)
{
	if(!buf || !vga.inited)
		return -1;

	_vga_text_write(buf, VT_WHITE, VT_BLACK);
	_vt_flush();
}

int vt_new_line(const char *buf)
{
	if(!buf || !vga.inited)
		return -1;
	
	_vga_text_write(buf, VT_WHITE, VT_BLACK);
	_vt_new_line();
}

int vga_text_write(const char *c)
{
	if(!c || !vga.inited)
		return -1;

	_vga_text_write(c, VT_WHITE, VT_BLACK);

	return 0;
}

int vt_cls(void)
{
	int i, j;

	if(!vga.inited)
		return -1;
	/**
	 * In VGA, to represent a character it needs two bytes. first is color attribute, second is a character.  
	 */
	memset(vga.bp, 0x00, VT_SIZE*2);
	for(i=0; i<(VT_ROWS*VT_MONS); i++) {
		for(j=0; j<VT_COLUMNS; j++) {
			vga.buffer[i][j].text = 0;	
			vga.buffer[i][j].attr = VT_WHITE;
		}
	}

	vga.curr_row = 0;
	vga.curr_col = 0;
	vga.pre_row = 0;
	vga.pre_col = 0;
}

int vga_text_init()
{
	int i, j;

	vga.bp = (struct vga_text *)vmm_phy_to_virt(VT_BASE);
	vga.cp = vga.bp;
	
	vga.inited = true;

	vt_cls();

	return 0;
}

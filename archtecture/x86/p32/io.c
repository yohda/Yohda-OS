#include "io.h"
#include "vga.h"
#include "stdlib.h"
#include "limit.h"
#include "string.h"

#define ASCII_MAX	128

static u8 printable[ASCII_MAX] = 
{
	0,0,0,0,0,0,0,0, // 0 ~ 7
	0,0,1,0,0,0,0,0, // 8 ~ 15
	0,0,0,0,0,0,0,0, // 16 ~ 23
	0,0,0,0,0,0,0,0, // 24 ~ 31
	1,1,1,1,1,4,1,1, 
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0
}; 

struct pr_info {
	va_list *args;
	int curr_pos;
	char buf[4096];
};

void (*pre)(struct pr_info *pi);

static void pre_int(struct pr_info *pi)
{
	int i = va_arg(*(pi->args), int);
	
	itoa(i, pi->buf);
}

static void pre_char(struct pr_info *pi)
{
	char ch = va_arg(*(pi->args), char);
	pi->buf[0] = ch;
}

static void pre_str(struct pr_info *pi)
{
	char *str = va_arg(*(pi->args), char*);
	int len = strlen(str);

	memcpy(pi->buf, str, len);
}

static void pre_hex(struct pr_info *pi)
{
	u32 h = va_arg(*(pi->args), u32);
	
	itoh(h, pi->buf);
}

__attribute__ ((format (printf, 1, 2))) void kprintf(const char *fmt, ...)		
{
	va_list args;
	va_start(args, fmt);
   	vkprintf(fmt, args);
	va_end(args);	
}

void vkprintf(const char *fmt, va_list args)
{
	u8 state = PRINT_NORMAL, pre_state = PRINT_NORMAL;
	struct pr_info pi;
	char c;
	int i = 0;

	memset(pi.buf, 0, sizeof(pi.buf));
	for(pi.args=&args; c=fmt[i]; i++) {
		if(!printable[c])
			continue;

		if(state == PRINT_NORMAL) {
			state = printable[c];
		} else if(state == PRINT_FORMAT) {
			switch(c) {
				case 'd':
					pre = pre_int;	
				break;
				case 'c':
					pre = pre_char;
				break;
				case 's':
					pre = pre_str;
				break;
				case 'x':
				case 'X':
					pre = pre_hex;
				break;
				default:
					// Error
				break;
			}

			pre(&pi);
		} 
		
		if (state == PRINT_CHAR) {
			pi.buf[i] = c;
			state = PRINT_NORMAL;
		} 
	}

	vt_flush_with_buf(pi.buf);
}

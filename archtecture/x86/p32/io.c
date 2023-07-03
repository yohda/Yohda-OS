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
	1,1,1,1,1,2,1,1, 
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
	int len;
	char buf[4096];
};

static void (*parse)(struct pr_info *pi, const int idx);

static void parse_int(struct pr_info *pi, const int idx)
{
	int i = va_arg(*(pi->args), int);	

	itoa(i, (pi->buf)+idx);
	pi->len = strlen(pi->buf);
}

static void parse_char(struct pr_info *pi, const int idx)
{
	char ch = va_arg(*(pi->args), char);
	pi->buf[idx] = ch;

	pi->len = strlen(pi->buf);
}

static void parse_str(struct pr_info *pi, const int idx)
{
	char *str = va_arg(*(pi->args), char*);
	int len = strlen(str);

	memcpy((pi->buf)+idx, str, len);
	pi->len = strlen(pi->buf);
}

static void parse_hex(struct pr_info *pi, const int idx)
{
	u32 h = va_arg(*(pi->args), u32);

	itoh(h, (pi->buf)+idx);
	pi->len = strlen(pi->buf);
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
	u8 state = PRINT_NORMAL;
	struct pr_info pi;
	char c;
	int i = 0;

	memset(&pi, 0, sizeof(pi));
	for(pi.args=&args; c=fmt[i]; i++) {
		if(!printable[c])
			continue;

		if(state == PRINT_NORMAL) {
			state = printable[c];
		} else if(state == PRINT_FORMAT) {
			switch(c) {
				case 'd':
					parse = parse_int;	
				break;
				case 'c':
					parse = parse_char;
				break;
				case 's':
					parse = parse_str;
				break;
				case 'x':
				case 'X':
					parse = parse_hex;
				break;
				default:
					// Error
				break;
			}

			parse(&pi, pi.len);
			state = PRINT_NORMAL;
		} 
		
		if (state == PRINT_CHAR) {
			pi.buf[pi.len] = c;
			pi.len += 1;
			state = PRINT_NORMAL;
		} 
	}

	vt_flush_with_buf(pi.buf);
}

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdarg.h>
#include "type.h"

enum {
	PRINT_NORMAL = 0x00,
	PRINT_FORMAT = 0x01,
	PRINT_OUTPUT = 0x02,
	PRINT_MAX,
	PRINT_INVAL,
};

__attribute__ ((format (printf, 1, 2))) void kprintf(const char *fmt, ...);
void vkprintf(const char *fmt, va_list args);

#endif

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdarg.h>
#include "type.h"

enum {
	PRINT_NORMAL = 0x00, // `NORMAL` mode is available to select one of below two modes. this mode isn`t a special function.
	PRINT_CHAR = 0x01, // `OUTPUT` mode is related to print character.
	PRINT_FORMAT = 0x02, // `FORMAT` mode is related to `%` format.
	PRINT_MAX,
	PRINT_INVAL,
};

__attribute__ ((format (printf, 1, 2))) void kprintf(const char *fmt, ...);
void vkprintf(const char *fmt, va_list args);

#endif

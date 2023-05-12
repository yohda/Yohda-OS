#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "type.h"

#define BIT_SET 0x01
#define BIT_CLR 0x00

struct bitmap {
	void *ptr; 
	u32 bit;
	u32 size;
};

int bitmap_alloc(struct bitmap *bmp, const u32 pack, const u32 size);
int bitmap_free();
int bitmap_set();
int bitmap_get_free();
int bitmap_get_status();

#endif

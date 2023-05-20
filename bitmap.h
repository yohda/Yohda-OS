#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "type.h"

#define BITMAP_DEF_BIT 	(32)
#define BIT_SET 		(0x01)
#define BIT_CLR 		(0x00)

struct bitmap {
	void *base; 
	u32 bit;
	u32 size;
};

int bitmap_alloc(struct bitmap *bmp, const u32 bit, const u32 size);
int bitmap_free();
int bitmap_set(struct bitmap *bmp, u32 ost, bool f);
int bitmap_get_free(struct bitmap *bmp);
int bitmap_get_status();

#endif

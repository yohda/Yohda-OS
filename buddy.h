#ifndef _BUDDY_H_
#define _BUDDY_H_

#include "type.h"
#include "bitmap.h"

struct bitmaps {
	u32 ord;
	u32 num;
	u32 frees;
	struct bitmap bmp;
};

// yohdaOS assumes that depth of buddy system is less than 255.
// Actuallly, to assign size 2^255 per page is impossible.
// If it is larger than 255, it need to change the way to manage memory.   
struct bud_blk_hdr {
	u8 dep;	
};

struct bud_blk {
	struct bud_blk_hdr hdr;
	void *addr;
};

#endif

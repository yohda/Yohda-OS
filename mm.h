#ifndef _MM_H_
#define _MM_H_

#include "list.h"

// In yohdaOS, it assumes size of RAM is 4GB.
// So, if you assumes that size of order-0-block is 2KB, the number of total chunks are 0x200000.
// But, those is managed from bitmaps. So, those need to be devided from 32 becuase size of bitmap is 32.
// Then, it needs 65,536(0x10000) 32-bit bitmaps.
// Also, you sequently calculate next bitmap. it order-1-block. it`s size is 4KB.
// ((4 * 1024 * 1024 * 1024 / 4096) / 32)  
// 
struct memory_layout {
	char name[64];
	u32 base;
	u32 size;
};

struct bitmaps {
	u32 ord;
	u32 len;
	u32 *block;
	//struct list_head *list;
};

// yohdaOS assumes that depth of buddy system is less than 255.
// Actuallly, to assign size 2^255 per page is impossible.
// If it is larger than 255, it need to change the way to manage memory.   
struct mm_blk_hdr {
	u8 dep;	
};

struct mm_blk {
	struct mm_blk_hdr hdr;
	void *addr;
};

int mm_init();
void* mm_alloc(u32 size);
void mm_free(void *addr);

#endif

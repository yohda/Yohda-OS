#ifndef _MM_H_
#define _MM_H_

#include "list.h"
#include "bitmap.h"

enum {
	MM_UL 			= 0x01, // User Level
	MM_KL 			= 0x02, // Kernel Level
	MM_FIX 			= 0x04, // No Swap
};

enum {
	MM_PRI = 0,
	MM_SEC, 
};
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

// For compatibility of multiheader2
struct mmap_entry {
  uint64_t addr;
  uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  uint32_t type;
  uint32_t zero;
};

struct mmap {
	uint32_t size;
	struct mmap_entry entries[0];
};

struct mm {
	void *base;	// primary memory management base address
	int size;
	int llc;
	int ulc;
};

int mm_init();
void* mm_alloc(u32 size, u32 flag);
void mm_free(void *addr);

int mm_where_is_addr(const void *addr);
int mm_get_pri_size(void);
int mm_set_pri_size(const int size);
int mm_set_pri_base(const void *base);
int mm_get_sec_size(void);
int mm_set_sec_size(const int size);
int mm_set_sec_base(const void *base);
int mm_get_pri_ulc(void);
int mm_get_pri_llc(void);
int mm_get_sec_ulc(void);
int mm_get_sec_llc(void);

#endif

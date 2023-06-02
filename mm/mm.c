#include "type.h"
#include "Console.h"
#include "errno.h"
#include "mm.h"
#include "mmi.h"
#include "debug.h"
#include "pool.h"
#include "bitmap.h"

/* For Test */
#define MM_TEST_HEAP128K_LL2048_BITMAP32x 

/* 64-bit yohdaOS Memory Layout */
#define MM_BASE				(0x2000000) 				// 32MB
#define MM_META_BASE 		(MM_BASE)					// 32MB
#define MM_HEAP_META_BASE	((MM_META_BASE)|(0x10000))	// 32MB + 64KB

// Actually Physical RAM size is 4GB. But, it is imposible to assign all 4GB. The parts in it are used in boot and system. 
#define MM_RAM_SIZE 		(1*1023*891*913) 	
#define MM_PAGE_SIZE		(0x800)	

#define MM_HEAP_MIN_SIZE 	(0x20000)

// A ratio primary storage size against secondary is 9:1.
// For example, if you received 1GB, you have to give 900MB the primary and 100MB the secondary. 

#define MM_PRI_CHUNK_ULC (MM_PRI_CHUNK_LLC*2*2*2*2*2*2*2*2*2*2*2)
#define MM_PRI_CHUNK_LLC (2048)
#define MM_SEC_CHUNK_ULC ((MM_PRI_CHUNK_LLC)/2)
#define MM_SEC_CHUNK_LLC (16)

#define IS_MEM_ERR(x) ((!x)||((x)>=(MM_BASE+mm.heap_size))||((x)<(MM_BASE)))

// I think that it is able to change the a size of bitmaps. In your computer with operating 64-bit, it can be 64.

struct mm_mgr {
	// memory info
	s64 org_size; 	// In initialization, size given to heap.
	s64 heap_size;	// Real heap size.
	s64 used_size;	// used size in real heap size
	s64 free_size;  // free size in real heap size
	s64 rmd_size;	// original size - heap size

	struct mm pri;
	struct mm sec;

	// mem layout
	struct memory_layout *ml;
	struct list_node ml_list;
	u32 mem_num;
};

struct mm_mgr mmm;

int mm_show_info(){
	u32 i;
	for(i = 0 ; i < mmm.mem_num ; i++) {	
		mm_debug("name#%s\n",mmm.ml[i].name);
		mm_debug("base#0x%x, size#0x%x\n", mmm.ml[i].base, mmm.ml[i].size);
	}
}

void* mm_get_ml(void)
{
	if(0)
		return NULL;
	
	return (mmm.ml[mmm.mem_num-1].base + mmm.ml[mmm.mem_num-1].size);
}

int mm_set_info(char *name, u32 *base, u32 size)
{
	struct memory_layout *lout;
	u32 i, len;
	if(size < 1) {
		mm_debug("Invalid parameter#%d\n", size);
		return -EINVAL;
	}

	len = strlen(name);
	if(len < 0 || len > 64) {
		mm_debug("Invalid parameter#%s\n", name);
		return -EINVAL;
	}

	if(mmm.mem_num * sizeof(struct memory_layout) >= MM_HEAP_META_BASE - MM_META_BASE) {
		mm_debug("Out of memory#%s\n", mmm.mem_num * sizeof(struct memory_layout));
		return -ENOMEM;
	}

	lout = mmm.ml + mmm.mem_num;

	memcpy(lout->name, name, len);
	lout->base = (u32)base;
	lout->size = size;
	mmm.mem_num++;
}

void* mm_alloc(u32 size, u32 flag)
{
	
}

static struct mm_blk_hdr *mm_decode(void *addr)
{
	//u32 chunk_ost = ((u32)addr - mm.heap_base);
	//struct mm_blk_hdr *hdr = (struct mm_blk_hdr *)(mm.hdr_base + (chunk_ost)/mm.ll);
	
	//return hdr;
}

void mm_free(void *addr)
{
	
}

int mm_where_is_addr(const void *addr)
{
	if(!addr)
		return err_dbg(-1, "err\n");
	
	if((addr>=mmm.sec.base) 
	&& (addr<(mmm.sec.base+mmm.sec.size)))	
		return MM_SEC;

	return MM_PRI;
}	

s64 mm_get_pri_size(void)
{
	return mmm.pri.size;
}

int mm_set_pri_size(const s64 size)
{
	if(size < 1)
		return err_dbg(-1, "err\n");

	mmm.pri.size = size;
}

int mm_set_pri_base(const void *base)
{
	if(!base)
		return err_dbg(-1, "err\n");
	
	mmm.pri.base = base;
}

s64 mm_get_sec_size(void)
{
	return mmm.sec.size;
}

int mm_set_sec_size(const s64 size)
{
	if(size < 1)
		return err_dbg(-1, "err\n");

	mmm.sec.size = size;
}

int mm_set_sec_base(const void *base)
{
	if(!base)
		return err_dbg(-1, "err\n");
	
	mmm.sec.base = base;
}

int mm_get_pri_ulc(void)
{
	return MM_PRI_CHUNK_ULC;
}

int mm_get_pri_llc(void)
{
	return MM_PRI_CHUNK_LLC;
}

int mm_get_sec_ulc(void)
{
	return MM_SEC_CHUNK_ULC;
}

int mm_get_sec_llc(void)
{
	return MM_SEC_CHUNK_LLC;
}

// RAM 사이즈에 대한 디텍은 BIOS 나 UEFI를 통해서 하는게 가장 좋다고 한다.
// 이게 Memory Controller를 통해서 RAM 사이즈를 얻는 경우가 가장 좋다는거 같은데, BIOS 나 UEFI가 칩 벤더 레벨에서 알아서 이 정보를 가져와 주기 때문인 거 같다.
int mm_init(const s64 heap_size)
{
	s64 real_size = 0, rmd = 0;
	int err = -1;

	if(heap_size < MM_HEAP_MIN_SIZE)
		return err_dbg(-1, "Heap memory isn`t insufficient size#0x%x\n", heap_size);

	mmm.org_size = heap_size;
	rmd = mmi_init(MM_BASE, heap_size);
	if(rmd < 0)
		return err_dbg(-1, "Failed to initialize Memoery Management\n");

	mmm.rmd_size = rmd;
	mmm.heap_size = mmm.org_size - mmm.rmd_size;

	/* After initializing the memory management, start to work memory management meta data */

	return 0;
}

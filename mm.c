#include "Types.h"
#include "Console.h"
#include "errno.h"
#include "mm.h"
#include "debug.h"
#include "pool.h"
#include "bitmap.h"

/* For Test */
#define MM_TEST_HEAP128K_LL2048_BITMAP32x 

/* 64-bit yohdaOS Memory Layout */
#define MM_BASE				(0x2000000) 				// 32MB
#define MM_META_BASE 		(MM_BASE)					// 32MB
#define MM_HEAP_META_BASE	((MM_META_BASE)|(0x10000))	// 32MB + 64KB
#define MM_HEAP_DATA_BASE  	(0x2100000)

// Actually Physical RAM size is 4GB. But, it is imposible to assign all 4GB. The parts in it are used in boot and system. 
#define MM_RAM_SIZE 		(1*1023*891*913) 	

#define MM_PAGE_SIZE		(0x800)	
// Upper Limit
#define MM_ALLOC_UL			()
// Low Limit
#define MM_ALLOC_LL 		MM_PAGE_SIZE // I think it is better to select the size same as page, or more than it.

#define MM_HEAP_MIN_SIZE 	(0x20000)

// I think that it is able to change the a size of bitmaps. In your computer with operating 64-bit, it can be 64.

#define MM_BUD_FREE		0x00000000
#define MM_BUD_USED		0x00000001

u32 BITMAP_INIT32[]= { 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF0, 0xFFFFFF00, 0xFFFF0000, 0x00000000 };
u64 BITMAP_INIT64[] = { 0x0000000000000000, 0xFFFFFFFE00000000, 0xFFFFFFFC00000000, 0xFFFFFFF000000000, 0xFFFFFF0000000000, 0xFFFF000000000000 , 0xFFFFFFFF00000000 };

#define MM_BITMAP_INIT(x) BITMAP_INIT##mm.bitmap_size

#define BUD_MAX_ORD	 		(11)
#define BUD_MIN_CHUNK		(8)

struct mm_manager {
	// buddy
	u32 dep;
	u8 bitmap_size;
	u32 ll;	// low limit
	u32 ul; // upper limit
	struct mm_blk_hdr *hdr_base;
	u32 hdr_size;
	//struct bitmaps *bitmaps;
	
	// mem layout
	struct memory_layout *ml;
	struct list_node ml_list;
	u32 mem_num;

	// memory info
	u32 page_size;
	u32 org_size;
	u32 heap_ul_size;
	u32 heap_meta_base;
	u32 heap_meta_size;
	u32 heap_base;
	u32 heap_size;
	u32 rmd_size;
};

struct bitmaps *bud;
struct mm_manager mm;

int mm_show_info()
{
	u32 i;
	for(i = 0 ; i < mm.mem_num ; i++) {	
		mm_debug("name#%s\n",mm.ml[i].name);
		mm_debug("base#0x%x, size#0x%x\n", mm.ml[i].base, mm.ml[i].size);
	}

	mm_debug("YohdaOS Buddy Info\n");
	mm_debug("Original size#0x%x\n", mm.org_size);
	//mm_debug("Data size#0x%x\n", mm.heap_size);
	mm_debug("Data size#0x%x\n", mm.heap_size);
	mm_debug("Meta data size#0x%x\n", mm.heap_meta_size);
	mm_debug("Remaider size#0x%x\n", mm.rmd_size);
	mm_debug("Buddy dep#%d\n", mm.dep);
	mm_debug("Buddy Upper limit#%d\n", mm.ul);
	mm_debug("Buddy Lower limit#%d\n", mm.ll);
	for(i=0 ; i<mm.dep ; i++) {
		mm_debug("Bitmap Order#%d\n", i);
		mm_debug("Size#%d, Number#%d, Fres#%d\n", bud[i].bmp.size, bud[i].num, bud[i].frees);
	}

}

void* mm_get_ml(void)
{
	if(0)
		return NULL;
	
	return (mm.ml[mm.mem_num-1].base + mm.ml[mm.mem_num-1].size);
}

int mm_set_info(char *name, u32 *base, u32 size)
{
	struct memory_layout *lout;
	u32 i, len;
	if(size < 1) {
		mm_debug("Invalid parameter#%d\n", size);
		return -EINVAL;
	}

	len = kStrLen(name);
	if(len < 0 || len > 64) {
		mm_debug("Invalid parameter#%s\n", name);
		return -EINVAL;
	}

	if(mm.mem_num * sizeof(struct memory_layout) >= MM_HEAP_META_BASE - MM_META_BASE) {
		mm_debug("Out of memory#%s\n", mm.mem_num * sizeof(struct memory_layout));
		return -ENOMEM;
	}

	lout = mm.ml + mm.mem_num;

	kMemCpy(lout->name, name, len);
	lout->base = (u32)base;
	lout->size = size;
	mm.mem_num++;
}

static u32 mm_get_chunk(u8 dep)
{
	u32 chunk = mm.ll;
	while(dep--)
		chunk *= 2;

	return chunk;
}

static int mm_get_dep(const int heap_size, const int _ll)
{
	int i;
	int ll;

	if(heap_size<1 || _ll<1)
		return -EINVAL; 

	if(heap_size <= _ll)
		return err_dbg(-1, "ram size is less than lower limit. you will need to check it.\n");

	ll = _ll;
	for(i=0 ; ll<=heap_size ; i++) {
		if(heap_size/ll < BUD_MIN_CHUNK)	
			return i;

		ll *= 2;
	}

	return i>=BUD_MAX_ORD ? BUD_MAX_ORD : i;
}

static int mm_rndup(u32 size)
{
	u32 block = mm.ll, i = 0;
	u32 dep = mm.dep;

	if(size < 1 || size > mm.heap_size) {
		mm_debug("Invalid parameter#%d\n", size);
		return -EINVAL;
	}

	for(i = 0 ; i < dep; i++) {
		if(size <= block) {
			return block;
		}
		block *= 2;
	}

	return -1;
}

static int mm_get_ord(u32 chunk)
{
	u32 dep = mm.dep;
	u32 size = mm.ll, i;

	if(chunk < mm.ll || chunk > mm.ul)
		return err_dbg(-EINVAL, "Invalid Parameter#%d\n", chunk);

	for(i = 0 ; i < dep; i++) {
		if(chunk == size)
			return i;

		size *= 2;
	}	

	return 0;
}

static int bud_set_bit(struct bitmaps *bud, int ost, bool f)
{
	int err = -1;
	
	if(!bud)
		return err_dbg(-1, "err\n");

	if(ost < 0)	
		return err_dbg(-EINVAL, "Invalid Parameter(ost)#%d\n", ost);

	if(f != MM_BUD_FREE && f != MM_BUD_USED)	
		err_dbg(-EINVAL, "Invalid Parameter(f)#%d\n", f);

	err = bitmap_get(&bud->bmp, ost);
	if(err < 0)
		return err_dbg(-1, "err\n");

	if(err == f)
		return 0;

	bud->frees += f ? -1 : 1;

	err = bitmap_set(&bud->bmp, ost, !!f);
	if(err < 0)
		return err_dbg(err, "err\n");
		
	return 0;
}

static bool mm_is_bud_used(u32 ord, u32 ost)
{
	u32 bud_ost = ost + ((ost % 2) ? -1 : 1);
	
	return bitmap_get(&bud[ord].bmp, ost);
}

static bool mm_merge(u8 ord, u32 ost, bool f)
{
	bud_set_bit(&bud[ord], ost, !!f);

	if(ord == (mm.dep-1)) {
		mm_debug("this block is largest in buddy. So, it`s buddy is not exist.\n");	
		return false;
	}

	if(mm_is_bud_used(ord, ost))
		return true;
	
	return false;
}

static bool mm_search(u8 ord, u32 ost, bool f)
{
	if(bitmap_get(&bud[ord].bmp, ost))
		return true;

	bud_set_bit(&bud[ord], ost, !!f);
	return false;
}

static int mm_chk_parent(u32 ord, u32 ost, bool f, bool (*mm_is_stop)(u8, u32, bool))
{
	u32 i, n;
	
	if(ord < 0 || ord >= mm.dep) {
		mm_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	if(ost < 0) {
		mm_debug("Invalid Parameter#%d\n", ost);
		return -EINVAL;
	}

	for(i = ord+1, n = ost/2; i < mm.dep ; i++, n = n/2) {
		if(mm_is_stop(i, n, f)) {
			return 0;
		}
	}

	return 0;
}

static int mm_chk_childs(u32 ord, u32 ost, bool f)
{
	int i, j, n, k = 1;

	if(ord < 0 || ord >= mm.dep) {
		mm_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	if(ost < 0) {
		mm_debug("Invalid Parameter#%d\n", ost);
		return -EINVAL;
	}

	n = ost;
	for(i = ord-1 ; i>-1 ; i--) {
		n *= 2;
		k *= 2;
		for(j = n; j < n+k; j++) {
			bud_set_bit(&bud[i], j, !!f);
		}
	}
	/*
	n = ost;
	for(i = ord-1 ; i>-1 ; i--) {
		j = 0;
		n *= 2;
		k *= 2;
		m = (k / mm.bitmap_size) + (k % mm.bitmap_size);
		m = (log(2, m) > 5) ? 5 : log(2, m); 
	
		do {
			blk = (mm.bitmaps[i].block) + (n / mm.bitmap_size) + j;
			*blk = *blk | (~BITMAP_INIT32[m]);
			
			j++;	
		} while(j < (k / mm.bitmap_size));
	}
	*/

	return 0;
}

static int mm_find_free(int ord)
{
	int ost;

	if(ord < 0) {
		mm_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	ost = bitmap_get_free(&bud[ord].bmp);	
	if(ost < 0)
		return err_dbg(-4, "Available chunk is not exist... instead, you can request less more than the chunk\n");

	bud_set_bit(&bud[ord], ost, MM_BUD_USED);
	mm_chk_childs(ord, ost, MM_BUD_USED);	
	mm_chk_parent(ord, ost, MM_BUD_USED, mm_search);

	return ost;	
}

static void *mm_encode(u32 chunk, u32 ost, u8 dep)
{
	int err;
	void *addr = (void *)(mm.heap_base + (chunk*ost));
	struct mm_blk_hdr *hdr = (struct mm_blk_hdr *)(mm.hdr_base + ((chunk*ost)/mm.ll));

	hdr->dep = dep;
	if(((u32)(addr) + chunk) > mm.heap_base + mm.heap_size) {
		mm_debug("Out of memory#%d\n", (u32)(addr) + chunk);
		return NULL; 
	}

	return addr;
}

void* mm_alloc(u32 size, u32 flag)
{
	int chunk, ord, ost;
	void *addr;

	if(size < 1) {
		mm_debug("(0x%x)Invaid Parameter#%d\n", __builtin_return_address(0), size);
		return NULL;
	}

	chunk = mm_rndup(size);
	if(chunk < mm.ll)
		return NULL;	

	ord = mm_get_ord(chunk);
	if(ord < 0) 
		return NULL;

	ost = mm_find_free(ord);
	if(ost < 0) {
		mm_debug("Memory allocation failed#%d\n", ost);	
		return NULL;
	}
	
	addr = mm_encode(chunk, ost, ord);
	if(!addr)
		return NULL; 
	
	return addr;
}

static struct mm_blk_hdr *mm_decode(void *addr)
{
	u32 chunk_ost = ((u32)addr - mm.heap_base);
	struct mm_blk_hdr *hdr = (struct mm_blk_hdr *)(mm.hdr_base + (chunk_ost)/mm.ll);
	
	return hdr;
}

void mm_free(void *addr)
{
	struct mm_blk_hdr *hdr;
	u32 chunk_ost, chunk, ost, ord;

	if(!addr || (mm.heap_base > (u32)addr)) {
		mm_debug("Invalid parameter#0x%x\n", addr);
		return -EINVAL;
	}

	hdr = mm_decode(addr);	
	ord = hdr->dep;
	chunk = mm_get_chunk(hdr->dep);
	chunk_ost = ((u32)addr - mm.heap_base);
	ost = chunk_ost/ chunk;

	bud_set_bit(&bud[ord], ost, MM_BUD_FREE);
	mm_chk_childs(ord, ost, MM_BUD_FREE);
	if(!mm_is_bud_used(ord, ost)) {
		mm_chk_parent(ord, ost, MM_BUD_FREE, mm_merge);
	}
}

int bud_create(const u32 size, const int ll)
{

}

static int bud_get_real_size(const int _size, const int _ll)
{
	int size = 0, ll = 0, i = 0, dep = 0;
	if(_ll<1 || _size<1)
		return err_dbg(-1, "err\n");

	dep = mm_get_dep(_size, mm.ll);
	if(dep < 0)
		return err_dbg(-4, "err\n");

	size = _size;
	ll = _ll;

	u32 num[dep];

	size -= sizeof(struct bitmaps)*dep;
	
	for(i=0; i<dep ; i++) {
		num[i] = (((_size/ll) + (mm.bitmap_size-1)) / mm.bitmap_size)*BITMAP_DEF_BYTE;
		ll *= 2;
		size -= num[i];
	}

	size -= (_size/mm.ll)*sizeof(struct mm_blk_hdr);	

	if(size < 0)
		return err_dbg(-8, "size is too small, so that you don`t have any meta data about buddy\n");

	return size; 
}

int mm_init(u64 heap_size)
{
	_mm_init(heap_size, 0, 0);
}
// RAM 사이즈에 대한 디텍은 BIOS 나 UEFI를 통해서 하는게 가장 좋다고 한다.
// 이게 Memory Controller를 통해서 RAM 사이즈를 얻는 경우가 가장 좋다는거 같은데, BIOS 나 UEFI가 칩 벤더 레벨에서 알아서 이 정보를 가져와 주기 때문인 거 같다.
int _mm_init(const u32 heap_size, const int llc, const int ulo)
{
	int i, j, size = 0, rmd = 0, real_size = 0;
	u8 *bmp_base = NULL;
	u32 ll = 0;

	if(heap_size < MM_HEAP_MIN_SIZE)
		return err_dbg(-1, "Heap memory isn`t insufficient size#0x%x\n", heap_size);

	mm.org_size = heap_size;
	mm.ml = MM_META_BASE;
	mm.heap_meta_base = MM_HEAP_META_BASE;
	mm.ll = (llc < 1) ? MM_ALLOC_LL : llc;

	mm.bitmap_size = BITMAP_DEF_BIT;
	
	real_size = bud_get_real_size(heap_size, mm.ll);
	if(real_size < 0)
		err_dbg(-4, "Failed to calculate the heap size\n");	

	mm.heap_size = real_size;

	mm.dep = ulo < 1 ? mm_get_dep(mm.heap_size, mm.ll) : ulo;
	mm.ul = mm.ll * power(2, mm.dep-1);

	u32 num[mm.dep];
	
	// bitmaps management meta data
	bud = (struct bitmaps *)mm.heap_meta_base;
	
	memset(bud, 0, sizeof(struct bitmaps) * mm.dep);
	memset(num, 0, sizeof(num));	
	
	ll = mm.ll;
   	bmp_base = (u8 *)(bud + mm.dep);
	size = sizeof(struct bitmaps)*mm.dep;

	mm_set_info("BUDDY BITMAPS MANAGEMENT", bud, size);

	size = 0;	
	for(i=0; i<mm.dep ; i++) {
		num[i] = ((real_size/ll) + (mm.bitmap_size-1)) / mm.bitmap_size;
		
		bud[i].bmp.base = bmp_base;	
		bud[i].num = real_size / ll;
		bud[i].frees = bud[i].num;
		bud[i].bmp.size = num[i];
		bud[i].bmp.bit = mm.bitmap_size;		

		memset(bud[i].bmp.base, 0, bud[i].bmp.size*(bud[i].bmp.bit/8));	
		for(j=((real_size/ll)%mm.bitmap_size) ; j<mm.bitmap_size ; j++)
			bitmap_set(&bud[i].bmp, (num[i]-1)*mm.bitmap_size+j, 1);

		bmp_base += (bud[i].bmp.size*(bud[i].bmp.bit/8));
		ll *= 2;
		size += (bud[i].bmp.size*(bud[i].bmp.bit/8));
	}

	mm_set_info("BUDDY BITMAPS", bud[0].bmp.base, size);
	
	// bitmaps heap header meta data
	mm.hdr_base = bud[0].bmp.base + size;
	mm.hdr_size = (real_size/mm.ll) * sizeof(struct mm_blk_hdr);
	
	size = 0;	
	size = mm.hdr_size;	

	mm_set_info("BUDDY BITMAPS HEADER", mm.hdr_base, mm.hdr_size);

	// heap base
	mm.heap_base = (u32)(mm.hdr_base + mm.hdr_size);
	mm.heap_meta_size = mm.heap_base - mm.heap_meta_base;

	mm_set_info("KERNEL HEAP", mm.heap_base, mm.heap_size);

	mm.rmd_size = heap_size - (mm.heap_size + mm.heap_meta_size); 
	pool_init(mm.heap_base + mm.heap_size, mm.rmd_size);

	return 0;
}

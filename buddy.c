#include "mmi.h"
#include "buddy.h"
#include "type.h"
#include "Console.h"
#include "errno.h"
#include "mm.h"
#include "debug.h"
#include "pool.h"
#include "bitmap.h"
#include "math.h"

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

#define BUD_FREE		0x00000000
#define BUD_USED		0x00000001

#define BUD_MAX_ORD	 		(11)
#define BUD_MIN_CHUNK		(8)

struct bud_manager {
	// buddy
	u32 dep;
	u8 bitmap_size;
	u32 ll;	// low limit
	u32 ul; // upper limit
	struct bud_blk_hdr *hdr_base;
	u32 hdr_size;
	//struct bitmaps *bitmaps;
	
	struct list_node ml_list;
	u32 mem_num;

	// memory info
	u32 page_size;
	s64 org_size;
	u32 heap_ul_size;
	u32 heap_meta_base;
	s64 heap_meta_size;
	u32 heap_base;
	s64 heap_size;
	s64 rmd_size;
};

struct bitmaps *bud;
struct lazy_buddy lazy; 
struct bud_manager bm;

static void bud_show_info();
static int bud_init(const void *base, const s64 size);
static void _bud_free(const void *addr);
static void bud_free(const void* addr);
static void *_bud_alloc(const int size);
static void *bud_alloc(const int size);
static int bud_rndup(const int size);

struct mmif pm_if = {
	.mm_init = bud_init,	
	.mm_free = bud_free,
	.mm_alloc = bud_alloc,
	.show_info = bud_show_info,
	.get_chunk = bud_rndup,
};

static void bud_show_info()
{
	int i; 

	bud_debug("YohdaOS Buddy Info\n");
	bud_debug("Original size#0x%x\n", bm.org_size);
	//bud_debug("Data size#0x%x\n", bm.heap_size);
	bud_debug("Data size#0x%x\n", bm.heap_size);
	bud_debug("Meta data size#0x%x\n", bm.heap_meta_size);
	bud_debug("Remaider size#0x%x\n", bm.rmd_size);
	bud_debug("Buddy dep#%d\n", bm.dep);
	bud_debug("Buddy Upper limit#%d\n", bm.ul);
	bud_debug("Buddy Lower limit#%d\n", bm.ll);
	for(i=0 ; i<bm.dep ; i++) {
		bud_debug("Bitmap Order#%d\n", i);
		bud_debug("Size#%d, Number#%d, Frees#%d\n", bud[i].bmp.size, bud[i].num, bud[i].frees);
	}
}

static int bud_get_chunk(const int _dep)
{
	u32 chunk = bm.ll, dep = 0;
	
	if(_dep < 0)
		return err_dbg(-1, "err\n");		
	
	dep = _dep;
	while(dep--)
		chunk *= 2;

	return chunk;
}

static int bud_get_dep(const s64 heap_size, const int _ll)
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
		
		if(i >= BUD_MAX_ORD)
			return i;

		ll *= 2;
	}
	
	return i;
}

static int bud_rndup(const int size)
{
	u32 block = bm.ll, i = 0;
	u32 dep = bm.dep;

	if(size < 1 || size > bm.heap_size)
		return err_dbg(-EINVAL, "Invalid parameter#%d\n", size);

	for(i = 0 ; i < dep; i++) {
		if(size <= block) {
			return block;
		}
		block *= 2;
	}

	return -1;
}

static int bud_get_ord(u32 chunk)
{
	u32 dep = bm.dep;
	u32 size = bm.ll, i;

	if(chunk < bm.ll || chunk > bm.ul)
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

	if(f != BUD_FREE && f != BUD_USED)	
		return err_dbg(-EINVAL, "Invalid Parameter(f)#%d\n", f);

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

static bool bud_is_bud_used(u32 ord, u32 ost)
{
	u32 bud_ost = ost + ((ost % 2) ? -1 : 1);
	
	return bitmap_get(&bud[ord].bmp, bud_ost);
}

static bool bud_merge(u8 ord, u32 ost, bool f)
{
	bud_set_bit(&bud[ord], ost, !!f);

	if(ord == (bm.dep-1)) {
		bud_debug("this block is largest in buddy. So, it`s buddy is not exist.\n");	
		return false;
	}

	if(bud_is_bud_used(ord, ost))
		return true;
	
	return false;
}

static bool bud_search(u8 ord, u32 ost, bool f)
{
	if(bitmap_get(&bud[ord].bmp, ost))
		return true;

	bud_set_bit(&bud[ord], ost, !!f);
	return false;
}

static int bud_chk_parent(u32 ord, u32 ost, bool f, bool (*bud_is_stop)(u8, u32, bool))
{
	u32 i, n;
	
	if(ord < 0 || ord >= bm.dep) {
		bud_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	if(ost < 0) {
		bud_debug("Invalid Parameter#%d\n", ost);
		return -EINVAL;
	}

	for(i = ord+1, n = ost/2; i < bm.dep ; i++, n = n/2) {
		if(bud_is_stop(i, n, f)) {
			return 0;
		}
	}

	return 0;
}

static int bud_chk_childs(u32 ord, u32 ost, bool f)
{
	int i, j, n, k = 1;

	if(ord < 0 || ord >= bm.dep) {
		bud_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	if(ost < 0) {
		bud_debug("Invalid Parameter#%d\n", ost);
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

	return 0;
}

static int bud_find_free(int ord)
{
	int ost;

	if(ord < 0) {
		bud_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	ost = bitmap_get_free(&bud[ord].bmp);	
	if(ost < 0)
		return err_dbg(-4, "Available chunk is not exist... instead, you can request less more than the chunk\n");

	bud_set_bit(&bud[ord], ost, BUD_USED);
	bud_chk_childs(ord, ost, BUD_USED);	
	bud_chk_parent(ord, ost, BUD_USED, bud_search);

	return ost;	
}

static void *bud_encode(u32 chunk, u32 ost, u8 dep)
{
	int err;
	void *addr = (void *)(bm.heap_base + (chunk*ost));
	struct bud_blk_hdr *hdr = (struct bud_blk_hdr *)(bm.hdr_base + ((chunk*ost)/bm.ll));

	hdr->dep = dep;
	if(((u32)(addr) + chunk) > bm.heap_base + bm.heap_size) {
		bud_debug("Out of memory#%d\n", (u32)(addr) + chunk);
		return NULL; 
	}

	return addr;
}

void *bud_lazy_alloc(const int size)
{
	struct list_node *node = NULL;
	struct lazy_page *page = NULL;
	int i;

	if(!lazy.num) {
		int batch = lazy.batch; 	
		struct lazy_page *pages = _bud_alloc(sizeof(struct lazy_page)*batch);
		if(!pages)
			return err_dbg(NULL, "err\n");

		for(i=0 ; i<batch ; i++) {
			pages[i].addr = _bud_alloc(MM_PAGE_SIZE);
			if(!pages[i].addr)
				return err_dbg(NULL, "err\n");

			list_add(&lazy.list, &pages[i].node); 					
			lazy.num++;
		}
	} 
	
	node = list_get(&lazy.list);
	if(!node)
		return err_dbg(NULL, "err\n");

	page = container_of(node, struct lazy_page, node);
	if(!page)
		return err_dbg(NULL, "err\n");

	lazy.num--;

	return page->addr;
}

static void *bud_alloc(const int size)
{
	void *addr = NULL;

	if(size < 1)
		return err_dbg(-EINVAL , "Invalid Paramter#%d\n", size);	

	if(size > MM_PAGE_SIZE) {
		addr = _bud_alloc(size);
	} else {
		addr = bud_lazy_alloc(size);
	}
	
	if(!addr)
		return err_dbg(-4 , "err\n");	

	return addr;	
}

static void *_bud_alloc(const int size)
{
	int chunk, ord, ost;
	void *addr;

	if(size < 1) {
		bud_debug("(0x%x)Invaid Parameter#%d\n", __builtin_return_address(0), size);
		return NULL;
	}

	chunk = bud_rndup(size);
	if(chunk < 0)
		return NULL;	

	ord = bud_get_ord(chunk);
	if(ord < 0) 
		return NULL;

	ost = bud_find_free(ord);
	if(ost < 0) {
		bud_debug("Memory allocation failed#%d\n", ost);	
		return NULL;
	}
	
	addr = bud_encode(chunk, ost, ord);
	if(!addr)
		return NULL; 
	
	return addr;
}

static struct bud_blk_hdr *bud_decode(void *addr)
{
	u32 chunk_ost = ((u32)addr - bm.heap_base);
	struct bud_blk_hdr *hdr = (struct bud_blk_hdr *)(bm.hdr_base + (chunk_ost)/bm.ll);
	
	return hdr;
}

int bud_lazy_free(const void *addr)
{
	struct lazy_page *lp;	
	if(!addr)
		return err_dbg(-1, "err\n");
	
	lp = _bud_alloc(sizeof(struct lazy_page));
	if(!lp)
		return err_dbg(-4, "err\n");

	lp->addr = addr;	

	list_add(&lazy.list, &lp->node); 
	lazy.num++;

	if(lazy.num > lazy.wmk) {
		int batch = lazy.batch; 	
		struct lazy_page *page; 
		struct list_node *node;

		while(batch--) {
			node = list_get(&lazy.list);
			if(!node)
				return err_dbg(-8, "err\n");

			page = container_of(node, struct lazy_page, node);
			if(!page)
				return err_dbg(-12, "err\n");

			_bud_free(page->addr);
		
			lazy.num--;		
		}	
	}	

	return 0;
}

static void bud_free(const void *addr)
{
	struct bud_blk_hdr *hdr;
	int chunk, ost, ord;

	if(!addr || (bm.heap_base > (u32)addr))
		return err_dbg(-EINVAL, "Invalid parameter#0x%x\n", addr);

	hdr = bud_decode(addr);	
	ord = hdr->dep;
	chunk = bud_get_chunk(hdr->dep);
	if(chunk < 0)
		return err_dbg(-12, "err\n");

	if(chunk <= MM_PAGE_SIZE) {
		bud_lazy_free(addr);
	} else {
		_bud_free(addr);
	}
}

static void _bud_free(const void *addr)
{
	struct bud_blk_hdr *hdr;
	int chunk_ost, chunk, ost, ord;

	if(!addr || (bm.heap_base > (u32)addr))
		return err_dbg(-EINVAL, "Invalid parameter#0x%x\n", addr);

	hdr = bud_decode(addr);	
	ord = hdr->dep;
	chunk = bud_get_chunk(hdr->dep);
	if(chunk < 0)
		return err_dbg(-12, "err\n");

	chunk_ost = ((u32)addr - bm.heap_base);
	ost = chunk_ost/ chunk;

	bud_set_bit(&bud[ord], ost, BUD_FREE);
	bud_chk_childs(ord, ost, BUD_FREE);
	if(!bud_is_bud_used(ord, ost)) {
		bud_chk_parent(ord, ost, BUD_FREE, bud_merge);
	}
}

static s64 bud_get_real_size(const s64 _size, const int _ll)
{
	int ll = 0, i = 0, dep = 0;
	s64 size = 0;

	if(_ll<1 || _size<1)
		return err_dbg(-1, "err\n");

	dep = bud_get_dep(_size, bm.ll);
	if(dep < 0)
		return err_dbg(-4, "err\n");

	size = _size;
	ll = _ll;

	u32 num[dep];

	size -= sizeof(struct bitmaps)*dep;
	
	for(i=0; i<dep ; i++) {
		num[i] = (((_size/ll) + (bm.bitmap_size-1)) / bm.bitmap_size)*BITMAP_DEF_BYTE;
		ll *= 2;
		size -= num[i];
	}

	size -= (_size/bm.ll)*sizeof(struct bud_blk_hdr);	

	if(size < 0)
		return err_dbg(-8, "size is too small, so that you don`t have any meta data about buddy\n");

	return size; 
}

static int _bud_init(const void* base, const s64 heap_size, const int llc)
{
	int i, j, rmd = 0;
	u8 *bmp_base = NULL;
	s64 size = 0, data_size = 0; 
	u32 ll = 0;
	
	if(!base)
		return err_dbg(-1, "Buddy base addres is NULL\n");
		
	if(heap_size < MM_HEAP_MIN_SIZE)
		return err_dbg(-4, "Heap memory isn`t insufficient size#0x%x\n", heap_size);

	if(llc < 1)
		return err_dbg(-8, "Need to modify the appriciate low limit chank size");

	bm.org_size = heap_size;
	bm.heap_meta_base = base;
	bm.ll = llc;

	bm.bitmap_size = BITMAP_DEF_BIT;
	
	data_size = bud_get_real_size(heap_size, bm.ll);
	if(data_size < 0)
		return err_dbg(-4, "Failed to calculate the heap size\n");	

	bm.heap_size = data_size;
	bm.dep = bud_get_dep(bm.heap_size, bm.ll);
	if(bm.dep < 0)
		return err_dbg(-12, "error to calculate buddy depth\n");
	
	bm.ul = bm.ll * power(2, bm.dep-1);

	u32 num[bm.dep];
	
	// bitmaps management meta data
	bud = (struct bitmaps *)bm.heap_meta_base;
	
	memset(bud, 0, sizeof(struct bitmaps) * bm.dep);
	memset(num, 0, sizeof(num));	
	
	ll = bm.ll;
   	bmp_base = (u8 *)(bud + bm.dep);
	size = sizeof(struct bitmaps)*bm.dep;

	//mm_set_info("BUDDY BITMAPS MANAGEMENT", bud, size);

	size = 0;	
	for(i=0; i<bm.dep ; i++) {
		num[i] = ((data_size/ll) + (bm.bitmap_size-1)) / bm.bitmap_size;
		
		bud[i].bmp.base = bmp_base;	
		bud[i].num = data_size / ll;
		bud[i].frees = bud[i].num;
		bud[i].bmp.size = num[i];
		bud[i].bmp.bit = bm.bitmap_size;		

		memset(bud[i].bmp.base, 0, bud[i].bmp.size*(bud[i].bmp.bit/8));	
		for(j=((data_size/ll)%bm.bitmap_size) ; j<bm.bitmap_size ; j++)
			bitmap_set(&bud[i].bmp, (num[i]-1)*bm.bitmap_size+j, 1);

		bmp_base += (bud[i].bmp.size*(bud[i].bmp.bit/8));
		ll *= 2;
		size += (bud[i].bmp.size*(bud[i].bmp.bit/8));
	}

	//mm_set_info("BUDDY BITMAPS", bud[0].bmp.base, size);
	
	// bitmaps heap header meta data
	bm.hdr_base = bud[0].bmp.base + size;
	bm.hdr_size = (data_size/bm.ll) * sizeof(struct bud_blk_hdr);
	
	size = 0;	
	size = bm.hdr_size;	

	//mm_set_info("BUDDY BITMAPS HEADER", bm.hdr_base, bm.hdr_size);

	// heap base
	bm.heap_base = (u32)(bm.hdr_base + bm.hdr_size);
	bm.heap_meta_size = bm.heap_base - bm.heap_meta_base;

	//mm_set_info("KERNEL HEAP", bm.heap_base, bm.heap_size);

	bm.rmd_size = heap_size - (bm.heap_size + bm.heap_meta_size); 

	// Setup lazy buddy
	lazy.batch = min((bm.heap_size / MM_PAGE_SIZE) - 1, 31); 	
	lazy.wmk = lazy.batch * 6;

	list_init_head(&lazy.list);

	return bm.rmd_size;
}

int bud_init(const void *base, const s64 heap_size)
{
	_bud_init(base, heap_size, 2048);
}

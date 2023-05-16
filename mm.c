#include "Types.h"
#include "Console.h"
#include "errno.h"
#include "mm.h"
#include "debug.h"

/* For Test */
#define MM_TEST_HEAP128K_LL2048_BITMAP32x 

/* 64-bit yohdaOS Memory Layout */
#define MM_BASE				(0x2000000) 				// 32MB
#define MM_META_BASE 		(MM_BASE)					// 32MB
#define MM_HEAP_META_BASE	((MM_META_BASE)|(0x10000))	// 32MB + 64KB
#define MM_HEAP_DATA_BASE  	(0x2100000)

// Actually Physical RAM size is 4GB. But, it is imposible to assign all 4GB. The parts in it are used in boot and system. 
#define MM_RAM_SIZE 	(1*1024*1024*1024) 	// 1GB

#define MM_PAGE_SIZEx	
// Upper Limit
#define MM_ALLOC_UL		()
// Low Limit
#define MM_ALLOC_LL 	0x800 // I think it is better to select the size same as page, or more than it.

#define MM_64_BASE_ADDR 	0xA00000  // 10MB

// I think that it is able to change the a size of bitmaps. In your computer with operating 64-bit, it can be 64.

#define MM_BUD_FREE		0x00000000
#define MM_BUD_USED		0x00000001

u32 BITMAP_INIT32[]= { 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF0, 0xFFFFFF00, 0xFFFF0000, 0x00000000 };
u64 BITMAP_INIT64[] = { 0x0000000000000000, 0xFFFFFFFE00000000, 0xFFFFFFFC00000000, 0xFFFFFFF000000000, 0xFFFFFF0000000000, 0xFFFF000000000000 , 0xFFFFFFFF00000000 };

#define MM_BITMAP_INIT(x) BITMAP_INIT##mm.bitmap_size

struct mm_manager {
	// buddy
	u32 dep;
	u8 bitmap_size;
	u32 ll;
	struct mm_blk_hdr *hdr_base;
	u32 hdr_size;
	struct bitmaps *bitmaps;

	// mem layout
	struct memory_layout *ml;
	struct list_node ml_list;
	u32 mem_num;

	// memory info
	u32 page_size;
	u32 heap_ul_size;
	u32 heap_meta_base;
	u32 heap_meta_size;
	u32 heap_base;
	u32 heap_size;
	u32 rmd_size;
};

struct mm_manager mm;
u32 **blocks;

int mm_show_info()
{
	u32 i;
	for(i = 0 ; i < mm.mem_num ; i++) {	
		mm_debug("name#%s\n",mm.ml[i].name);
		mm_debug("base#0x%x, size#0x%x\n", mm.ml[i].base, mm.ml[i].size);
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

static int mm_get_dep(u32 heap_size, u32 ll)
{
	int i;
	int size;

	if(heap_size < 1 || ll < 1)
		return -EINVAL; 

	if(heap_size <= ll) {
		mm_debug("ram size is less than lower limit. you will need to check it.\n");
		return -1;
	}

	size = ll;
	for(i = 0 ; ll <= heap_size ; i++)
		ll *= 2;

	return i;
}

// RAM 사이즈에 대한 디텍은 BIOS 나 UEFI를 통해서 하는게 가장 좋다고 한다.
// 이게 Memory Controller를 통해서 RAM 사이즈를 얻는 경우가 가장 좋다는거 같은데, BIOS 나 UEFI가 칩 벤더 레벨에서 알아서 이 정보를 가져와 주기 때문인 거 같다.
int mm_init(u64 heap_size)
{
	int i, j;
	// automatically calculate RAM size ... and assign each number of order-n-block bitmaps
	heap_size = MM_RAM_SIZE;
#ifdef MM_TEST_HEAP128K_LL2048_BITMAP32
	heap_size = 0x20000|0x200;
#endif
	u32 ll;

	mm.ml = MM_META_BASE;
	mm.heap_meta_base = MM_HEAP_META_BASE;
	mm.ll = 2048;
#ifdef MM_TEST_HEAP128K_LL2048_BITMAP32
	mm.ll = 2048;
#endif

	mm.bitmap_size = 32;
#ifdef MM_TEST_HEAP128K_LL2048_BITMAP32
	mm.bitmap_size = 32;
#endif

	mm.dep = mm_get_dep(heap_size, mm.ll);
	mm.heap_size = mm.ll * power(2, mm.dep-1); 
	mm.rmd_size = heap_size - mm.heap_size;

	mm_debug("dep#%d heap#%d rmd#%d\n", mm.dep, mm.heap_size, mm.rmd_size);

	//mm.heap_size = 0x10000; // 32KB for test

	u32 num[mm.dep];

	ll = mm.ll; 
	blocks[0] = (u32 *)mm.heap_meta_base;
	
	// bitmaps heap meta data
	for(i = 0; i < mm.dep ; i++) {
		num[i] = (mm.heap_size / ll);
		num[i] = (num[i] / mm.bitmap_size) + ((num[i] % mm.bitmap_size) ? 1 : 0) ;
	
		//mm_debug("dep#%d, mem#0x%x, block-size#%d, num#%d\n", i, blocks[i], ll, num[i]);
		ll *= 2;

		blocks[i+1] = blocks[i] + num[i];
		kMemSet(blocks[i], BITMAP_INIT32[5], (u32)blocks[i+1] - (u32)blocks[i]);	
	}

	j = log(2, mm.bitmap_size);
	for(i = mm.dep-j ; j > -1 ; j--, i++) {
		*blocks[i] = BITMAP_INIT32[j-1];
	}

	mm_set_info("BUDDY BITMAPS", blocks[0], (u32)(blocks[mm.dep]) - (u32)(blocks[0]));
	
	// bitmaps management meta data
	mm.bitmaps = (struct bitmaps *)(blocks[mm.dep-1] + num[mm.dep-1]);	
	for(i = 0 ; i < mm.dep ; i++) {
		mm.bitmaps[i].ord = i;
		mm.bitmaps[i].len = num[i];
		mm.bitmaps[i].block = blocks[i];	
	}

	mm_set_info("BUDDY BITMAPS MANAGEMENT", mm.bitmaps, (u32)(mm.bitmaps + (mm.dep)) - (u32)mm.bitmaps);

	// bitmaps heap header meta data
	mm.hdr_base = (u8 *)(mm.bitmaps + mm.dep);
	mm.hdr_size = (mm.heap_size / mm.ll) * sizeof(struct mm_blk_hdr);
	
	mm_set_info("BUDDY BITMAPS HEADER", mm.hdr_base, mm.hdr_size);

	// heap base
	mm.heap_base = (u32)(mm.hdr_base + mm.hdr_size);
	//mm.heap_size += mm.heap_hd_size;

	mm_set_info("KERNEL HEAP", mm.heap_base, mm.heap_size);

#ifdef MM_TEST_HEAP128K_LL2048_BITMAP32
	void *a1 = mm_alloc(15400, MM_KL);
	//mm_debug("15.4K#0x%x\n", a1);
	mm_debug("ord#0 block#0x%x\n", *blocks[0]);
	mm_debug("ord#1 block#0x%x\n", *blocks[1]);
	mm_debug("ord#2 block#0x%x\n", *blocks[2]);
	mm_debug("ord#3 block#0x%x\n", *blocks[3]);
	mm_debug("ord#4 block#0x%x\n", *blocks[4]);
	mm_debug("ord#5 block#0x%x\n", *blocks[5]);
	mm_debug("ord#6 block#0x%x\n", *blocks[6]);

	//void *a2 = mm_alloc(6700);
	//mm_debug("6.7K#0x%x\n", a2);
	//mm_debug("ord#0 block#0x%x\n", *blocks[0]);
	//mm_debug("ord#1 block#0x%x\n", *blocks[1]);
	//mm_debug("ord#2 block#0x%x\n", *blocks[2]);
	//mm_debug("ord#3 block#0x%x\n", *blocks[3]);
	//mm_debug("ord#4 block#0x%x\n", *blocks[4]);
	//mm_debug("ord#5 block#0x%x\n", *blocks[5]);
	//mm_debug("ord#6 block#0x%x\n", *blocks[6]);

	//void *a3 = mm_alloc(1200);
	//mm_debug("1.2K#0x%x\n", a3);

	//void *a4 = mm_alloc(43700);
	//mm_debug("43.7K#0x%x\n", a4);
	
	//void *a5 = mm_alloc(27700);
	//mm_debug("27.7K#0x%x\n", a5);
	
	//void *a6 = mm_alloc(27700); // Out of memory
	//mm_debug("27.7K#0x%x\n", a6);
	
	//void *a7 = mm_alloc(3029);

	//void *a8 = mm_alloc(2200); // Out of memory
	//mm_debug("2.2K#0x%x\n", a8);
	
	//void *a9 = mm_alloc(892);
	
	//mm_free(a3);
	//mm_free(a8);
	//mm_free(a9);
	//mm_free(a7);
#endif

	return 0;
}

static int mm_rndup(u32 size)
{
	u32 block = mm.ll, i;
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

	if(chunk < mm.ll || chunk > mm.heap_size) {
		mm_debug("Invalid Parameter#%d\n", chunk);
		return -EINVAL;
	}

	for(i = 0 ; i < dep; i++) {
		if(chunk == size)
			return i;

		size *= 2;
	}	

	return 0;
}

static int mm_find_free_bit(u32 dep, u32 byte_ost)
{
	u32* blk;
 	u32 i;

	if(dep < 0) {
		mm_debug("Invalid Parameter(dep)#%d\n", dep);
		return -EINVAL;
	}

	if(byte_ost > mm.bitmaps[dep].len || byte_ost < 0) {
		mm_debug("Invalid Parameter(byte_ost)#%d\n", byte_ost);
		return -EINVAL;
	}

	blk = mm.bitmaps[dep].block + byte_ost;
	for(i = 0 ; i < mm.bitmap_size ; i++) {
		if(!(((*blk) >> i) & 0x00000001)) {
			*blk = (*blk) | (0x01 << i); // From now on, this block will be in used.	
			return i;
		}
	}

	return -1;	
}

// If founded the free a block, the function will return location of it unit a byte. 
static int mm_find_free_byte(u32 dep)
{
	u32 i;
	u32 *blk;

	if(dep < 0) {
		mm_debug("Invalid Parameter#%d\n", dep);
		return -EINVAL;
	}

	for(i = 0 ; i < mm.bitmaps[dep].len ; i++) {
		blk = (mm.bitmaps[dep].block) + i;
		if(((*blk) & 0xFFFFFFFF) == 0xFFFFFFFF)
			continue;

		return i;
	}
		
	return -1;
}

static int mm_set_bit(u32 dep, u32 ost, u32 f)
{
	u32 *blk;
	u8 ret;
	if(ost < 0) {	
		mm_debug("Invalid Parameter(ost)#%d\n", ost);
		return -EINVAL;
	}

	if(f != MM_BUD_FREE && f != MM_BUD_USED) {	
		mm_debug("Invalid Parameter(f)#%d\n", f);
		return -EINVAL;
	}
 
	blk = (mm.bitmaps[dep].block) + (ost / mm.bitmap_size);
	if(f == MM_BUD_USED) {
		*blk = *blk | (f << (ost % mm.bitmap_size));
	} else {
		*blk = *blk & (~((0x01) << (ost % mm.bitmap_size)));
	}

	return 0;
}

static bool mm_is_used(u32 ord, u32 ost)
{
	u32 *blk = (mm.bitmaps[ord].block) + (ost / mm.bitmap_size);

	return (*blk & (0x01 << (ost % mm.bitmap_size)));
}

static bool mm_is_bud_used(u32 ord, u32 ost)
{
	u32 bud_ost = ost + ((ost % 2) ? -1 : 1);
	
	return mm_is_used(ord, bud_ost);
}

static bool mm_merge(u8 ord, u32 ost, bool f)
{
	mm_set_bit(ord, ost, !!f);

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
	if(mm_is_used(ord, ost))
		return true;

	mm_set_bit(ord, ost, !!f);
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
			mm_set_bit(i, j, !!f);
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
	int ost, byte_ost, bit_ost;

	if(ord < 0) {
		mm_debug("Invalid Parameter#%d\n", ord);
		return -EINVAL;
	}

	byte_ost = mm_find_free_byte(ord);
	if(byte_ost < 0) {
		mm_debug("requested size of block is not exist\n");
		return byte_ost;
	}

	bit_ost = mm_find_free_bit(ord, byte_ost);
	if(bit_ost < 0)
		return bit_ost;

	ost = (byte_ost * mm.bitmap_size) + bit_ost;
	
	mm_set_bit(ord, ost, MM_BUD_USED);
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
	//mm_debug("hdr_base#0x%x\n", hdr_base);
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
		mm_debug("Invaid Parameter#%d\n", size);
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

	mm_set_bit(ord, ost, MM_BUD_FREE);
	mm_chk_childs(ord, ost, MM_BUD_FREE);
	if(!mm_is_bud_used(ord, ost)) {
		mm_chk_parent(ord, ost, MM_BUD_FREE, mm_merge);
	}
}


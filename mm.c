#include "Types.h"
#include "Console.h"
#include "errno.h"
#include "mm.h"
#include "debug.h"

/* For Test */
#define MM_TEST_HEAP_128K 
#define MM_TEST_LL_2048
#define MM_TEST_BITMAP_32

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

u32 BITMAP_INIT32[]= { 0x00000000, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF0, 0xFFFFFF00, 0xFFFF0000 };
u64 BITMAP_INIT64[] = { 0x0000000000000000, 0xFFFFFFFE00000000, 0xFFFFFFFC00000000, 0xFFFFFFF000000000, 0xFFFFFF0000000000, 0xFFFF000000000000 , 0xFFFFFFFF00000000 };

#define MM_BITMAP_INIT(x) BITMAP_INIT##mm.bitmap_size

struct mm_manager {
	// buddy
	u32 dep;
	u8 bitmap_size;
	u32 ll;
	struct bitmaps *bitmaps;

	// mem layout
	struct memory_layout *ml;
	struct list_head ml_list;
	u32 mem_num;

	// memory info
	u32 page_size;
	u32 heap_ul_size;
	u32 heap_meta_base;
	u32 heap_meta_size;
	u32 heap_base;
	u32 heap_size;
	u32 heap_hd_size;
	u32 rmd_size;
};

struct mm_manager mm;
u32 **blocks;

int mm_show_info()
{
	u32 i;
	for(i = 0 ; i < mm.mem_num ; i++) {	
		debug("name#%s\n",mm.ml[i].name);
		debug("base#0x%x, size#0x%x\n", mm.ml[i].base, mm.ml[i].size);
	}
}

int mm_set_info(char *name, u32 *base, u32 size)
{
	struct memory_layout *lout;
	u32 i, len;
	if(size < 1) {
		debug("Invalid parameter#%d\n", size);
		return -EINVAL;
	}

	len = kStrLen(name);
	if(len < 0 || len > 64) {
		debug("Invalid parameter#%s\n", name);
		return -EINVAL;
	}

	if(mm.mem_num * sizeof(struct memory_layout) >= MM_HEAP_META_BASE - MM_META_BASE) {
		debug("Out of memory#%s\n", mm.mem_num * sizeof(struct memory_layout));
		return -ENOMEM;
	}

	lout = mm.ml + mm.mem_num;

	kMemCpy(lout->name, name, len);
	lout->base = (u32)base;
	lout->size = size;
	mm.mem_num++;
}

static int mm_get_dep(u32 heap_size, u32 ll)
{
	int i;
	int size;

	if(heap_size < 1 || ll < 1)
		return -EINVAL; 

	if(heap_size <= ll) {
		debug("ram size is less than lower limit. you will need to check it.\n");
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
#ifdef MM_TEST_HEAP_128K
	heap_size = 0x20000|0x200;
#endif
	u32 ll;

	mm.ml = MM_META_BASE;
	mm.heap_meta_base = MM_HEAP_META_BASE;
	mm.ll = 2048;
#ifdef MM_TEST_LL_2048
	mm.ll = 2048;
#endif

	mm.bitmap_size = 32;
#ifdef MM_TEST_BITMAP_32
	mm.bitmap_size = 32;
#endif

	mm.dep = mm_get_dep(heap_size, mm.ll);
	mm.heap_size = mm.ll * power(2, mm.dep-1); 
	mm.rmd_size = heap_size - mm.heap_size;

	debug("dep#%d heap#%d rmd#%d\n", mm.dep, mm.heap_size, mm.rmd_size);

	//mm.heap_size = 0x10000; // 32KB for test

	u32 num[mm.dep];

	ll = mm.ll;
	blocks[0] = (u32 *)mm.heap_meta_base;
	for(i = 0; i < mm.dep ; i++) {
		num[i] = (mm.heap_size / ll);
		num[i] = (num[i] / mm.bitmap_size) + ((num[i] % mm.bitmap_size) ? 1 : 0) ;
	
		debug("dep#%d, mem#0x%x, block-size#%d, num#%d\n", i, blocks[i], ll, num[i]);
		ll *= 2;

		blocks[i+1] = blocks[i] + num[i];
		kMemSet(blocks[i], BITMAP_INIT32[0], (u32)blocks[i+1] - (u32)blocks[i]);	
	}

	j = log(2, mm.bitmap_size);
	for(i = mm.dep-j ; j > 0 ; j--, i++) {
		*blocks[i] = BITMAP_INIT32[j];
	}

	mm_set_info("BUDDY BITMAPS", blocks[0], (u32)(blocks[mm.dep]) - (u32)(blocks[0]));
	
	mm.heap_hd_size = (mm.heap_size / mm.ll) * sizeof(struct mm_blk_hdr);
	mm.bitmaps = (struct bitmaps *)(blocks[mm.dep-1] + num[mm.dep-1]);	
	for(i = 0 ; i < mm.dep ; i++) {
		mm.bitmaps[i].ord = i;
		mm.bitmaps[i].len = num[i];
		mm.bitmaps[i].block = blocks[i];	
	}

	mm_set_info("BUDDY BITMAPS MANAGEMENT", mm.bitmaps, (u32)(mm.bitmaps + (mm.dep)) - (u32)mm.bitmaps);

	mm.heap_base = (u32)(mm.bitmaps + mm.dep);
	mm.heap_size += mm.heap_hd_size;

	mm_set_info("KERNEL HEAP", mm.heap_base, mm.heap_size);

	//debug("mm.heap_hd_size#0x%x\n", mm.heap_hd_size);
	//debug("mm.heap_size#0x%x\n", mm.heap_size);
	
	void *a1 = mm_alloc(15400);
	//debug("15.4K#0x%x\n", a1);
	//debug("3#0x%x\n", *(mm.bitmaps[3].block));
	//debug("2#0x%x\n", *(mm.bitmaps[2].block));
	//debug("1#0x%x\n", *(mm.bitmaps[1].block));
	//debug("0#0x%x\n", *(mm.bitmaps[0].block));

	void *a2 = mm_alloc(6700);
	//debug("6.7K#0x%x\n", a2);

	void *a3 = mm_alloc(1200);
	//debug("1.2K#0x%x\n", a3);

	void *a4 = mm_alloc(43700);
	//debug("43.7K#0x%x\n", a4);

	void *a5 = mm_alloc(27700);
	//debug("27.7K#0x%x\n", a5);
	
	mm_free(a1);
	mm_free(a2);
	mm_free(a3);
	mm_free(a4);
	mm_free(a5);

	return 0;
}

static int mm_rndup(u32 size)
{
	u32 block = mm.ll, i;
	u32 dep = mm.dep;

	if(size < 1 || size > mm.heap_size) {
		debug("Invalid parameter#%d\n", size);
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
		debug("Invalid Parameter#%d\n", chunk);
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
		debug("Invalid Parameter(dep)#%d\n", dep);
		return -EINVAL;
	}

	if(byte_ost > mm.bitmaps[dep].len || byte_ost < 0) {
		debug("Invalid Parameter(byte_ost)#%d\n", byte_ost);
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
		debug("Invalid Parameter#%d\n", dep);
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

static bool mm_is_used(u32 dep, u32 ost)
{
	u32 *blk = (mm.bitmaps[dep].block) + (ost / mm.bitmap_size);

	return (*blk & (0x01 << (ost % mm.bitmap_size)));
}

static int mm_set_bit(u32 dep, u32 ost, u32 f)
{
	u32 *blk;
	u8 ret;
	if(ost < 0) {	
		debug("Invalid Parameter(ost)#%d\n", ost);
		return -EINVAL;
	}

	if(f != MM_BUD_FREE && f != MM_BUD_USED) {	
		debug("Invalid Parameter(f)#%d\n", f);
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

static int mm_chk_parent(u32 dep, u32 ost)
{
	u32 i, n;
	if(dep < 0) {
		debug("Invalid Parameter#%d\n", dep);
		return -EINVAL;
	}

	if(dep == -1) 
		return 0;

	for(i = dep, n = ost/2; i < mm.dep ; i++, n = n/2) {
		if(mm_is_used(i, n))
			return 0;

		mm_set_bit(i, n , MM_BUD_USED);
	}
	
	return 0;
}

static int mm_chk_childs(u32 dep, u32 ost)
{
	int i, j, n, k = 1;

	if(dep < 0) {
		debug("Invalid Parameter#%d\n", dep);
		return -EINVAL;
	}

	if(dep == -1) 
		return 0;

	n = ost;
	for(i = dep ; i>-1 ; i--) {
		n *= 2;
		k *= 2;
		for(j = n; j < n+k; j++) {
			mm_set_bit(i, j, MM_BUD_USED);
		}
	}
}

static int mm_find_free(dep)
{
	int ost, byte_ost, bit_ost;

	if(dep < 0) {
		debug("Invalid Parameter#%d\n", dep);
		return -EINVAL;
	}

	byte_ost = mm_find_free_byte(dep);
	if(byte_ost < 0)
		return byte_ost;

	bit_ost = mm_find_free_bit(dep, byte_ost);
	if(bit_ost < 0)
		return bit_ost;

	ost = (byte_ost * mm.bitmap_size) + bit_ost;
	mm_chk_childs(dep-1, ost);	
	mm_chk_parent(dep+1, ost);

	return ost;	
}

static void *mm_encode(u32 chunk, u32 ost, u8 dep)
{
	int err;
	void *addr;
	u32 hdr_chunk = chunk + sizeof(struct mm_blk_hdr);
	struct mm_blk_hdr* hdr_base = (struct mm_blk_hdr*)(mm.heap_base + (hdr_chunk*ost));
	
	hdr_base->dep = dep;

	hdr_base += 1;
	if(((u32)(hdr_base) + chunk) > mm.heap_base + mm.heap_size) {
		debug("Out of memory#%d\n", (u32)(hdr_base) + chunk);
		return NULL; 
	}

	return (void *)hdr_base;
}

void* mm_alloc(u32 size)
{
	u32 chunk, dep, ost;
	void *addr;

	if(size < 1) {
		debug("Invaid Parameter#%d\n", size);
		return size;
	}

	chunk = mm_rndup(size);
	if(chunk < mm.ll)
		return chunk;	

	dep = mm_get_ord(chunk);
	if(dep < 0) 
		return dep;

	ost = mm_find_free(dep);
	if(ost < 0)
		return ost;
	
	addr = mm_encode(chunk, ost, dep);
	if(!addr)
		return -ENOMEM; 
	
	return addr;
}

void mm_free(void *addr)
{
	struct mm_blk_hdr *hdr = (struct mm_blk *)((u8 *)addr - sizeof(struct mm_blk_hdr));		

/*	
	ost = decode();	

	set_bit(ost);

	ret = compare_buddy();
	ok = check_merge(ret);
	if(ok)
		merge();
	else
		finish();
	mm_chk_childs(dep-1, ost);	
	mm_chk_parent(dep+1, ost);
*/

	debug("addr#0x%x, dep#0x%x\n", hdr, hdr->dep);
	debug("addr#0x%x, dep#0x%x\n", (u32)hdr - mm.heap_base, hdr->dep);
}


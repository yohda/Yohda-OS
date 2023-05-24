#include "type.h"
#include "pool.h"
#include "list.h"
#include "mm.h"
#include "mmi.h"
#include "debug.h"
#include "bitmap.h"
#include "error.h"
#include "math.h"

#define POOL_HEADER_SIZE 		(sizeof(struct pool_header))	

static int pl_init(const void *base, const s64 size);
static void *pl_alloc(const int size);
static void pl_free(const void *addr);
static void pl_show_info(void);
static int pl_get_chunk(const int size);

struct mmif sm_if = {
	.mm_init = pl_init,	
	.mm_free = pl_free,
	.mm_alloc = pl_alloc,
	.show_info = pl_show_info,
	.get_chunk = pl_get_chunk,
};

struct mm_pool {
	u8 *base;   // Memory pool base address
	s64 size;	// Available allocation size of memory pool
	u8 *data_base;
	int llc;
	int ulc;
	u32 rmd;
	s64 meta_size;
	s64 data_size;
	
	// memory pool number
	int num;
};

static struct mm_pool pmm;
static struct pool *pools;

static void pl_show_info(void)
{
	int i;
	for(i=0 ; i<pmm.num ; i++) {
		if(!pools[i].size) {
			pl_debug("Any memory is not allocated to pools[%d]\n", i);
			continue;	
		}

		pl_debug("[%d] base address#0x%x\n", i, pools[i].base);
		pl_debug("chunk#%d\n", pools[i].chunk);
		pl_debug("size#%d\n", pools[i].size);
		pl_debug("frees#%d\n", pools[i].frees);
	}
}

static int pool_calc_size(const s64 size)
{
	int i;
	s64 rmd = 0;

	if(size<1)
		return err_dbg(-1, "err\n");

	rmd = size;
	for(i=0 ; rmd>=pools[0].ll ; i++) {
		if((rmd-pools[i%pmm.num].ll) >= 0) {
			pools[i%pmm.num].size += pools[i%pmm.num].ll;
			rmd -= pools[i%pmm.num].ll;	
		}
	}

	return rmd;
}

static int pool_get_ord(u32 chunk)
{
	int size = pmm.llc, i;

	if(chunk<size || chunk>pmm.ulc)
		return err_dbg(-EINVAL, "Invalid Parameter#%d\n", chunk);

	for(i=0 ; i<pmm.num ; i++) {
		if(chunk == size)
			return i;

		size *= 2;
	}	

	return 0;
}

static int pool_get_id(u32 addr)
{
	int i;

	for(i=0 ; i<pmm.num ; i++) {
		u32 diff = addr - (u32)pools[i].base;	
		if(diff < pools[i].size)
			return i;
	} 	

	return -1;
}

static int pl_get_chunk(const int size)
{
	u32 block = pools[0].chunk, i = 0;

	if(size < 1)
		return err_dbg(-EINVAL, "Invalid parameter#%d\n", size);

	if(size > pmm.ulc)
		return 0;

	for(i = 0 ; i < pmm.num; i++) {
		if(size <= block) {
			return block;
		}
		block *= 2;
	}

	return -1;
}

static void *pl_alloc(const int size)
{
	int chunk = 0 , handle = 0, id = -1, next = 0;
	void *addr = NULL;
	
	if(size < 1)
		return err_dbg(-1, "Invalid Parameter#%d\n", size);
	
	chunk = pl_get_chunk(size);
	if(chunk < 0)
		return err_dbg(chunk, "err\n");

	id = pool_get_ord(chunk);
	if(id < 0)
		return err_dbg(id, "err\n");

	addr = pools[id].head + POOL_HEADER_SIZE;
	next = *((u32 *)pools[id].head);
	if(!next)
		pools[id].head += pools[id].chunk + POOL_HEADER_SIZE;
	else
		pools[id].head = (u8 *)(next);	

	(pools[id].frees)--;

	return addr;
}

static void pl_free(const void *addr)
{
	struct pool_header *hdr = NULL;
	int id = -1;

	if(!addr)
		return err_dbg(-1, "err\n");		

	hdr = (struct pool_header *)(addr - POOL_HEADER_SIZE);
	if(!hdr)
		err_dbg(-4, "err\n");

	id = pool_get_id(hdr);
	if(id < 0)
		return err_dbg(-4, "err\n");		
	
	hdr->next_addr = pools[id].head;
	pools[id].head = hdr;

	(pools[id].frees)++;
}

static int pl_get_data_size(const s64 size)
{
	s64 data_size = 0;
	int i;

	if(size < 1)
		return err_dbg(-1, "err\n");	

	if(pmm.num < 1)
		return err_dbg(-4, "err\n");		

	data_size = size - sizeof(struct pool)*pmm.num;

	return data_size;
}

s64 pl_get_init_size(void)
{
	int i = 0, llc = 0, ulc = 0, num = 0;
	s64 size = 0;
		
	llc = mm_get_sec_llc();
	ulc = mm_get_sec_ulc();
	num = (log(2, ulc)-log(2, llc))+1;
	
	size = sizeof(struct pool)*num;
	for(i=0 ; i<num ; i++) {
		size += (POOL_HEADER_SIZE+llc) * POOL_CHUNK_LL_NUM;
		llc *= 2;	
	}

	return size; 
}

static int pl_init(const void *base, const s64 size)
{
	int i = 0, rmd = 0, err = -1, llc = 0;
	struct pool *pool = NULL;
	u8 *data_addr = NULL;
	s64 data_size = 0;

	if(!base)
		return err_dbg(-1, "A base address of memory pool is null.\n");

	if(size < 1)
		return err_dbg(-4, "The size allcating a memory pool is too small#%d\n", size);

	pmm.base = (u8 *)base;
	pmm.size = size;
	pmm.llc = mm_get_sec_llc();
	pmm.ulc = mm_get_sec_ulc();
	pmm.num = (log(2, pmm.ulc)-log(2, pmm.llc))+1;

	mm_set_sec_base(base);
	mm_set_sec_size(size);

	data_size = pl_get_data_size(size);
	if(data_size < 0)
		return err_dbg(-6, "err\n");
			
	pmm.size = size;
	pmm.data_size = data_size;

	llc = pmm.llc;
	pools = (struct pool *)base;
	memset(pools, 0, sizeof(struct pool)*pmm.num);	
	for(i=0 ; i<pmm.num ; i++) {
		pools[i].chunk = llc;
		pools[i].ll = (POOL_HEADER_SIZE+llc) * POOL_CHUNK_LL_NUM;

		llc *= 2;	
	}
	
	rmd = pool_calc_size(data_size);
	if(rmd < 0)
		err_dbg(-8, "err\n");

	if(rmd < 0)
		err_dbg(-12, "err\n");

	data_addr = base + sizeof(struct pool)*pmm.num;
	for(i=0 ; i<pmm.num ; i++) {
		pools[i].base = (u8 *)data_addr;
		pools[i].head = pools[i].base;
		pools[i].frees = pools[i].size / (pools[i].chunk + POOL_HEADER_SIZE);
		data_addr += pools[i].size;			

		memset(pools[i].base, 0, pools[i].size);
	}

	return rmd;
}

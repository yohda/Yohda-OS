#include "type.h"
#include "pool.h"
#include "list.h"
#include "mm.h"
#include "debug.h"
#include "bitmap.h"
#include "error.h"

#define POOL_HEADER_SIZE 		(sizeof(struct pool_header))	

static struct pool pools[] = {
	{
		.chunk = POOL_HEADER_SIZE + 16,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 16),
	},
	{
		.chunk = POOL_HEADER_SIZE + 32,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 32), 
	},
	{
		.chunk = POOL_HEADER_SIZE + 64,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 64), 
	},
	{
		.chunk = POOL_HEADER_SIZE + 128,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 128), 
	},
	{
		.chunk = POOL_HEADER_SIZE + 256,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 256), 
	},
	{
		.chunk = POOL_HEADER_SIZE + 512,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 512), 
	},
	{
		.chunk = POOL_HEADER_SIZE + 1024,
		.ll = POOL_HEADER_SIZE + (POOL_CHUNK_LL_NUM * 1024), 
	},
};

#define POOL_NUMBER 			(sizeof(pools)/sizeof(struct pool))

struct mm_pool {
	u8 *base;   // Memory pool base address
	int size;	// Available allocation size of memory pool
	u8 *data_base;
	int llc;
	int ulc;
	u32 rmd;
};

struct mm_pool pmm;

static int pool_calc_size(const int size)
{
	int i, rmd = 0;
	
	if(size<1)
		return err_dbg(-1, "err\n");

	rmd = size;
	for(i=0 ; rmd>=pools[0].ll ; i++) {
		if((rmd-pools[i%POOL_NUMBER].ll) >= 0) {
			pools[i%POOL_NUMBER].size += pools[i%POOL_NUMBER].ll;
			rmd -= pools[i%POOL_NUMBER].ll;	
		}
	}
	
	pl_debug("rmd#%d\n", rmd);

	return rmd;
}

static int pool_get_ord(u32 chunk)
{
	int size = pmm.llc, i;

	if(chunk<size || chunk>pmm.ulc)
		err_dbg(-EINVAL, "Invalid Parameter#%d\n", chunk);

	for(i=0 ; i<POOL_NUMBER ; i++) {
		if(chunk == size)
			return i;

		size *= 2;
	}	

	return 0;
}

static int pool_get_id(u32 addr)
{
	int i;

	for(i=0 ; i<POOL_NUMBER ; i++) {
		int diff = addr - (u32)pools[i].base;	
		if(diff < pools[i].size)
			return i;
	} 	

	return -1;
}

static int pool_get_chunk(int size)
{
	u32 block = pools[0].ll, i = 0;

	if(size<1 || size>pmm.ulc)
		return err_dbg(-EINVAL, "Invalid parameter#%d\n", size);
	
	for(i = 0 ; i < POOL_NUMBER; i++) {
		if(size <= block) {
			return block;
		}
		block *= 2;
	}

	return -1;
}

void *pool_alloc(int size)
{
	int chunk = 0 , handle = 0, id = -1;
	void *addr = NULL;

	if(size < 1)
		return err_dbg(-1, "Invalid Parameter#%d\n", size);
	
	chunk = pool_get_chunk(size);
	if(chunk < 0)
		return err_dbg(chunk, "err\n");

	id = pool_get_ord(chunk);
	if(id < 0)
		return err_dbg(id, "err\n");

	addr = pools[id].head + POOL_HEADER_SIZE;
	if(pools[id].head == 0x00)
		pools[id].head += pools[id].chunk;
	else
		pools[id].head = (u8 *)(*((u32 *)pools[id].head));	

	return addr;
}

void pool_free(void	*addr)
{
	struct pool_header *hdr = NULL;
	int id = -1;

	if(!addr)
		return err_dbg(-1, "err\n");		

	hdr = (struct pool_header *)(addr - POOL_HEADER_SIZE);
	if(!hdr)
		err_dbg(-4, "err\n");

	id = pool_get_id(addr);
	if(id < 1)
		return err_dbg(-4, "err\n");		
	
	hdr->next_addr = pools[id].head;
	pools[id].head = hdr;
}

int pool_init(const int base, const int size)
{
	int i = 0, addr = 0, rmd = 0;
	struct pool *pool = NULL;

	if((base<0) || (size<1))
		return err_dbg(-1, "err\n");

	pl_debug("pool base#0x%x\n", base);

	addr = base;
	pmm.base = (u8 *)base;
	pmm.size = size;
	pmm.llc = pools[0].ll; 
	pmm.ulc = pools[POOL_NUMBER-1].ll;

	rmd = pool_calc_size(size);
	if(rmd < 0)
		err_dbg(-4, "err\n");
	
	pl_debug("rmd#%d\n", rmd);

	if(rmd < 0)
		err_dbg(-4, "err\n");

	for(i=0 ; i<POOL_NUMBER ; i++) {
		pools[i].base = (u8 *)addr;
		addr += pools[i].size;			

		pl_debug("base#0x%x\n", pools[i].base);
	}

	return 0;
}

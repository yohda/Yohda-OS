#ifndef _POOL_H_
#define _POOL_H_

#include "test.h"

#define POOL_CHUNK_LL_NUM		(128)

struct pool_header {
	u8 *next_addr;
};

struct pool {
	u8 *base, *head; 		// base address, head poiner
	u32 size;				// for limit and protection. don`t allow for cp to cross beyond the permited size 
	u32 frees;
	u32 chunk;				// each pool chunk size including header(next address)
	int ll;					// low limit size of pool
};

#endif

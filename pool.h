#ifndef _POOL_H_
#define _POOL_H_

#define POOL_CHUNK_LL_NUM		(128)

#define POOL_USED				(BIT_SET)
#define POOL_FREE				(BIT_CLR)

struct pool_header {
	u8 *next_addr;
};

struct pool {
	u8 *base, *head; 		// base pointer, current point
	u32 size;			// for limit and protection. don`t allow for cp to cross beyond the permited size 
	u32 frees;
	u32 chunk;
	int ll;
};

#endif

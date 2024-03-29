#include "bitmap.h"
#include "mm/mmi.h"
#include "debug.h"
#include "error.h"
#include "string.h"

#define BYTE_UNIT 		8

// On 2023.5.12, it support only by 32-bit. later, i plan to support by 64-bit.
const u32 bit_cmp[64] = { 
	0x00000000, 									// 0
	0x00000001, 0x00000003, 0x00000007, 0x0000000F, // 1 ~ 4
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF, // 5 ~ 8
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, // 9 ~ 12
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF, // 13 ~ 15
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, // 16 ~ 19
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF, // 20 ~ 23
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, // 24 ~ 27
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, // 28 ~ 31
};
/*
 * bmp - `struct bitmap` address
 * bit - what bits you handle i.e) 32-bit, 24-bit, 16-bit ...
 * size - bitmap number
 * */
int bitmap_alloc(struct bitmap *bmp, const u32 bit, const u32 size)
{
	if(!bmp)
		return -1;

	if(bit < 1 || size < 1)
		return -1;

	bmp->base = yalloc((bit/BYTE_UNIT)*size);
	if(!bmp->base)
		return -1;

	memset(bmp->base, 0, sizeof(bit*size));
	bmp->bit = bit;
	bmp->size = size;

	return 0;
}

int bitmap_set(struct bitmap *bmp, u32 ost, bool f)
{
	int i = 0, n = 0, bit = 0;
	u32 *bits;

	if(!bmp || !bmp->base) 
		return err_dbg(-1, "err\n");

	bits = (u32 *)bmp->base;
	bit = bmp->bit;

	if(f) {
		*(bits+(ost/bit)) |= 0x1<<(ost%bit);
	} else {
		*(bits+(ost/bit)) &= ~(0x1<<(ost%bit));		
	}

	return ost;
}

int bitmap_get_free(struct bitmap *bmp)
{
	int i = 0, j = 0, n = 0, bit = 0;
	u32 *bits;

	if(!bmp || !bmp->base) 
		return err_dbg(-1, "err\n");

	bits = (u32 *)bmp->base;
	bit = bmp->bit;
	n = bmp->size;	
	
	for(i=0 ; i<n ; i++) {
		if(*(bits+i) != bit_cmp[bit])
			break;
	}

	if(i == n)
		return err_dbg(-4, "err\n");

	bits = bits+i;
	for(j=0 ; j<bit ; j++) {
		if(!(*bits>>j & 0x1))
			break;
	}

	return i*bit+j;
}

int bitmap_get(struct bitmap *bmp, int ost)
{
	int bit = 0;
	u32 *bits;

	if(!bmp || !bmp->base) 
		return err_dbg(-1, "err\n");

	bits = (u32 *)bmp->base;
	bit = bmp->bit;
	
	if(ost < 0)
		return err_dbg(-EINVAL, "err\n");

	return (*(bits+(ost/bit)) & (0x1<<(ost%bit))) ? 1 : 0;
}

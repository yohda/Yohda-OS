#include "gdt.h"
#include "error.h"
#include "debug.h"

#define GDT_TYPE_CODE_READABLE		((0x01)<<9)
#define GDT_TYPE_CODE_PRIV_ALLOW 	((0x01)<<10)
#define GDT_TYPE_DATA_WRITEABLE		((0x01)<<9)
#define GDT_TYPE_DATA_GROW_DOWN		((0x01)<<10)
#define GDT_TYPE_EXE_DATA			((0x00)<<11)
#define GDT_TYPE_EXE_CODE			((0x01)<<11)
#define GDT_S_SYS					((0x00)<<12)
#define GDT_S_CD					((0x01)<<12)
#define GDT_DPL_KERN 				((0x00)<<13)
#define GDT_DPL_USER 				((0x03)<<13)
#define GDT_PRE_VALID	 			((0x01)<<15)

// FLAGS
#define GDT_32BIT					((0x00)<<21)
#define GDT_64BIT					((0x01)<<21)
#define GDT_DB_16					((0x00)<<22)
#define GDT_DB_32					((0x01)<<22)
#define GDT_G_BYTE					((0x00)<<23)
#define GDT_G_PAGE					((0x01)<<23)

#define GDT_BASE_ADDR 				(0x200000)

#define GDT_64CODE_KERN_READABLE (GDT_TYPE_CODE_READABLE|GDT_TYPE_CODE_PRIV_ALLOW|GDT_TYPE_EXE_CODE|GDT_S_CD|GDT_DPL_KERN|GDT_PRE_VALID|GDT_64BIT|GDT_G_PAGE)
#define GDT_32CODE_KERN_READABLE (GDT_TYPE_CODE_READABLE|GDT_TYPE_CODE_PRIV_ALLOW|GDT_TYPE_EXE_CODE|GDT_S_CD|GDT_DPL_KERN|GDT_PRE_VALID|GDT_32BIT|GDT_G_PAGE)


#define GDT_64DATA_KERN_WRITEABLE (GDT_TYPE_DATA_WRITEABLE|GDT_TYPE_EXE_DATA|GDT_S_CD|GDT_DPL_KERN|GDT_PRE_VALID|GDT_64BIT|GDT_G_PAGE)
#define GDT_32DATA_KERN_WRITEABLE (GDT_TYPE_DATA_WRITEABLE|GDT_TYPE_EXE_DATA|GDT_S_CD|GDT_DPL_KERN|GDT_PRE_VALID|GDT_32BIT|GDT_G_PAGE)

#define GDT_MAX_NUM 		(8192)
#define GDT_MAX_LIMIT	 	(0x000FFFFF)

struct gdt_mgr {
	void *bp, *cp;
	int num;
};

struct gdt_mgr gmt;

static void *gdt_crt32(const u32 base, const u32 limit, const u16 flag)
{
	u32 *desc = gmt.cp;

	if(gmt.num > GDT_MAX_NUM)
		return err_dbg(NULL, "GDT SIZE EXCEED!!\n");

	// Low-half in 64-bit
	*desc = (limit|0x0000FFFF);
	*desc |= ((base|0x0000FFFF)<<16);

	// High-half in 64-bit
	desc += 1;

	*desc = ((base|0x00FF0000)>>16);
	*desc |= flag;
	*desc |= (limit|0x000F0000);
	*desc |= (base|0xFF000000);

	desc += 1;

	gmt.cp = desc;
	gmt.num++;

	return desc;
}

static const void *gdt_crt64(const u64 base, const u32 limit, const u16 flag)
{
	if(limit < 1 || limit > GDT_MAX_LIMIT) 
		return err_dbg(NULL, "Invalid Parameter(limit)#0x%x\n", limit);

	if(base < 0)
		return err_dbg(NULL, "Invalid Parameter(base)#0x%x\n", base);

	u64 *desc = gdt_crt32(base, limit, flag);
	if(!desc)
		return err_dbg(NULL, "err\n");		

	*desc |= base>>32;

	gmt.cp = desc;

	return desc;
}

int gdt_crt_kern64_code(const u64 base, const u32 limit)
{
	if(!gdt_crt64(base, limit, GDT_64CODE_KERN_READABLE))
		return err_dbg(-1, "err\n");

	return 0;
}

int get_crt_kern64_data(const u64 base, const u32 limit)
{
	if(!gdt_crt64(base, limit, GDT_64DATA_KERN_WRITEABLE))
		return err_dbg(-1, "err\n");

	return 0;
}

int gdt_crt_user64_code(const u64 base, const u32 limit)
{
	// To-do
}

int gdt_crt_user64_data(const u64 base, const u32 limit)
{
	// To-do
}

int gdt_crt_kern32_code(const u32 base, const u32 limit)
{
	gdt_crt32(base, limit, GDT_32CODE_KERN_READABLE);
}

int get_crt_kern32_data(const u32 base, const u32 limit)
{
	gdt_crt32(base, limit, GDT_32DATA_KERN_WRITEABLE);
}

int gdt_crt_user32_code(const u32 base, const u32 limit)
{
	// To-do
}

int gdt_crt_user32_data(const u32 base, const u32 limit)
{
	// To-do
}

void gdt_init()
{
	gmt.bp = GDT_BASE_ADDR;
	gmt.cp = gmt.bp;
}

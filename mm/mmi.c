#include "type.h"
#include "mm/mmi.h"
#include "error.h"
#include "debug.h"
#include "mm/mm.h"

extern struct mmif pm_if;
extern struct mmif sm_if;

void mmi_show_info(void)
{
	//if(!pm_if.show_info || !sm_if.show_info)
	//	return err_dbg(-1, "err\n");	

	if(!pm_if.show_info)
		return err_dbg(-1, "err\n");

	pm_if.show_info();

	if(!sm_if.show_info)
		return err_dbg(-4, "Secondary memory management is absence.\n");	

	sm_if.show_info();

}	

// yohdaOS allocator
void *yalloc(const s64 size)
{
	void *addr = NULL;
	int err = -1, type = -1;

	if(size < 1)
		return err_dbg(-1, "err\n");

	if(size >= mm_get_pri_llc()) {
		addr = pm_if.mm_alloc(size);
	} else {
		addr = sm_if.mm_alloc(size);
	} 

	if(!addr)
		return err_dbg(-8, "err\n");
	
	return addr;	
}

void yfree(void *addr)
{
	if(!addr)
		return err_dbg(-1, "err\n");
	
	switch(mm_where_is_addr(addr)) {
		case (MM_PRI) :
			pm_if.mm_free(addr);
		break;
		case (MM_SEC) :
			sm_if.mm_free(addr);
		break;
		default :
			return err_dbg(-4, "err\n");
		break;
	}
}

int mmi_init(const void *base, const s64 size)
{
	s64 pri_size = 0, sec_size = 0, pri_rmd = 0, rmd = 0;
	void *pl_base = NULL;

	if(!pm_if.mm_init || !pm_if.mm_alloc || !pm_if.mm_free) 
		return err_dbg(-1, "Primary memory management is not exist\n");

	if(!sm_if.mm_init || !sm_if.mm_alloc || !sm_if.mm_free)
		return err_dbg(-4, "Primary memory management is not exist\n");

	if(!base)
		return err_dbg(-5, "err\n");

	if(size < 1)
		return err_dbg(-6, "err\n");

	rmd = pm_if.mm_init(base, size);
	if(rmd < 0)
		return err_dbg(-8, "Failed to initialize the primary memory management\n");

	sec_size = pl_get_init_size();
	if(sec_size < 0)
		return err_dbg(-9, "err\n");
	
	pl_base = pm_if.mm_alloc(sec_size);
	if(!pl_base)
		return err_dbg(-10, "err\n");

	sec_size = pm_if.get_chunk(sec_size);
	if(sec_size < 0)
		return err_dbg(-14, "err\n");

	mm_debug("base#0x%x, pl_base#0x%x, rmd#0x%x\n", base, pl_base, rmd);
	rmd += sm_if.mm_init(pl_base, sec_size);
	if(rmd < 0)
		return err_dbg(-18, "Failed to initialize the secondary memory management\n");

	return rmd;
}

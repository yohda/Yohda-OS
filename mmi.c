#include "type.h"
#include "mmi.h"
#include "errno.h"
#include "debug.h"
#include "mm.h"

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
void *yalloc(const int size)
{
	void *addr = NULL;
	int err = -1;

	if(!pm_if.mm_alloc || !sm_if.mm_alloc)
		return err_dbg(-1, "err\n");
	
	//addr = sm_if.mm_alloc(size);
	//if(!addr)
	//	err_dbg(-4, "err\n");

	addr = pm_if.mm_alloc(size);
	if(!addr)
		return err_dbg(-8, "err\n");
	
	return addr;	
}

void yfree(void *addr)
{
	if(!pm_if.mm_free)
		return err_dbg(-1, "err\n");

	pm_if.mm_free(addr);
}

int mmi_init(const void *base, const int size)
{
	int err = -1;

	if(!pm_if.mm_init) 
		return err_dbg(-1, "Primary memory management is not exist\n");
	
	err = pm_if.mm_init(base, size);
	if(err < 0)
		return err_dbg(-4, "errn\n");

	return 0;
}

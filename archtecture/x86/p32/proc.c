#include "proc.h"
#include "mm/mmi.h"
#include "debug.h"
#include "sched.h"

struct proc_mgmt {
	struct proc_ctrl_blk proc_table[PROC_MAX];
	uint8_t *stack;
	uint32_t pid;
	uint32_t used;
	bool inited;
};

struct proc_mgmt proc_mgr;

void *proc_create(const void *entry_point)
{
	struct proc_ctrl_blk *proc = NULL;
	int i;

	if(!proc_mgr.inited) {
		debug("Check if process initailization\n");
		return NULL;
	}

	for(i=0; i<PROC_MAX; i++) {
		if(!proc_mgr.proc_table[i].state)
			break;	
	}

	if(i >= PROC_MAX)
		return NULL;

	proc = &proc_mgr.proc_table[i];
	proc->state = PROC_READY; 
	proc->stack = proc_mgr.stack + (i*PROC_STACK_SIZE);
	proc->id = proc_mgr.pid++;

	proc->ic = (proc->stack + PROC_STACK_SIZE) - sizeof(struct intr_context);
	//proc->ic->entry = switch_kern_to_user; 
		
	proc->pc = (uint32_t)(proc->ic) - sizeof(struct proc_context);
	memset(proc->pc, 0, sizeof(struct proc_context));

	proc->pc->eip = entry_point ? entry_point : switch_kern_to_user;

	sched_add_ready(proc);	

	return proc;
}

void *proc_create_user(void)
{
	struct proc_ctrl_blk* init;

	
}

void *proc_exit(struct proc_ctrl_blk *proc)
{
	if(!proc)
		return NULL;		

	proc->state = PROC_FREE;
}

int proc_init()
{
	memset(proc_mgr, 0, sizeof(proc_mgr));
	proc_mgr.stack = yalloc(PROC_MAX * PROC_STACK_SIZE); 
	if(!proc_mgr.stack)
		err_dbg(-1, "Failed to allocate the kernel stack size#", PROC_MAX * PROC_STACK_SIZE);

	proc_mgr.inited = 1;
}

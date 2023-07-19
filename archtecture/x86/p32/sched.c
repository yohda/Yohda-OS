#include "sched.h"
#include "list.h"
#include "proc.h"
#include "mm/mmi.h"
#include "debug.h"

struct scheduler {
	bool inited;
	struct list_node *ready_list;
	uint32_t readys;
	struct list_node *block_list;	
	uint32_t blocks;
	struct proc_context *context;	// scheduler context
	struct proc_ctrl_blk *proc;		// current process running 
};

struct scheduler scheduler;

static void _sched(void)
{
	struct list_node *node = NULL;
	struct proc_ctrl_blk *new = NULL;	

	if(!scheduler.inited)
		return -1;
	
	while(1) {
		if(!scheduler.readys)
			continue;

		node = list_get(scheduler.ready_list);
		if(!node) // ready list is empty
			return 0; 	

		new = container_of(node, struct proc_ctrl_blk, node); 
		if(!new) // ready list is empty
			return 0; 	
	
		scheduler.readys--;
		new->state = PROC_RUN;
		scheduler.proc = new;

		context_switch(&(scheduler.context), new->pc);
	}
		
	// Never come here	
}

void sched(void)
{
	_sched();
}

struct proc_ctrl_blk *sched_get_proc(void)
{
	return scheduler.proc;
}

static void _sched_add_ready(struct proc_ctrl_blk *proc)
{
	list_add(scheduler.ready_list, &proc->node);
	scheduler.readys++;
}

void sched_exit(void)
{
	struct proc_ctrl_blk *proc = scheduler.proc;
	
	if(!scheduler.inited)
		return -1;
	
	if(!proc)
		return NULL;

	proc->state = PROC_TERM;	
	
	_sched_add_ready(proc);
}

void sched_timer(void)
{
	struct proc_ctrl_blk *proc = scheduler.proc;
	
	if(!scheduler.inited)
		return err_dbg(-1, "ERR\n");
	
	if(!proc)
		return err_dbg(NULL, "ERR\n");

	proc->state = PROC_READY;	
	
	_sched_add_ready(proc);
}

// But, it does not mean it killed myself.
void sched_yield(void)
{
	struct proc_ctrl_blk *curr_proc = scheduler.proc;
	
	if(!scheduler.inited)
		return err_dbg(-1, "ERR\n");
	
	if(!curr_proc)
		return err_dbg(NULL, "ERR\n");

	curr_proc->state = PROC_READY;	
	_sched_add_ready(curr_proc);
	
	context_switch(&(curr_proc->pc), scheduler.context);	
}

// Wait for a event about I/O
void sched_sleep(void)
{
	struct proc_ctrl_blk *proc = scheduler.proc;

	if(!scheduler.inited)
		return -1;
	
	if(!proc)
		return NULL;

	proc->state = PROC_BLOCK;
		
	sched_add_block(proc);	
}

void sched_add_block(struct proc_ctrl_blk *proc)
{
	if(!scheduler.inited)
		return -1;

	if(!proc)
		return NULL;

	list_add(scheduler.block_list, &proc->node);
}

void sched_add_ready(struct proc_ctrl_blk *proc)
{
	if(!scheduler.inited)
		return -1;
	
	if(!proc)
		return NULL;

	_sched_add_ready(proc);
}

void sched_del_ready(const struct proc_ctrl_blk *proc)
{
	if(!scheduler.inited)
		return -1;
	
	if(!proc)
		return NULL;

	if(proc->state != PROC_READY)	
		return NULL;

	list_del(scheduler.ready_list, &proc->node);
	scheduler.readys--;
}

int sched_init()
{	
	/* Scheduler Initialization  */
	memset(scheduler, 0, sizeof(scheduler));
	scheduler.ready_list = yalloc(sizeof(struct list_node));
	if(!scheduler.ready_list)	
		return err_dbg(-1, "err\n");	

	scheduler.block_list = yalloc(sizeof(struct list_node));
	if(!scheduler.block_list)	
		return err_dbg(-2, "err\n");	

	list_init_head(scheduler.ready_list);
	list_init_head(scheduler.block_list);
	
	scheduler.readys = 0;
	scheduler.blocks = 0;
	
	scheduler.inited = true;

	/* Kernel Init Process is scheduler */	
}

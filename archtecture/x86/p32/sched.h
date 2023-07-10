#ifndef _SCHED_H_ 
#define _SCHED_H_

#include "type.h"
#include "proc.h"

extern void context_switch(struct proc_context **old, struct proc_context *new);

void sched(void);
int sched_init(void);
void sched_timer(void);
void sched_yield(void);
void sched_sleep(void);
void sched_add_block(struct proc_ctrl_blk *proc);
void sched_add_ready(struct proc_ctrl_blk *proc);
struct proc_ctrl_blk *sched_get_proc(void);

#endif

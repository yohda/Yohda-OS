#ifndef _SCHED_H_ 
#define _SCHED_H_

#include "type.h"
#include "proc.h"

void sched(void);
int sched_init(struct proc_ctrl_blk *dummy);
void sched_timer(void);
void sched_yield(void);
void sched_sleep(void);
void sched_add_block(struct proc_ctrl_blk *proc);
void sched_add_ready(struct proc_ctrl_blk *proc);

#endif

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "type.h"
#include "list.h"

#define PROC_MAX 128
#define PROC_STACK_SIZE 2048

enum PROC_STATE {
	PROC_FREE,
	PROC_READY,
	PROC_RUN,
	PROC_BLOCK,
	PROC_TERM,
	RPOC_INVAL
};

// This structure must consist of ebp, ebx, esi, edi, eip in the order.
struct proc_context {
	uint32_t ebp;
   	uint32_t ebx;
	uint32_t esi;
	uint32_t edi;
	uint32_t eip;
};

struct proc_ctrl_blk {
	uint32_t id;
	uint8_t state;
	uint8_t *stack;
	struct proc_context *context;
	struct list_node node;
};

void *proc_create(const void *entry_point);
void *proc_exit(struct proc_ctrl_blk *proc);
int proc_init();

#endif

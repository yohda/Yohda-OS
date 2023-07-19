#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "type.h"
#include "list.h"

#define PROC_MAX 128
#define PROC_STACK_SIZE 2048

extern void switch_kern_to_user(void);
extern void switch_user_to_kern(void);

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

struct intr_context {
	// switch point from kernel to user
	void *entry;

	// the order is `puhsd` instruction
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t rsvd; // pusha esp
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	// segment
	uint32_t ds;
	uint32_t es;
	uint32_t fs;
	uint32_t gs;

	// Automatically, inserted data by interrupt handler
	uint32_t vec;
	uint32_t err;

	// Automatically, inserted data by processor 
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
   	uint32_t ss;	
}__attribute__((packed));

struct proc_ctrl_blk {
	uint32_t id;
	uint8_t state;
	uint8_t *stack;
	struct intr_context *ic;
	struct proc_context *pc;
	struct list_node node;
};

void *proc_create(const void *entry_point);
void *proc_exit(struct proc_ctrl_blk *proc);
int proc_init();

#endif

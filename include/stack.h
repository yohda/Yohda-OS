#ifndef _STACK_H_
#define _STACK_H_

#include "type.h"

#define STK_EMPTY -1

struct stack {
	u32 top;
	void **stk;
};

void *stack_init(struct stack *ptr, const int size);
void *stack_pop(struct stack *ptr);
void *stack_push(struct stack *ptr, void *data);
void *stack_peek(struct stack *ptr);


#endif

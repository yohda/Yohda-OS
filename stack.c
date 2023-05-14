#include "stack.h"
#include "mm.h"

// This function of second argument is unit of byte.
void *stack_init(struct stack *ptr, const int size)
{
	if(!ptr || size<1)
		return NULL;

	ptr->stk = mm_alloc(size, MM_KL);
	if(!ptr->stk)
		return NULL;

	ptr->top = STK_EMPTY;

	return ptr;
}

void *stack_pop(struct stack *ptr)
{
	if(!ptr || !ptr->stk)
		return NULL;
	
	if(ptr->top == STK_EMPTY)
		return NULL;

	return (ptr->stk)[(ptr->top)--];	
}

void *stack_push(struct stack *ptr, void *data)
{
	if(!ptr || !ptr->stk)
		return NULL;

	if(!data)
		return NULL;	

	(ptr->stk)[++ptr->top] = data; 	
	
	return data;
}

void *stack_peek(struct stack *ptr)
{
	if(!ptr || !ptr->stk)
		return NULL;
	
	if(ptr->top == 0)
		return NULL;

	return (ptr->stk)[ptr->top];
}

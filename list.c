#include "list.h"
#include "Types.h"
#include "errno.h"

// Determining the prvious point of head in linked list is depending on each case. 
// My opnion is that a head node points yourself. But, tail of head node does NULL.
// NULL means the end of list.
void list_init_head(struct list_head *head)
{
	head->prev = head;
	head->next = NULL;
}

void list_add_head(struct list_head *p, struct list_head *new)
{
	if(p == NULL || new == NULL)
		return -EINVAL;

	new->next = p;
	new->prev = p->prev;

	new = new->prev->next;
	p->prev = new;

	return 0;
}

void list_add(struct list_head *p, struct list_head *new)
{
	if(p == NULL || new == NULL)
		return -EINVAL;

	new->next = p->next;	
	new->prev = p;

	p->next = new;
	new = new->next->prev;

	return 0;
}

void list_del(struct list_head *head)
{

}



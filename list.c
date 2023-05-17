#include "list.h"
#include "Types.h"
#include "errno.h"

// Determining the prvious point of head in linked list is depending on each case. 
// My opnion is that a head node points yourself. But, tail of head node does NULL.
// NULL means the end of list.
void list_init_head(struct list_node *head)
{
	head->prev = NULL;
	head->next = NULL;
}

struct list_node *list_add_head(struct list_node *head, struct list_node *new)
{
	if(head == NULL || new == NULL)
		return NULL;
	
	if(head->next) {
		new->prev = head;
		new->next = head->next;
		
		head->next->prev = new;
		head->next = new;
	} else {
		new->prev = head;
		new->next = head;
		
		head->prev = new;
		head->next = new;
	}

	return new;
}

struct list_node *list_add(struct list_node *head, struct list_node *new)
{
	if(head == NULL || new == NULL)
		return NULL;

	if(head->next) {
		new->prev = head->prev;
		new->next = head;

		head->prev->next = new;
		head->prev = new;	
	} else {
		new->prev = head;
		new->next = head;	
		
		head->prev = new;
		head->next = new;
	}

	return new;
}

void list_del(struct list_node *node)
{

}



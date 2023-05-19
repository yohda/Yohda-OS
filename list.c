#include "list.h"
#include "type.h"
#include "errno.h"
#include "debug.h"

// I decided to use a circle linked list because there is no need additional metadatas. 
// In the process in implementation of circle linked list, i will recognize the advantage of circle list.
// It is better to point prev & next pointer to itself. Pointing itself means empty state of list.
void list_init_head(struct list_node *head)
{
	head->prev = head;
	head->next = head;
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

struct list_node *list_del(struct list_node *head, struct list_node *node)
{
	if(head == NULL || node == NULL)
		return NULL;

	if(head->prev == head->next)
		return NULL;

	node->next->prev = node->prev;	
	node->prev->next = node->next;
	
	return node;
}



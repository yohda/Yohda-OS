#ifndef __LIST_H__
#define __LIST_H__

#define list_for_each(head, node) \
	for (node=head->next ; node!=head ; node=node->next)

struct list_node {
	struct list_node *next, *prev;
};

void list_init_node(struct list_node *node);
void list_add(struct list_node *p, struct list_node *new);
void list_add_node(struct list_node *p, struct list_node *new);
void list_for_each_entry();
void list_get_entry();
void list_del();
void list_sort();

#endif /*__LIST_H__*/

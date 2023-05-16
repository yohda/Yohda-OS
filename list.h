#ifndef __LIST_H__
#define __LIST_H__

#define list_for_each(head, node) \
	for (node=(head)->next ; (node)&&((node)!=(head)) ; node=(node)->next)

struct list_node {
	struct list_node *next, *prev;
};

void list_init_head(struct list_node *head);
void list_add_head(struct list_node *head, struct list_node *new);
void list_add(struct list_node *head, struct list_node *new);
void list_del();
void list_sort();

#endif /*__LIST_H__*/

#ifndef __LIST_H__
#define __LIST_H__

// In YohdaOS, a head of list hasn`t any data.
// The head of list is just management of list.
// So, you can see it in the below codes of initalization portion. In loop, first node is `head->next`.
#define list_for_each(head, node) \
	for (node=(head)->next ; (node)&&((node)!=(head)) ; node=(node)->next)

struct list_node {
	struct list_node *next, *prev;
};

void list_init_head(struct list_node *head);
struct list_node *list_add_head(struct list_node *head, struct list_node *new);
struct list_node *list_add(struct list_node *head, struct list_node *new);
struct list_node *list_del(struct list_node *head, struct list_node *node);
void list_sort();

#endif /*__LIST_H__*/

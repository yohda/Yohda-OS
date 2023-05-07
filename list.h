#ifndef __LIST_H__
#define __LIST_H__

struct list_head {
	struct list_head *next, *prev;
};

void list_init_head(struct list_head *head);
void list_add(struct list_head *p, struct list_head *new);
void list_add_head(struct list_head *p, struct list_head *new);
void list_del();
void list_sort();

#endif /*__LIST_H__*/

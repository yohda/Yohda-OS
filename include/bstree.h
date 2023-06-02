#ifndef _BSTREE_H_
#define _BSTREE_H_

#define BST_LEFT 	0x00	
#define BST_RIGHT 	0x01
#define BST_MATCH 	0x02
#define BST_ERROR 	0x02

struct bst_node {
	struct bst_node *node[3];
};

int bst_node_init(struct bst_node *root);
struct bst_node * bst_insert(struct bst_node *root, struct bst_node *new, int (*comp)(struct bst_node *, struct bst_node *));
//void bst_remove(struct bst_node root, struct bst_node *new, int (*comp)(struct bst_node*, struct bst_node *));
struct bst_node *bst_search(struct bst_node *root, void *key, int (*comp)(struct bst_node*, void *));

#endif

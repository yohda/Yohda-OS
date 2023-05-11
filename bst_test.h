#ifndef _BST_TEST_H_
#define _BST_TEST_H_

int bst_insert_comp(struct bst_node *root, struct bst_node *new);
int bst_search_comp(struct bst_node *root, void *key);
void bst_test_insert();
void bst_test_search_001();

#endif

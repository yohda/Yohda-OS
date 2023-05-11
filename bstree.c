#include "Types.h"
#include "bstree.h"
#include "mm.h"
#include "debug.h"

void bst_root_init(struct bst_node *root)
{
	root->node[BST_LEFT] = NULL;
	root->node[BST_RIGHT] = NULL;
	root->node[BST_MATCH] = NULL;
}

// In below function, it is very important of return value in third parameter.
// It it function pointer, that is, cusotom compare function.
// You have to follow the rule.
// The compare function returns `0` because key is equal and less than value of node, or, `1`. 
struct bst_node * bst_insert(struct bst_node *root, struct bst_node *new, int (*comp)(struct bst_node *, struct bst_node *))
{
	struct bst_node *tmp = root;
	int dir = 0;

	if(!root)
		return NULL;

	if(!new || !comp)
		return NULL;
	
	// You must check duplicated name before insertion.
	// Because bst insertion algorithum is under without duplicated name. So, before doing insertion, you must do that.
	if(bst_search(root, new, comp))
		return NULL;

	dir=comp(tmp, new);
	for( tmp = root ; tmp->node[dir] ; dir=comp(tmp, new))
		tmp = tmp->node[dir];

	tmp->node[dir] = new; 
	
	return &(tmp->node[dir]);
}

void bst_remove(struct bst_node root, void *key, int (*comp)(struct bst_node*, void *))
{

}

// In below function, it is very important of return value in third parameter.
// It it function pointer, that is, cusotom compare function.
// You have to follow the rule.
// The compare function returns `0` because key is less than value of node, or greater than `1`. If it is equal to that, you have to return `2`.

// Actually, in below the function of first parameter does not have to be pointer type.
// The reason why i used it is due to performance. data copy is longer than pointer copy.
struct bst_node *bst_search(struct bst_node *root, void *key, int (*comp)(struct bst_node*, void *))
{
	struct bst_node *tmp = root;
   	int dir = 0;
	
	if(!root)
		return NULL;
	
	if(!key || !comp)
		return NULL;

	dir=comp(tmp, key);
	for( tmp = root ; tmp->node[dir] ; dir=comp(tmp, key))
		tmp = tmp->node[dir];	
	
	return (dir == BST_MATCH) ? tmp : NULL;
}

void bst_sort(struct bst_node *root)
{

}

int bst_get_dep()
{


}

#include "Types.h"
#include "debug.h"
#include "bstree.h"
#include "bst_test.h"
#include "fat.h"
#include "mm.h"

struct bst_test {
	int dummy;
	char name[12];
	//struct bst_node *node;
	struct bst_node node;
};

int bst_search_comp(struct bst_node *_root, void *key)
{
	struct bst_test *root = container_of(_root, struct bst_test, node);
	char *name = (char *)key;
	int diff = 0;

	diff = strcmp(name, root->name);
	if(diff)
		return (diff < 0) ? BST_LEFT : BST_RIGHT;
		
	return BST_MATCH;
}

int bst_insert_comp(struct bst_node *_root, struct bst_node *_nw)
{
	struct bst_test *root = container_of(_root, struct bst_test, node);
	struct bst_test *nw = container_of(_nw, struct bst_test, node);
	int diff = 0;

	diff = strcmp(nw->name, root->name);
	if(diff)
		return (diff < 0) ? BST_LEFT : BST_RIGHT;
		
	return BST_MATCH;
}

void bst_test_insert()
{
	int i = 0;
	struct bst_test zzz;
	struct bst_node *srh; 
	struct bst_test *test;
	struct bst_test childs[10];
	char *names[10] = {
		"father", "apple", "bear", "banana", "function", "study", "name", "zero", "quite", "year"
	};

	memset(&zzz, 0, sizeof(zzz));
	//zzz.node = mm_alloc(sizeof(struct bst_node), MM_KL);
	bst_root_init(&(zzz.node));
	
	strncpy(zzz.name, "mother", 6);
	zzz.name[7] = 0;
	zzz.dummy = 3;
	
	srh = &(zzz.node);

	debug("root#0x%x, root.node#0x%x, name#%s\n", &zzz, &(zzz.node), zzz.name);
	memset(childs, 0, sizeof(childs));
	for(i=0 ; i<10 ; i++) {
		strncpy(childs[i].name, names[i], 12);
		debug("echilds[%d]#0x%x, node#0x%x, name#%s, len#%d\n", i, &childs[i], &(childs[i].node), names[i], strlen(names[i]));
		bst_insert(&(zzz.node), &(childs[i].node), bst_insert_comp);	
	}

	srh = bst_search(&(zzz.node), "apple", bst_search_comp);	
	test = container_of(srh, struct bst_test, node);
	debug("name#%s\n", test->name);

/*	
	srh = bst_search(&(zzz.node), "wef234fd", bst_search_comp);	
	if(!srh) {
		debug("NULL\n");
		return ;
	}
	
	test = container_of(srh, struct bst_test, node);
	debug("name#0x%s\n", test->name);
*/

	struct bst_test test001;
	strncpy(test001.name, "219jifew", 8);
	test001.name[9] = 0;
	test001.dummy = 6;

	struct bst_test *test002;	
	bst_insert(&(zzz.node), &(test001.node), bst_insert_comp);
	srh = bst_search(&(zzz.node), "219jifew", bst_search_comp);	
	test002 = container_of(srh, struct bst_test, node);
	debug("name#%s\n", test002->name);

}

void bst_test_search_001()
{


}

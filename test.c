#include "test.h"
#include "list.h"
#include "string.h"
#include "stack.h"
#include "mm/mm.h"
#include "mm/mmi.h"
#include "bitmap.h"
//#include "pool.h"
//#include "buddy.h"

#ifdef STRING_TEST
void string_test_strtok()
{
	char *test1 = mm_alloc(100, MM_KL);
	strncpy(test1, "123/456/abc/.txt", 30);
	
	msg("Case - `123/456/abc/.txt` \n");
	char *tk = strtok(test1, "/");
	do {
		msg("%s\n", tk);
		tk = strtok(NULL, "/");
	} while(tk);

	msg("Case - `/123/456/abc/.txt` \n");
	strncpy(test1, "/123/456/abc/.txt", 30);
	tk = strtok(test1, "/");
	do {
		msg("%s\n", tk);
		tk = strtok(NULL, "/");
	} while(tk);

	msg("Case - `/123/456/abc/.///` \n");
	strncpy(test1, "/123/456/abc/.///", 30);
	tk = strtok(test1, "/");
	do {
		msg("%s\n", tk);
		tk = strtok(NULL, "/");
	} while(tk);

}

void string_test_strrchr()
{
	char *test1 = mm_alloc(100, MM_KL);

	memset(test1, 0, 100);
	strncpy(test1, "//.txt", 30);

	char *test2 = strrchr(test1, '/');
	msg("%s\n", test2);

	char test3[test2-test1];
	strncpy(test3, test1, test2-test1);

	char *test4 = strrchr(test3, '/');
	msg("%c, %d\n", test4[0], strlen(test4));
	

}

void string_test_strlen()
{
	int len = 0;

	// Test for NULL	
	len = strlen(NULL);
	debug("%d\n", len);

	// Test for very big string 
	char ts2[3392822];
	memset(ts2, 0xFF, 3392822);
	len = strlen(ts2);
	debug("%d\n", len);

	len = strlen("efeef"); // 5
	debug("%d\n", len);
}

void string_test_strncpy()
{
	int len = 0;
	char dst[16];
	char src[16];
	char *ret = NULL;

	memset(dst, 0x00, 16);
	memset(src, 0x00, 16);	

	ret = strncpy(ret, "yohdaOS", 3); // ret is NULL
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(dst, "yohdaOS", 3);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(dst, "apple", 23478);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(dst, "house", -1);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(dst, "all time", 0);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(NULL, "all time", 234);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(dst, NULL, 23);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));

	ret = strncpy(dst, "efefefregrg23rwe2323", 23);
	debug("%s:%d\n", ret, strlen(dst));
	memset(dst, 0, sizeof(dst));
}

void string_test_strcmp()
{
	debug("test2#%d\n", strcmp(NULL, NULL));
	debug("test1#%d\n", strcmp(NULL, "efds123"));
	debug("test2#%d\n", strcmp("wefewf", NULL));
	debug("test3#%d\n", strcmp("wef23fef23", "100"));
	debug("test3#%d\n", strcmp("23", "wef23fef23"));
	debug("test3#%d\n", strcmp("123", "123"));
	debug("test3#%d\n", strcmp("werwe2", "wefwef"));
}
#endif

void stack_test_001()
{
	// This code happened error - storage size of ‘tmp’ isn’t known
	//struct stack tmp;
	//stack_init(&tmp, 128);
}

#ifdef LIST_TEST
struct list_test head; 
void list_test_init()
{
	debug("========root init========\n")
	memset(&head, 0, sizeof(head));
	list_init_head(&head.node);
	
	if(!strncpy(head.name, "mother", 6))
		debug("err\n");
	
	head.a = 14;
}

void list_test_crt_data(struct list_test *childs, int size)
{
	int i = 0;
	char *names[10] = {
		"father", "yohdaOS is best", "happy new year", NULL, "function", "study", NULL, "zero", "quite", "apple"
	};

	memset(childs, 0, size);
	for(i=0 ; i<10 ; i++) {
		strncpy(childs[i].name, names[i], 36);
		childs[i].a = i;
		//debug("childs[%d]#0x%x, node#0x%x, name#%s, len#%d\n", i, &childs[i], &(childs[i].node), names[i], strlen(names[i]));
		if(!list_add(&(head.node), &(childs[i].node)))
			debug("%d failed\n", i);
	}
}

void list_test_empty_list_for()
{
	struct list_test root;
	//struct list_node *head;
	struct list_node *test = NULL;
	list_init_head(&root.node);

	struct list_test tmp1;
	list_for_each(&root.node, test) {
		debug("ef\n");
	}
}

void list_test_add()
{
	struct list_test childs[10];
	
	list_test_init();
	list_test_crt_data(childs, sizeof(childs));
	
	struct list_node *aaa;
	struct list_test *temp;
	
	list_for_each(&head.node, aaa) 
	{
		temp = container_of(aaa, struct list_test, node);
		debug("0x%d\n", temp->a);	
	};
}

void list_test_del()
{
	struct list_test childs[10];
	
	list_test_init();

	if(!list_del(&head.node, &childs[0].node))
		debug("failed delete\n");	
	
	list_test_crt_data(childs, sizeof(childs));
	
	struct list_node *aaa;
	struct list_test *temp;

	if(!list_del(&head.node, &childs[0].node))
		debug("failed delete\n");	

	if(!list_del(&head.node, &childs[5].node))
		debug("failed delete\n");	
	
	if(!list_del(&head.node, &childs[2].node))
		debug("failed delete\n");	

	if(!list_del(&head.node, NULL))
		debug("failed delete\n");	
	
	list_for_each(&head.node, aaa) 
	{
		temp = container_of(aaa, struct list_test, node);
		debug("0x%d\n", temp->a);	
	};
}

#endif

int bst_search_comp(struct bst_node *_root, void *key)
{
	struct bst_test *root = container_of(_root, struct bst_test, node);
	char *name = (char *)key;
	int diff = 0;

	//debug("root#%s, target#%s\n", root->name, name);
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

	// two check the error cases.
	// the first is address is null(name).
	// the second is empty string(name[0]). 
	// there can be the case that address isn`t NULL and empty string.
	if((!root->name || !nw->name) 
	|| (!root->name[0] || !nw->name[0]))
		return BST_ERROR;

	diff = strcmp(nw->name, root->name);
	if(diff)
		return (diff < 0) ? BST_LEFT : BST_RIGHT;
		
	return BST_ERROR;
}

#ifdef BST_TEST
struct bst_test root;

void bst_test_init()
{
	debug("========root init========\n")
	memset(&root, 0, sizeof(root));
	bst_node_init(&(root.node));
	
	if(!strncpy(root.name, "mother", 6))
		debug("err\n");

	root.dummy = 14;
}

void bst_test_crt_test_data(struct bst_test *childs, int size)
{
	int i = 0;
	char *names[10] = {
		"father", "yohdaOS is best", "happy new year", NULL, "function", "study", NULL, "zero", "quite", "apple"
	};

	memset(childs, 0, size);
	for(i=0 ; i<10 ; i++) {
		strncpy(childs[i].name, names[i], 36);
		childs[i].dummy = i;
		//debug("childs[%d]#0x%x, node#0x%x, name#%s, len#%d\n", i, &childs[i], &(childs[i].node), names[i], strlen(names[i]));
		if(!bst_insert(&(root.node), &(childs[i].node), bst_insert_comp))
			debug("%d failed\n", i);
	}
}

void bst_test_insert()
{
	struct bst_node *srh; 
	struct bst_test *test;
	struct bst_test childs[10];
	
	bst_test_init();
	bst_test_crt_test_data(childs, sizeof(childs));

	srh = bst_search(&(root.node), "function", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	}
	test = container_of(srh, struct bst_test, node);
	debug("name#%s dummy#%d\n", test->name, test->dummy);
	srh = test = NULL;


	srh = bst_search(&(root.node), "apple", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	}
	test = container_of(srh, struct bst_test, node);
	debug("name#%s dummy#%d\n", test->name, test->dummy);
	srh = test = NULL;


	srh = bst_search(&(root.node), "zero", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	}
	test = container_of(srh, struct bst_test, node);
	debug("name#%s dummy#%d\n", test->name, test->dummy);
	srh = test = NULL;


	srh = bst_search(&(root.node), "23ruf", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	}
		
	test = container_of(srh, struct bst_test, node);
	debug("name#%s dummy#%d\n", test->name, test->dummy);
	srh = test = NULL;
}

// Test for no-exist name.
void bst_test_search_noexist_name()
{
	struct bst_node *srh; 
	struct bst_test *test;
	struct bst_test childs[10];
	
	bst_test_init();
	bst_test_crt_test_data(childs, sizeof(childs));

	srh = bst_search(&(root.node), NULL, bst_search_comp);	
	if(!srh) {
		debug("failed\n");
	} else {
		test = container_of(srh, struct bst_test, node);
		debug("name#%s dummy#%d\n", test->name, test->dummy);
	}
	srh = test = NULL;

	srh = bst_search(&(root.node), "function", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	} else {
		test = container_of(srh, struct bst_test, node);
		debug("name#%s dummy#%d\n", test->name, test->dummy);
	}
	srh = test = NULL;

	srh = bst_search(&(root.node), "apple", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	}
	test = container_of(srh, struct bst_test, node);
	debug("name#%s dummy#%d\n", test->name, test->dummy);
	srh = test = NULL;

	srh = bst_search(&(root.node), "zero", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
		return ;
	}
	test = container_of(srh, struct bst_test, node);
	debug("name#%s dummy#%d\n", test->name, test->dummy);
	srh = test = NULL;

	srh = bst_search(&(root.node), "23ruf", bst_search_comp);	
	if(!srh) {
		debug("failed\n");
	} else {
		test = container_of(srh, struct bst_test, node);
		debug("name#%s dummy#%d\n", test->name, test->dummy);
	}
	srh = test = NULL;
}

#endif

#ifdef PL_TEST
extern struct mmif sm_if;
void pool_test()
{
	sm_if.mm_init(0x2000000, 175*1024);
	void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;

	// Start from here, allocation for pool memeory
	p1 = sm_if.mm_alloc(15);
	pl_debug("0x%x\n", p1);

	p2 = sm_if.mm_alloc(48);
	pl_debug("0x%x\n", p2);

	p3 = sm_if.mm_alloc(27);
	pl_debug("0x%x\n", p3);

	p4 = sm_if.mm_alloc(12);
	pl_debug("0x%x\n", p4);

	p5 = sm_if.mm_alloc(92);
	pl_debug("0x%x\n", p5);

/////////////////////////////////////
	// Start from here, free pool de-allocation

	sm_if.mm_free(p3);
	sm_if.mm_free(p4);

/////////////////////////////////////
	p3 = sm_if.mm_alloc(14);
	pl_debug("0x%x\n", p3);

	p4 = sm_if.mm_alloc(7);
	pl_debug("0x%x\n", p4);

	p6 = sm_if.mm_alloc(14);
	pl_debug("0x%x\n", p6);

	p7 = sm_if.mm_alloc(7);
	pl_debug("0x%x\n", p7);

}	
#endif

#ifdef BUD_TEST
extern struct mmif pm_if;
void bud_test()
{
	pm_if.mm_init(0x2000000, 175*1024);
	
	void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14, *p15, *p16, *p17, *p18, *p19, *p20;

	p1 = pm_if.mm_alloc(921);
	bud_debug("1-0x%x\n", p1);

	p2 = pm_if.mm_alloc(3293);
	bud_debug("2-0x%x\n", p2);

	p3 = pm_if.mm_alloc(18293);
	//bud_debug("3-0x%x\n", p3);

	p4 = pm_if.mm_alloc(1208);
	bud_debug("4-0x%x\n", p4);

	p5 = pm_if.mm_alloc(13029);
	bud_debug("5-0x%x\n", p5);

/////////////////////////////////////////////////////////

	p6 = pm_if.mm_alloc(7829);
	bud_debug("6-0x%x\n", p6);

	p7 = pm_if.mm_alloc(15273);
	bud_debug("7-0x%x\n", p7);

	p8 = pm_if.mm_alloc(210293);
	//bud_debug("8-0x%x\n", p8);

	p9 = pm_if.mm_alloc(15273);
	bud_debug("9-0x%x\n", p9);

	p10 = pm_if.mm_alloc(NULL);
	//bud_debug("10-0x%x\n", p10);


////////////////////////////////////////////////////////////


	p11 = pm_if.mm_alloc(15273);
	bud_debug("11-0x%x\n", p11);
	
	p12 = pm_if.mm_alloc(15273);
	bud_debug("12-0x%x\n", p12);
	
	p13 = pm_if.mm_alloc(15273);
	bud_debug("13-0x%x\n", p13);

	p14 = pm_if.mm_alloc(15273);
	bud_debug("14-0x%x\n", p14);

	p15 = pm_if.mm_alloc(15273);
	bud_debug("15-0x%x\n", p15);

////////////////////////////////////////////////////

	p16 = pm_if.mm_alloc(15273);
	bud_debug("16-0x%x\n", p16);

	p17 = pm_if.mm_alloc(7382);
	bud_debug("17-0x%x\n", p17);

	p18 = pm_if.mm_alloc(1832);
	bud_debug("18-0x%x\n", p18);

	p19 = pm_if.mm_alloc(3920);
	bud_debug("19-0x%x\n", p19);
	
	p20 = pm_if.mm_alloc(1832);
	bud_debug("20-0x%x\n", p20);
/////////////////////////////////////
	// Start from here, free pool de-allocation

	//mm_free(p3);
	//mm_free(p4);

#ifdef MM_TEST_HEAP128K_LL2048_BITMAP32
	void *a1 = pm_if.mm_alloc(15400, MM_KL);
	//bud_debug("15.4K#0x%x\n", a1);
	bud_debug("ord#0 block#0x%x\n", *bitmaps[0]);
	bud_debug("ord#1 block#0x%x\n", *bitmaps[1]);
	bud_debug("ord#2 block#0x%x\n", *bitmaps[2]);
	bud_debug("ord#3 block#0x%x\n", *bitmaps[3]);
	bud_debug("ord#4 block#0x%x\n", *bitmaps[4]);
	bud_debug("ord#5 block#0x%x\n", *bitmaps[5]);
	bud_debug("ord#6 block#0x%x\n", *bitmaps[6]);

	//void *a2 = pm_if.mm_alloc(6700);
	//bud_debug("6.7K#0x%x\n", a2);
	//bud_debug("ord#0 block#0x%x\n", *bitmaps[0]);
	//bud_debug("ord#1 block#0x%x\n", *bitmaps[1]);
	//bud_debug("ord#2 block#0x%x\n", *bitmaps[2]);
	//bud_debug("ord#3 block#0x%x\n", *bitmaps[3]);
	//bud_debug("ord#4 block#0x%x\n", *bitmaps[4]);
	//bud_debug("ord#5 block#0x%x\n", *bitmaps[5]);
	//bud_debug("ord#6 block#0x%x\n", *bitmaps[6]);

	//void *a3 = pm_if.mm_alloc(1200);
	//bud_debug("1.2K#0x%x\n", a3);

	//void *a4 = pm_if.mm_alloc(43700);
	//bud_debug("43.7K#0x%x\n", a4);
	
	//void *a5 = pm_if.mm_alloc(27700);
	//bud_debug("27.7K#0x%x\n", a5);
	
	//void *a6 = pm_if.mm_alloc(27700); // Out of memory
	//bud_debug("27.7K#0x%x\n", a6);
	
	//void *a7 = pm_if.mm_alloc(3029);

	//void *a8 = pm_if.mm_alloc(2200); // Out of memory
	//bud_debug("2.2K#0x%x\n", a8);
	
	//void *a9 = pm_if.mm_alloc(892);
	
	//mm_free(a3);
	//mm_free(a8);
	//mm_free(a9);
	//mm_free(a7);
#endif
}
#endif

#ifdef MMI_TEST
void mmi_test()
{
	mmi_init(0x2000000, 1*1023*1024*815);

	void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14, *p15, *p16, *p17, *p18, *p19, *p20;
	void *s1, *s2, *s3, *s4;

	p1 = yalloc(1921);
	mm_debug("p1-0x%x\n", p1);
	
	s1 = yalloc(921);
	mm_debug("s1-0x%x\n", s1);
 
	p2 = yalloc(3293);
	mm_debug("2-0x%x\n", p2);

	s2 = yalloc(23);
	mm_debug("2-0x%x\n", s2);

	p3 = yalloc(18293);
	//mm_debug("3-0x%x\n", p3);

	p4 = yalloc(1208);
	mm_debug("4-0x%x\n", p4);

	s3 = yalloc(233);
	mm_debug("4-0x%x\n", s3);
	
	p5 = yalloc(13029);
	mm_debug("5-0x%x\n", p5);

/////////////////////////////////////////////////////////

	p6 = yalloc(7829);
	mm_debug("6-0x%x\n", p6);

	p7 = yalloc(15273);
	mm_debug("7-0x%x\n", p7);

	p8 = yalloc(210293);
	//mm_debug("8-0x%x\n", p8);

	p9 = yalloc(15273);
	mm_debug("9-0x%x\n", p9);

	p10 = yalloc(NULL);
	//mm_debug("10-0x%x\n", p10);


////////////////////////////////////////////////////////////


	p11 = yalloc(15273);
	mm_debug("11-0x%x\n", p11);
	
	p12 = yalloc(15273);
	mm_debug("12-0x%x\n", p12);
	
	p13 = yalloc(15273);
	mm_debug("13-0x%x\n", p13);

	p14 = yalloc(15273);
	mm_debug("14-0x%x\n", p14);

	p15 = yalloc(15273);
	mm_debug("15-0x%x\n", p15);

////////////////////////////////////////////////////

	p16 = yalloc(15273);
	mm_debug("16-0x%x\n", p16);

	p17 = yalloc(7382);
	mm_debug("17-0x%x\n", p17);

	p18 = yalloc(1832);
	mm_debug("18-0x%x\n", p18);

	p19 = yalloc(3920);
	mm_debug("19-0x%x\n", p19);
	
	p20 = yalloc(1832);
	mm_debug("20-0x%x\n", p20);
/////////////////////////////////////
	// Start from here, free pool de-allocation

	yfree(s1);
	yfree(s2);
	yfree(s3);
	yfree(p20);
	yfree(p18);
	yfree(p17);
	yfree(p16);
	yfree(p15);
	yfree(p14);
	yfree(p13);
	yfree(p12);
	yfree(p11);
	yfree(p9);
	yfree(p7);
	yfree(p5);
	yfree(p6);
	yfree(p4);
	yfree(p2);
	yfree(p1);

	return ;
}
#endif

#ifdef LAZY_BUD_TEST
void lazy_buddy_test()
{
	//mmi_init(0x2000000, 1*1023*1024*815);

	void *p1, *p2, **p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14, *p15, *p16, *p17, *p18, *p19, *p20;

	p1 = pm_if.mm_alloc(1921);
	mm_debug("1-0x%x\n", p1);

	pm_if.mm_free(p1);
	
	p2 = pm_if.mm_alloc(18293);
	mm_debug("2-0x%x\n", p2);

	pm_if.mm_free(p2);
	
	p4 = pm_if.mm_alloc(1921);
	mm_debug("1-0x%x\n", p4);
	
	pm_if.mm_free(p4);
	
	int i;
	for(i=0 ; i<10000; i++) {
		p3[i] = pm_if.mm_alloc(1893);
		//mm_debug("3-0x%x\n", p3[i]);
	}

	for(i=0 ; i<10000; i++) {
		pm_if.mm_free(p3[i]);
	}

	p5 = pm_if.mm_alloc(1921);
	//mm_debug("1-0x%x\n", p5);
	
	pm_if.mm_free(p5);
	
	p6 = pm_if.mm_alloc(24932);
	//mm_debug("4-0x%x\n", p6);

	pm_if.mm_free(p6);
	
	return ;
	
	p5 = yalloc(13029);
	mm_debug("5-0x%x\n", p5);

/////////////////////////////////////////////////////////

	p6 = yalloc(7829);
	mm_debug("6-0x%x\n", p6);

	p7 = yalloc(15273);
	mm_debug("7-0x%x\n", p7);

	p8 = yalloc(210293);
	//mm_debug("8-0x%x\n", p8);

	p9 = yalloc(15273);
	mm_debug("9-0x%x\n", p9);

	p10 = yalloc(NULL);
	//mm_debug("10-0x%x\n", p10);


////////////////////////////////////////////////////////////


	p11 = yalloc(15273);
	mm_debug("11-0x%x\n", p11);
	
	p12 = yalloc(15273);
	mm_debug("12-0x%x\n", p12);
	
	p13 = yalloc(15273);
	mm_debug("13-0x%x\n", p13);

	p14 = yalloc(15273);
	mm_debug("14-0x%x\n", p14);

	p15 = yalloc(15273);
	mm_debug("15-0x%x\n", p15);

////////////////////////////////////////////////////

	p16 = yalloc(15273);
	mm_debug("16-0x%x\n", p16);

	p17 = yalloc(7382);
	mm_debug("17-0x%x\n", p17);

	p18 = yalloc(1832);
	mm_debug("18-0x%x\n", p18);

	p19 = yalloc(3920);
	mm_debug("19-0x%x\n", p19);
	
	p20 = yalloc(1832);
	mm_debug("20-0x%x\n", p20);
/////////////////////////////////////
	// Start from here, free pool de-allocation

	yfree(p20);
	yfree(p18);
	yfree(p17);
	yfree(p16);
	yfree(p15);
	yfree(p14);
	yfree(p13);
	yfree(p12);
	yfree(p11);
	yfree(p9);
	yfree(p7);
	yfree(p5);
	yfree(p6);
	yfree(p4);
	yfree(p2);
	yfree(p1);

	return ;
}
#endif

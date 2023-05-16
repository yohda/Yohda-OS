#include "test.h"
#include "list.h"
#include "string.h"
#include "stack.h"
#include "mm.h"

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

void stack_test_001()
{
	// This code happened error - storage size of ‘tmp’ isn’t known
	//struct stack tmp;
	//stack_init(&tmp, 128);
}

void list_test_add()
{
	struct list_node *head;
	list_init_head(head);

	struct list_test tmp1;
	tmp1.a = 1;
	tmp1.b = 323;
	
	struct list_test tmp2;
	tmp2.a = 2;
	tmp2.b = 98;

	struct list_test tmp3;
	tmp3.a = 3;
	tmp3.b = 4829;
	
	struct list_test tmp4;
	tmp4.a = 4;
	tmp4.b = 423829;

	struct list_test tmp5;
	tmp5.a = 5;
	tmp5.b = 482429;

	list_add_head(head, &tmp1.list);
	list_add_head(head, &tmp2.list);
	list_add_head(head, &tmp3.list);	
	list_add_head(head, &tmp4.list);	
	list_add_head(head, &tmp5.list);	
	
	struct list_node *aaa;
	struct list_test *temp;
	
	list_for_each(head, aaa) 
	{
		temp = container_of(aaa, struct list_test, list);
		debug("0x%d\n", temp->a);	
	};
}

void list_test_for()
{

}

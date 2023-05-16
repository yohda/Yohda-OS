#ifndef _TEST_H_
#define _TEST_H_

#define LIST_TEST
#define BST_TEST
#define STK_TEST
#define STRING_TEST

#include "type.h"
#include "list.h"

void string_test_strtok();
void stack_test_001();

struct list_test {
	int a;
	int b;
	struct list_node list;
};

void list_test_add();
void list_test_for();

#endif

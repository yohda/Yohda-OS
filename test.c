#include "test.h"
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
	strncpy(test1, "123/456/abc/.txt", 30);
	
	char *test2 = strrchr(test1, '/');
	msg("%s\n", test2);

	char test3[test2-test1];
	strncpy(test3, test1, test2-test1);

	char *test4 = strrchr(test3, '/');
	msg("%s\n", test4);
}

void stack_test_001()
{
	// This code happened error - storage size of ‘tmp’ isn’t known
	//struct stack tmp;
	//stack_init(&tmp, 128);
}


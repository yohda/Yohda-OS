#include "string.h"
#include "mm.h"

// https://en.cppreference.com/w/c/string/byte/strtok
char *strtok(const char *str, const char *delim)
{
	static char *stk = NULL;
	char *tmp = NULL;
	static int len = 0; 
	static int i = 0;

	if(!stk && !str)
		return NULL;	

	if(str) {
		i = 0;
		stk = str;
		len = strlen(str);
	}

	tmp = (i<len) ? stk+i : NULL;
	for(; i<len ; i++) {
		if(stk[i] == delim[0]) {
			stk[i++] = 0;
			break;
		}
	}

	return tmp; 
}

// https://en.cppreference.com/w/c/string/byte/strrchr
char *strrchr(const char *str, char ch)
{
	int i = 0, len = 0;
	if(!str)
		return NULL;

	len = strlen(str);
	for(i=len ; i>-1 ; i--) {
		if(str[i]==ch)	
			return str+i;			
	}

	return NULL;
}

// https://en.cppreference.com/w/c/string/byte/strchr
char *strchr(const char *str, char ch)
{
	int i, len = strlen(str);
	
	if(!str || !len)
		return NULL;

	for(i=0 ; i<len ; i++) {
		if(str[i] == ch)
			return str+i;
	}

	return NULL;
}

// https://en.cppreference.com/w/c/string/byte/strlen
int strlen(const char* str)
{
	int n = 0;
	if(!str)
		return 0;

	while(str[n])
		n++;

	return n;
}

// https://en.cppreference.com/w/c/string/byte/strncpy
char *strncpy(char *dst, const char *src, int cnt)
{
	int i;

	for(i=0 ; i<cnt ; i++) {
		dst[i] = src[i];
	}

	return dst;
}

// https://en.cppreference.com/w/c/string/byte/strcmp
int strcmp(const char *s1, const char *s2)
{
	int n1 = strlen(s1);
	int n2 = strlen(s2);
	int len = min(n1, n2);
	int i, s;

	for(i = 0; i < len; i++) {
		s = s1[i] - s2[i];
		if(s)
			return s;	
	}
			
	return 0;
}

// https://en.cppreference.com/w/c/string/byte/memset
void *memset(void *dst, int ch, int cnt)
{
	int i, n = cnt;

	if(n < 1)
		return NULL;

	while(n--)
		*(((u8 *)dst)+n) = (u8)ch;

	return dst;
}

// https://en.cppreference.com/w/c/string/byte/memcpy
void *memcpy(void *dst, const void *src, int cnt)
{
	int n = cnt;

	while(n--){
		*(((u8 *)dst)+n) = *(((u8 *)src)+n);
	}

	return dst;
}

#include "string.h"
#include "mm/mm.h"
#include "limit.h"

void reverse(char *str)
{
	int len = strlen(str), i = 0, j = 0;
	char buf[len];

	i = 0;
	j = len-1;
	while(i < len)
		buf[j--] = str[i++];
			
	memcpy(str, buf, len);
}

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
// strlen is based on NULL-terminaed string. If you passed the string withut NULL-terminated, it happended the overflow and, it`s the shame that  there is no way to decide to exit, So, you must pass the NULL-terminated string.
int strlen(const char* str)
{
	int n = 0, i = 0;
	if(!str)
		return 0;

	for(i=0 ; str[n]&&(i<=INT_MAX); i++)
		n++;

	return (n != INT_MAX) ? n : 0;
}

// https://en.cppreference.com/w/c/string/byte/strncpy
// In strncpy, it is very important to determine length for copy src to dst.
// As you know, if cnt is greater than length of dst, that is overflow. So, the point is that cnt must be equal to less than length of dst.
// But, the possibility about length-dst of zero is present. 
char *strncpy(char *dst, const char *src, int cnt)
{
	int i = 0 , len = 0;

	if(!dst || !src)
		return NULL;

	if(cnt < 1)
		return NULL;
	
	len = min(strlen(src), cnt);
	for(i=0 ; i<len ; i++) {
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

	if(!len)
		return *s1 - *s2;

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
	int i = 0;
	
	if(!dst || !src)
		return NULL;

	if(cnt < 1)
		return NULL;
	
	for(i=0 ; i<cnt ; i++)
		*(((u8 *)dst)+i) = *(((u8 *)src)+i);

	return dst;
}

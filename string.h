#ifndef _STRING_H_
#define _STRING_H_

#include "Types.h"
#include "debug.h"

char *strtok(const char *str, const char *delim);
char *strrchr(const char *str, char ch);
char *strchr(const char *str, char ch);
int strlen(const char* str);
int strcmp(const char *s1, const char *s2);
void *memset(void *dst, int ch, int cnt);
char *strncpy(char *dst, const char *src, int cnt);
void *memcpy(void *dst, const void *src, int cnt);
	
#endif

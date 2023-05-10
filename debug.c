#include "debug.h"

void mdebug(void *addr, int n)
{
	int i;
		
	kPrintf("0x%x(%d) - 0x", addr, n);	
	for(i=0 ; i<(n+3)/4; i++) {
		kPrintf("%x_", *(((u32 *)addr)+i));	
	}
	kPrintf("\n");	
}

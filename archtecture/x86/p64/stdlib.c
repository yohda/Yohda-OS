#include "stdlib.h"
#include "string.h"
#include "limit.h"

// https://en.wikibooks.org/wiki/C_Programming/stdlib.h/itoa
void itoa(int n, char s[])
{
	char buf[INT_MAX_DIG];
	int i = 0, nxt = n;
	if(nxt < 0)
		nxt = -n;
		
	memset(buf, 0, sizeof(buf));			
	do {
		buf[i++] = nxt%10 + '0';
		nxt /= 10;
	} while(nxt);

	if(n < 0)
		buf[i] = '-';

	reverse(buf);
	memcpy(s, buf, INT_MAX_DIG);
}

void itoh(u32 n, char s[])
{
	char buf[INT_MAX_DIG];
	u32 d=0, r=0;
	int i = 0;
	
	d = n;
	memset(buf, 0, sizeof(buf));
	do {
		r = d%16;
		if (r < 10) { // less than 10
			buf[i++] = r + '0';
		} else { // if greater than or equal 10
			buf[i++] = r - 10 + 'A';
		} 
	} while((d/=16));
	
	reverse(buf);
	memcpy(s, buf, INT_MAX_DIG);
}

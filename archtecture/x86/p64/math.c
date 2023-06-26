#include "math.h"
#include "debug.h"

int abs(const int n)
{
	return (n < 0) ? (n * -1) : n;
}

int max(const int a, const int b)
{
	return (a >= b) ? a : b;
}

int min(const int a, const int b)
{
	return (a >= b) ? b : a;
}

int log(u32 base, u32 arg)
{
	u32 exp = 0, tmp = 0;

	// base never be negative and one(1), zero(0).
	if(base == 1 || base <= 0) {
		//debug("Invalid Parameter#%d\n", base);
		return -EINVAL;
	}

	// argument is greater than 0.
	if(arg < 1) {
		//debug("Invalid Parameter#%d\n", arg);
		return -EINVAL;
	}

	tmp = arg;
	do {
		tmp = tmp / base;
		exp++;	
	} while(tmp);

	return exp-1; 
}


// This function only supports interger.  
// So, a exponent never be negative.
// And, a base(x) must be greater than 0 and not be one(1).
u32 power(int base, int exp)
{
	int ret = 1;
	
	// base is 
	if(base == 1 || base <= 0) {
		//debug("Invalid parameter#%d\n", base);	
		return -EINVAL;
	}

	if(exp < 0)
		return -EINVAL;

	if(!exp)
		return 1;

	while(exp--)
		ret *= base;

	return ret;
}

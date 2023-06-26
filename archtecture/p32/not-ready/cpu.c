#include "cpu.h"
#include "vga.h"

extern void cli(void);

int cpu_to_long()
{
	cli();
	cpuid_check_valid();
	
	int a = CPUID_X64;
	int b = CPUID_EXT_FUNCS;
	int c = CPUID_STD_FUNCS;
	int d = CPUID_IS_VALID;

	kprintf("x64#%x, ext#%x, std#%x, vali#%x\n", a, b, c, d);
	
}

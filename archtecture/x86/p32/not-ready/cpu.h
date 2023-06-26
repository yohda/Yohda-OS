#ifndef _CPU_H_
#define _CPU_H_

#include "type.h"

extern int cpuid_check_valid(void);

extern int CPUID_IS_VALID;
extern int CPUID_X64;
extern int CPUID_EXT_FUNCS;
extern int CPUID_STD_FUNCS;

int cpu_to_long();
void cli();
void sti();

#endif

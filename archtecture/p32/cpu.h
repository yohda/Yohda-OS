#ifndef _CPU_H_
#define _CPU_H_

extern void mode64(void);
extern void cpu_init(void);
extern void cli(void);
extern void sti(void);

extern int CPUID;		
extern int CPUID_64;			
extern int CPUID_EXT_FUNCS;		
extern int CPUID_PAGE_1GB;
extern int CPUID_STD_FUNCS;	

#endif

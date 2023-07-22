#include "lock.h"
#include "sys.h"

void lock_irq_save(void)
{
	uint32_t eflags = 0;
	__asm__ __volatile__ (
					"pushfl\n\t" 
			  		"pop %0;"
			  		:"=r"(eflags));

	cli();	
}

void lock_irq_restore()
{
	uint32_t eflags;
	sti();
}

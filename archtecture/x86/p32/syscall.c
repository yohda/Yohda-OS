#include "syscall.h"
#include "interrupt.h"
#include "io.h"

struct syscall {
	int num;
};

static void *syscall_table[] = 
{
	kprintf
};


void syscall_dispatch_handler(const int fn)
{
	void *eip = syscall_table[fn];
}

void syscall_init(void)
{
	// Task is not permited from re-entrant. So, you must set interupt gate, not trap.
	interrupt_register_handler(INT_VEC_SYS_CALL, isr_syscall, IDT_USER_INTR);
}

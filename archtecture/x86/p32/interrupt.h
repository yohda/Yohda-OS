#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "type.h"

extern void isr_division_error(void);
extern void isr_non_maskable_interrupt(void);
extern void isr_general_protection_fault(void);
extern void isr_page_fault(void);
extern void isr_system_timer_interrupt(void);
extern void isr_ps2_keyboard_interrupt(void);
extern void isr_syscall(void);

struct exception_info {
	uint32_t ds, es, fs, gs;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t irq ,err;
	uint32_t eip, cs, eflags, uesp, uss; // prefix `u` is related to user-level
};

enum {
	INT_VEC_DBZ 		= 0x00,
	INT_VEC_DBG,
	INT_VEC_NMI,
	INT_VEC_BRK,
	INT_VEC_OVF,
	INT_VEC_IO 			= 0x06,
	INT_RSVD0 			= 0x0C,
	INT_VEC_GPF 		= 0x0D,
	INT_VEC_PF 			= 0x0E,
	INT_RSVD1 			= 0x1F,
	INT_VEC_SYS_TIMER 	= 0x20,
	INT_VEC_PS2_KEY		= 0x21,
	INT_VEC_SYS_CALL	= 0x80,
	INT_VEC_MAX
};

#define IDT_KERN_INTR			(0x8E)
#define IDT_KERN_TRAP			(0x8F)
#define IDT_USER_INTR			(0xEE)
#define IDT_USER_TRAP			(0xEF)

void interrupt_register_handler(const int vector, const u32 offset, const u8 attr);
void interrupt_init(void);

#endif

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "type.h"

extern void isr_division_error(void);
extern void isr_non_maskable_interrupt(void);
extern void isr_general_protection_fault(void);
extern void isr_page_fault(void);
extern void isr_system_timer_interrupt(void);
extern void isr_ps2_keyboard_interrupt(void);

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
	INT_VEC_MAX
};

#define IDT_ATTR_TYPE_32INT 	(0x0E)
#define IDT_ATTR_TYPE_32TRAP 	(0x0F)
#define IDT_ATTR_TYPE_16INT		(0x06)
#define IDT_ATTR_TYPE_16TRAP	(0x07)
#define IDT_PRES				(0x80)
/*
// interrupt descriptor table register
struct idtr32 {
	u16 limit;
	u32 base;
}__attribute__((packed));

// interrupt descriptor table entry
struct idt32_entry {
	u16 loffset;
	u16 selector;
	u8 rsvd;
	u8 attr;
	u16 hoffset;
}__attribute__((packed));
*/
#endif

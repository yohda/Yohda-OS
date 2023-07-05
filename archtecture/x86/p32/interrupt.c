#include "interrupt.h"
#include "debug.h"
#include "cpu.h"

#define IDT_ENTRY_SIZE	0x10
#define IDT_MAX_ENTRYS 	0x100

static struct idtr32 idtr;
static struct idt32_entry  __attribute__((aligned(IDT_ENTRY_SIZE))) idt_tbl[IDT_MAX_ENTRYS];

void isr_common_handler(const int irq)
{
	debug("irq:0x%x\n", irq);

	while(1);
}

void isr_division_error_handler(const int irq)
{
	debug("irq:0x%x\n", irq);

	while(1);
}

void isr_non_maskable_interrupt_handler(const int irq)
{
	
	debug("irq:0x%x\n", irq);
	
	while(1);
}

void isr_general_protection_fault_handler(const struct exception_info exc)
{
	vt_cls();

	debug("irq:0x%x err#0x%x\n", exc.irq, exc.err);
	debug("eip:0x%x eflags#0x%x\n", exc.eip, exc.eflags);
	debug("eax#0x%x ebx#0x%x\necx#0x%x edx#0x%x\nesi#0x%x edi#0x%x\nesp#0x%x ebp#0x%x\n", 
			exc.eax, exc.ebx, exc.ecx, exc.edx,
			exc.esi, exc.edi, exc.esp, exc.ebp);
	debug("cs:0x%x ds#0x%x es#0x%x fs#0x%x gs#0x%x", exc.cs, exc.ds, exc.es, exc.fs, exc.gs);
	
	while(1);
}

void isr_page_fault_handler(const struct exception_info exc)
{
	vt_cls();
	
	debug("irq:0x%x err#0x%x\n", exc.irq, exc.err);
	debug("eip:0x%x eflags#0x%x\n", exc.eip, exc.eflags);
	debug("eax#0x%x ebx#0x%x ecx#0x%x edx#0x%x esi#0x%x edi#0x%x esp#0x%x ebp#0x%x\n", 
			exc.eax, exc.ebx, exc.ecx, exc.edx,
			exc.esi, exc.edi, exc.esp, exc.ebp);
	debug("cs:0x%x ds#0x%x es#0x%x fs#0x%x gs#0x%x", exc.cs, exc.ds, exc.es, exc.fs, exc.gs);
	
	while(1);
}

void idt_reg_isr(const int vector, const u32 offset, const u8 attr)
{
	if(vector<0 || vector>IDT_MAX_ENTRYS)
		return -1;

	idt_tbl[vector].loffset = offset;
	idt_tbl[vector].hoffset = offset >> 16;
	idt_tbl[vector].selector = 0x08;
	idt_tbl[vector].attr = attr;
}

void interrupt_init()
{
	int i;

	idtr.limit = sizeof(idt_tbl) - 1;	
	idtr.base = (u32)idt_tbl;

	for(i=0 ; i<INT_VEC_MAX; i++) {
		idt_reg_isr(i, isr_common_handler, 0x8F);	
	}

	idt_reg_isr(INT_VEC_DBZ, isr_division_error, 0x8F);
	idt_reg_isr(INT_VEC_NMI, isr_non_maskable_interrupt, 0x8F);
	idt_reg_isr(INT_VEC_GPF, isr_general_protection_fault, 0x8F);
	idt_reg_isr(INT_VEC_PF, isr_page_fault, 0x8F);
	
	__asm__ __volatile__ ("lidt %0" : : "m"(idtr));
}

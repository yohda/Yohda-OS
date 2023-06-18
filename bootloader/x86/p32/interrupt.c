#include "interrupt.h"
#include "vga.h"

#define IDT_BASE_ADDR 	0x108000 
#define IDT_ENTRY_SIZE	0x10
#define IDT_MAX_ENTRYS 	0x100

extern void isr_division_error(void);
extern void isr_non_maskable_interrupt(void);
extern void isr_general_protection_fault(void);
extern void isr_page_fault(void);
extern void isr_system_timer_interrupt(void);

static struct idtr32 idtr;
static struct idt32_entry  __attribute__((aligned(IDT_ENTRY_SIZE))) idt_tbl[IDT_MAX_ENTRYS];

void isr_common_handler(const int irq)
{
	vga_text_write(__func__);
	vga_text_write(irq);
}

void isr_division_error_handler(const int irq)
{
	vga_text_write(__func__);	
}

void isr_non_maskable_interrupt_handler(const int irq)
{
	vga_text_write(__func__);	
}

void isr_general_protection_fault_handler(const int irq)
{
	vga_text_write(__func__);	
}

void isr_page_fault_handler(const int irq, const u32 err)
{
	vga_text_write(__func__);	
}

void isr_system_timer_handler(const int irq)
{
	vga_text_write(__func__);	
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
	idt_reg_isr(INT_VEC_SYS_TIMER, isr_system_timer_interrupt, 0x8F);

	__asm__ __volatile__ ("lidt %0" : : "m"(idtr));
	__asm__ __volatile__ ("sti");
}

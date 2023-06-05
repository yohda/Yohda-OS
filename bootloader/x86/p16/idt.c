#include "idt.h"

#define IDT_BASE_ADDR 	0x108000 
#define IDT_ENTRY_SIZE	0x10
#define IDT_MAX_ENTRYS 	0x100

extern isr_division_error;

static struct idtr32  idtr;
static struct idt32_entry  __attribute__((aligned(IDT_ENTRY_SIZE))) idt_tbl[IDT_MAX_ENTRYS];

void idt_crt_entry(const int vector, const u32 offset, const u8 attr)
{
	if(vector<0 || vector>IDT_MAX_ENTRYS)
		return -1;

	idt_tbl[vector].loffset = offset;
	idt_tbl[vector].hoffset = offset >> 16;
	idt_tbl[vector].selector = 0x08;
	idt_tbl[vector].attr = attr;
}

void idt_init()
{
	idt_crt_entry(0, isr_division_error, 0x00000000);
}

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "type.h"

enum {
	INT_VEC_DBZ 	= 0x00,
	INT_VEC_DBG,
	INT_VEC_NMI,
	INT_VEC_BRK,
	INT_VEC_OVF,
	INT_VEC_IO 		= 0x06,
	RSVD0 = 0x0C,
	INT_VEC_GPF 	= 0x0D,
	INT_VEC_PF 		= 0x0E,
	RSVD1 = 0x1F,
	INT_VEC_MAX
};

#define IDT_ATTR_TYPE_32INT 	(0x0E)
#define IDT_ATTR_TYPE_32TRAP 	(0x0F)
#define IDT_ATTR_TYPE_16INT		(0x06)
#define IDT_ATTR_TYPE_16TRAP	(0x07)
#define IDT_PRES				(0x80)

struct idtr32 {
	u16 limit;
	u32 base;
}__attribute__((packed));

struct idt32_entry {
	u16 loffset;
	u16 selector;
	u8 rsvd;
	u8 attr;
	u16 hoffset;
}__attribute__((packed));

#endif

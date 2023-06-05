#ifndef _IDT_H_
#define _IDT_H_

#include "type.h"

struct idtr32 {
	u16 size;
	u32 offset;
}__attribute__((packed));

struct idt32_entry {
	u16 loffset;
	u16 selector;
	u8 rsvd;
	u8 attr;
	u16 hoffset;
}__attribute__((packed));

#endif

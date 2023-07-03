#ifndef _PORT_H_
#define _PORT_H_

#include "type.h"

extern u8 inb(const u16 addr);
extern u32 inw(const u16 addr);
extern u32 ind(const u16 addr);

extern void outb(const u16 addr, const u8 val);
extern u16 outw(u16 addr, u16 value);
extern void outd(const u16 addr, const u32 value);

#endif

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "type.h"

/* paging */
#define PAGE_SIZE 4096

/* linker script symbols */
extern const char _virt_base[];
extern const char _kernel32_size[]; 

// interrupt descriptor table register
struct intr_desc_tbl_reg32 {
	uint16_t limit;
	uint32_t base;
}__attribute__((packed));

// interrupt descriptor table entry
struct intr_desc_tbl_entry32 {
	uint16_t loffset;
	uint16_t selector;
	uint8_t rsvd;
	uint8_t attr;
	uint16_t hoffset;
}__attribute__((packed));

struct tss {
	uint16_t	pre_link, rsvd0;
	uint32_t	esp0;
    uint16_t	ss0, rsvd1;
    uint32_t	esp1;
    uint16_t	ss1, rsvd2;
    uint32_t	esp2;
    uint16_t	ss2, rsvd3;
    uint32_t	cr3;
    uint32_t	eip;
    uint32_t	eflags;
    uint32_t	eax, ecx, edx, ebx;
    uint32_t	esp, ebp, esi, edi;
    uint16_t	es, rsvd4;
    uint16_t	cs, rsvd5;
    uint16_t	ss, rsvd6;
    uint16_t	ds, rsvd7;
    uint16_t	fs, rsvd8;
    uint16_t	gs, rsvd9;
    uint16_t	ldt, rsvd10;
    uint16_t	trace, bitmap;	
};

struct tss_desc {
	uint16_t limit;
	uint16_t base;
	uint8_t rsvd;
			
}__attribute__((packed));

#endif

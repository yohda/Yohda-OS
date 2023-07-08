#ifndef _PAGE_H_
#define _PAGE_H_

#include "type.h"

#define P	0x00000001
#define RW 	0x00000002
#define US	0x00000004
#define PWT 0x00000008
#define PCD 0x00000010
#define A	0x00000020
#define D	0x00000040
#define PS	0x00000080 // page directory[7]
#define PAT 0x00000080 // page table[7]
#define G   0x00000100

union pte {
	uint32_t entry;
	struct {
		uint32_t p:1;
		uint32_t rw:1;
		uint32_t us:1;
		uint32_t pwt:1;
		uint32_t pcd:1;
		uint32_t a:1;
		uint32_t d:1;
		uint32_t pat:1;
		uint32_t g:1;
		uint32_t ign:3;
		uint32_t addr:20;
	};
};

union pde {
	uint32_t entry;
	struct {
		uint32_t p:1;
		uint32_t rw:1;
		uint32_t us:1;
		uint32_t pwt:1;
		uint32_t pcd:1;
		uint32_t a:1;
		uint32_t d:1;
		uint32_t ps:1;
		uint32_t g:1;
		uint32_t ign:3;
		uint32_t addr:20;
	};
};

#endif

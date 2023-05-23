#ifndef _MMI_H_
#define _MMI_H_

#include "Types.h"

struct mmif {
	int (*mm_init)(const void *base, const s64 size);
	void (*mm_free)(const void* addr);
	void *(*mm_alloc)(const int size);
	void (*show_info)(void);
	int (*get_chunk)(const int size);
};

int mmi_init(const void *base, const s64 size);
int mmif_register(struct mmif* mif);

#endif

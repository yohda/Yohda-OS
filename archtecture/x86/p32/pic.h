#ifndef _PIC_H_
#define _PIC_H_

#include "type.h"

enum {
	PIC_IRQ0 = 0x01,
	PIC_IRQ1 = 0x02,
	PIC_IRQ2 = 0x04,
	PIC_IRQ3 = 0x08,
	PIC_IRQ_MAX = 0x10,
	PIC_IRQ_INVAL,
};

uint16_t pic_get_imr(void);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);
int pic_init(void);

#endif

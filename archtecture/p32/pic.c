#include "pic.h"

extern void outb(const u16 addr, const u8 val);
extern u8 inb(const u16 addr); 

#define PRI_PIC			0x20		/* IO base address for master PIC */
#define SEC_PRI			0xA0		/* IO base address for slave PIC */
#define PRI_PIC_COMMAND	PRI_PIC
#define PRI_PIC_DATA	(PRI_PIC+1)
#define SEC_PRI_COMMAND	SEC_PRI
#define SEC_PIC_DATA	(SEC_PRI+1)

#define PRI_PIC_REMAP	0x20		/* 0x20 ~ 0x27 */
#define SEC_PIC_REMAP	0x28		/* 0x28 ~ 0x2F */

#define ICW1_ICW4		0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE		0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL		0x08		/* Level triggered (edge) mode */
#define ICW1_INIT		0x10		/* Initialization - required! */
 
#define ICW4_8086		0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO		0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM		0x10		/* Special fully nested (not) */

int pic_init()
{
	u8 a1, a2;
 
	a1 = inb(PRI_PIC_DATA);                        // save masks
	a2 = inb(SEC_PIC_DATA);
 
	outb(PRI_PIC_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(SEC_PRI_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PRI_PIC_DATA, PRI_PIC_REMAP);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(SEC_PIC_DATA, SEC_PIC_REMAP);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PRI_PIC_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(SEC_PIC_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
 
	outb(PRI_PIC_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(SEC_PIC_DATA, ICW4_8086);
	io_wait();
 
	outb(PRI_PIC_DATA, a1);   // restore saved masks.
	outb(SEC_PIC_DATA, a2);	
}

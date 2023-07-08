#include "keyboard.h"
#include "interrupt.h"
#include "debug.h"
#include "pic.h"
#include "port.h"

static const char scan_code_set1[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

void isr_ps2_keyboard_interrupt_handler(const int irq)
{
	uint8_t code = inb(0x60);
	if(code >= 128) {
		pic_eoi(irq);
		return ;
	}

	if(code == BACKSPACE_PRESSED) {
		kprintf("%s\n", "backspace");
	} else if (code == CAPLOCK_PRESSED) {
		kprintf("%s\n", "caplock");
	} else if (code == NUMBERLOCK_PRESSED) {
		kprintf("%s\n", "numberlock");
	}	

	char c = scan_code_set1[code]; 
	kprintf("%c", c);
	pic_eoi(irq);
}

int keyboard_init(void)
{
	uint16_t mask;
	
	mask = pic_get_imr();
	if(mask & PIC_IRQ1) {
		debug("Keyboard Disabled...??\n");
		pic_set_mask(PIC_IRQ1, false);		
	}

	idt_reg_isr(INT_VEC_PS2_KEY, isr_ps2_keyboard_interrupt, 0x8F);
}


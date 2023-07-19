#include "pit.h"
#include "pic.h"
#include "interrupt.h"
#include "port.h"
#include "debug.h"

#define PIT_DEF_HZ		(1193180)
#define PIT_MAX_FREQ 	(65536)
#define PIT_1MSC_FREQ	(1193)

#define PIT_TMR0_DATA	(0x40)
#define PIT_TMR0_CMD	(0x43)

struct pit_dev {
	uint32_t tps;	// ticks per second
	bool inited;
};

struct pit_dev pit;
uint32_t time_ticks;
uint32_t time_secs;

void msleep(const int msec)
{
	uint32_t snap = time_ticks;
	if(msec <= 0)
		return ;

	while(1) {
		if(time_ticks - snap > msec)
			break;
	}
}

void isr_system_timer_handler(const int irq)
{
	time_ticks++;

	if(!pit.tps) {
		pic_eoi(irq);
		return ;
	}

	if(time_ticks % pit.tps == 0) {
		//time_secs++;
		//sched_timer();
		//debug("Passed %d seconds!!\n", time_ticks / pit.tps);
	}
	
	pic_eoi(irq);
}

void pit_set_freq(const int freq)
{
	int div = PIT_DEF_HZ;

	if(freq <= 0 || freq > PIT_MAX_FREQ) {
		debug("PIT default setting#%d\n", pit.tps);
		return -1;
	}
		
	div /= freq;	
	pit.tps = div;
 
	debug("ticks per second#%d\n", pit.tps);
	
	if(freq == PIT_MAX_FREQ)
		div = 0;
	
	outb(PIT_TMR0_CMD, 0x36);	
	outb(PIT_TMR0_DATA, div & 0xFF);
	outb(PIT_TMR0_DATA, div >> 8);	
}

int pit_init()
{
	pit.tps = PIT_DEF_HZ / PIT_MAX_FREQ; // default frequency 

	pit_set_freq(PIT_1MSC_FREQ);
	
	interrupt_register_handler(INT_VEC_SYS_TIMER, isr_system_timer_interrupt, 0x8F);
	
	pit.inited = true;
}

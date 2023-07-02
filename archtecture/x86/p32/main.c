#include "vga.h"
#include "interrupt.h"
#include "cpu.h"
#include "ata.h"
#include "pic.h"
#include "io.h"
#include "mm/mm.h"
#include "block/ahci.h"
#include "debug.h"
#include "multiboot2.h"

extern void load_higher_half(void);

int main();

static int parse_multiboot(unsigned long magic, unsigned long mbi_base)
{
	struct multiboot_tag *tag;
	unsigned int total_size; 	

	if(magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		debug("The boot-loader in this system doesn`t support the Multiboot2#0x%x\n", magic);
		return -1;
	}

	if(mbi_base & 7) {
		debug("Unaligned in 8 bytes: 0x%x\n", mbi_base);
		return -2;
	}

	total_size = *(unsigned int *)mbi_base;
	debug("Multiboot information size#0x%x\n", total_size);
	for(tag=(struct multiboot_tag *)(mbi_base+8); tag->type!=MULTIBOOT_TAG_TYPE_END ; tag=(struct multiboot_tag *)((unsigned char *)tag + ((tag->size+7)&~7))) {

		switch(tag->type) {
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
			break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			break;
			case MULTIBOOT_TAG_TYPE_BOOTDEV:
			break;
			case MULTIBOOT_TAG_TYPE_MMAP:
			{
				struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;
				struct multiboot_mmap_entry *entries = (struct multiboot_mmap_entry *)mmap->entries;
				while((uint8_t *)entries < (uint8_t *)mmap + mmap->size) {
					debug("type#0x%x, addr#0x%x_%x, len#0x%x_%x\n", 
									entries->type,
									(unsigned int)(entries->addr >> 32),
									(unsigned int)(entries->addr & 0xFFFFFFFF),
									(unsigned int)(entries->len >> 32),
									(unsigned int)(entries->len & 0xFFFFFFFF)
								);
					entries = (struct multiboot_mmap_entry *)((unsigned long)entries + mmap->entry_size);
				}
			}
			break;
			default:
				if(tag->type < 0 || tag->type > 21) {
					debug("Not supported mbi#0x%x\n", tag->type);
					return -5;
				}
			break;
		}
		
	}

	return 0;
}

int main(unsigned long magic, unsigned long addr)
{
	
	int i;
	// you must first initialize VGA other than.
	// Without that, there is nothing to see the debug log.
	vga_text_init();
	//vga_text_write("Enter Protected mode\n");
	debug("Enter Protected mode\n");
	
	parse_multiboot(magic, addr);

	cpu_init();
	if(CPUID_64) {
		debug("CPUID#0x%x, X64#0x%x, 1GB#0x%x STD#0x%x EXT#0x%x\n", CPUID, CPUID_64, CPUID_PAGE_1GB, CPUID_STD_FUNCS, CPUID_EXT_FUNCS);
		debug("x86-64 supported\n");	
		mode64();
	} else {
		debug("x86-64 not supported\n");
	}
	/*
	 * PIC remap some interrupts to prevent from hanppening the double fault.
	 * So, PIC was initialized first than interrupt enable.
	 * */
	pic_init();
	interrupt_init();	

	mm_init(0x100000 * 512); // 512MB
	
	pci_init();
	ahci_init();
	//ata_init();

	while(1);
}

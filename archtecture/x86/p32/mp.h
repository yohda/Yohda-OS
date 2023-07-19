#ifndef _MP_H_
#define _MP_H_

#include "type.h"

#define MP_FPS_SIG		 		("_MP_")

#define MP_BDA_BASE_ADDR 		(0x400)
#define MP_BDA_EBDA_ADDR_OFFSET	(0x0E)
#define MP_BDA_MEM_SIZE_OFFSET	(0x13)
#define MP_BIOS_ROM_BASE_ADDR 	(0xF0000)

struct mp_fps {            
  uint8_t sig[4];  		        // "_MP_"
  void *phy_addr;               // phys addr of MP config table
  uint8_t len;                 
  uint8_t rev;                
  uint8_t chk;               	// sum of all bytes must be 0
  uint8_t info1;               	// if this is zero, mp config is present.  
  uint8_t info2;
  uint8_t info3[3];				// reserved - must be zero
}__attribute__((packed));

#define MP_UNITS (sizeof(struct mp_fps))

#define MP_CONF_TAB_SIG "PCMP"

struct mp_conf_tab {
	uint8_t sig[4];           // "PCMP"
  	uint16_t len;                // total table length
 	uint8_t rev;                // [14]
  	uint8_t chk;               // all bytes must add up to 0
  	uint8_t product[20];            // product id
  	uint32_t *oemtable;               // OEM table pointer
  	uint16_t oemlength;             // OEM table length
  	uint16_t ent_cnt;                 // entry count
  	uint32_t *lapic_addr;              // address of local APIC
  	uint16_t ext_len;               // extended table length
  	uint8_t ext_chk;              // extended table checksum
  	uint8_t rsvd;
}__attribute__((packed));

#define MP_ENTRY_SIZE 8

enum {
	MP_ENTRY_PROC = 0,
	MP_ENTRY_BUS = 1,
	MP_ENTRY_IOAPIC = 2,
	MP_ENTRY_IOINTR = 3,
	MP_ENTRY_LOCAL_INTR = 4
};

struct mp_proc_entry {        	 // processor table entry
  uint8_t type;                   // entry type (0)
  uint8_t id;               	  // local APIC id
  uint8_t rev;           	     // local APIC verison
  uint8_t flags;                  // CPU flags
    #define MPBOOT 0x02           // This proc is the bootstrap processor.
  uint32_t sig; 		          // CPU signature
  uint32_t feature;               // feature flags from CPUID instruction
  uint32_t rvsd[2];
}__attribute__((packed));

struct mp_ioapic_entry {       // I/O APIC table entry
  uint8_t type;                   // entry type (2)
  uint8_t id;                 // I/O APIC id
  uint8_t rev;                // I/O APIC version
  uint8_t flags;                  // I/O APIC flags
  uint32_t *addr;                  // I/O APIC address
}__attribute__((packed));

void mp_init(void);

#endif

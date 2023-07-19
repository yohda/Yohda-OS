#include "mp.h"
#include "string.h"
#include "vmm.h"
#include "sys.h"
#include "debug.h"

extern struct cpu_mgr cpus;

static uint8_t mp_checksum(const uint8_t *fps, const int len)
{
	int i, sum = 0;

	for(i=0; i<len; i++)
		sum += fps[i];

	return sum;
}

static int mp_validate(const char *sig, const uint8_t *mp)
{
	int len = 0;
	if(strcmp(mp, sig))
		return -1;

	// If above codes are validated, below codes also are validated because already above validation was done. 
	len = (sig[0] == 'P') ? *(uint16_t *)&mp[4] : (*(uint8_t *)&mp[8])*MP_UNITS;

	if(mp_checksum(mp, len))
		return -2;

	return 0;
}

// Look for an MP structure in the len bytes at addr.
static struct mp_fps* mp_parse(uint32_t base, int size)
{
  uint8_t *end, *p, *fps;

  fps = vmm_phy_to_virt(base);
  end = fps + size;
  for(p = fps; p < end; p += sizeof(struct mp_fps))
    if(!mp_validate(MP_FPS_SIG, p))
      return (struct mp_fps*)p;

  return NULL;
}

// Search for the MP Floating Pointer Structure, which according to the
// spec is in one of the following three locations:
static struct mp_fps* mp_search(void)
{
  	uint16_t *bios_bda;
	uint8_t *bda;
  	uint32_t p;
  	struct mp_fps *mp;
		
	// 1) in the first KB of the Ebios_bda;
	p = (vmm_get_uint16(MP_BDA_BASE_ADDR + MP_BDA_EBDA_ADDR_OFFSET) << 4);
  	if((mp = mp_parse(p, 1024)))
  		return mp;
   
	// 2) in the last KB of system base memory;
  	p = (vmm_get_uint16(MP_BDA_BASE_ADDR + MP_BDA_MEM_SIZE_OFFSET) * 1024);
  	if((mp = mp_parse(p-1024, 1024)))
		return mp;

	// 3) in the BIOS ROM between 0xF0000 and 0xFFFFF.
  	return mp_parse(MP_BIOS_ROM_BASE_ADDR, 0x10000);
}

// Search for an MP configuration table.  For now,
// don't accept the default configurations (physaddr == 0).
// Check for correct signature, calculate the checksum and,
// if correct, check the version.
// To do: check extended table checksum.
static struct mp_conf_table* mp_config(struct mp **pmp)
{
  struct mp_conf_tab *conf;
  struct mp_fps *mp_fps;

  if((mp_fps = mp_search()) == 0 || mp_fps->phy_addr == 0)
    return NULL;

  conf = (struct mp_conf_tab *) vmm_phy_to_virt((uint32_t)mp_fps->phy_addr);

  if(mp_validate(MP_CONF_TAB_SIG, (uint8_t *)conf))
    return NULL;
 
  *pmp = mp_fps;
  return conf;
}

void mp_init(void)
{
  uint8_t *p, *e;
  int i, ismp, ent_cnt;
  struct mp_fps *mp_fps;
  struct mp_conf_tab *mp_conf;
  struct mp_proc_entry *mp_proc;
  struct mp_ioapic_entry *mp_ioapic;

  if((mp_conf = mp_config(&mp_fps)) == 0)
	err_dbg(-1, "panic\n");

  cpus.lapic_addr = (uint32_t*)mp_conf->lapic_addr;
  ent_cnt = mp_conf->ent_cnt;
  for(p=(uint8_t*)(mp_conf+1), i=0; i<ent_cnt; i++){
    switch(*p){
    case MP_ENTRY_PROC:
      mp_proc = (struct mp_proc*)p;
      if(cpus.ncpu < SYS_MAX_CPU) {
        cpus.cpu[cpus.ncpu].id = mp_proc->id;  // apicid may differ from ncpu
        cpus.ncpu++;
      }

      p += sizeof(struct mp_proc_entry);
      
	  continue;
    case MP_ENTRY_IOAPIC:
	  // it assumes that in this system, there is only one io apic.
      cpus.ioapic = (struct mp_ioapic_entry *)p;
      p += sizeof(struct mp_ioapic_entry);

      continue;
    case MP_ENTRY_BUS:
    case MP_ENTRY_IOINTR:
    case MP_ENTRY_LOCAL_INTR:
      p += MP_ENTRY_SIZE;

      continue;
    default:
	  err_dbg(-2, "panic\n");
      break;
    }
  }

  if(mp_fps->info2){
    // Bochs doesn't support IMCR, so this doesn't run on Bochs.
    // But it would on real hardware.
    outb(0x22, 0x70);   // Select IMCR
    outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
  }

}


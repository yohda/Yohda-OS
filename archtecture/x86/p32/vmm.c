#include "vmm.h"
#include "string.h"
#include "sys.h"

#define VMM_DIR_OFFSET (22)
#define VMM_TBL_OFFSET (12)

struct vmm {
	uint32_t base;
	uint8_t inited;
};

struct vmm vmm;

uint32_t vmm_get_tbl_offset(const uint32_t virt_addr)
{
	return virt_addr >> VMM_TBL_OFFSET;
}

uint32_t vmm_get_dir_offset(const uint32_t virt_addr)
{
	return virt_addr >> VMM_DIR_OFFSET;
}

void *vmm_virt_to_phy(const uint32_t virt)
{
	return (void *)(virt - vmm.base);
}

void *vmm_phy_to_virt(const uint32_t phy)
{
	return (void *)(phy + vmm.base);
}

int vmm_init(void)
{
	if(vmm.inited)
		return 0;

	vmm.base = (uint32_t)&_virt_base;	

	vmm.inited = true;
}

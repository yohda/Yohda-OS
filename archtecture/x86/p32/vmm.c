#include "vmm.h"
#include "string.h"

extern char _virt_base[];
struct vmm {
	uint32_t base;
};

struct vmm vmm;

void *vmm_virt_to_phy(const void *virt)
{
	

}

void *vmm_phy_to_virt(const void *phy)
{
	uint32_t _phy = (uint32_t)phy;
	_phy += vmm.base;
	
	return (void *)_phy;
}

int vmm_init(void)
{
	vmm.base = (uint32_t)&_virt_base;	
}

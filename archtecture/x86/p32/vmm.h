#ifndef _VMM_H_
#define _VMM_H_

#include "type.h"
void *vmm_virt_to_phy(const uint32_t virt);
void *vmm_phy_to_virt(const uint32_t phy);
int vmm_init(void);

#endif

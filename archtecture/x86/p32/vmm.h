#ifndef _VMM_H_
#define _VMM_H_

#include "type.h"

void *vmm_virt_to_phy(const void *virt);
void *vmm_phy_to_virt(const void *phy);
int vmm_init(void);

#endif

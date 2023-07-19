#ifndef _VMM_H_
#define _VMM_H_

#include "type.h"
void *vmm_virt_to_phy(const uint32_t virt);
void *vmm_phy_to_virt(const uint32_t phy);
int vmm_init(void);

uint32_t vmm_get_uint32(const uint32_t phy);
uint16_t vmm_get_uint16(const uint32_t phy);
uint8_t vmm_get_uint8(const uint32_t phy);

#endif

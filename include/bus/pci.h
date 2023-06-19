#ifndef _PCI_H_
#define _PCI_H_

#include "Types.h"

#define PCI_HEADER_TYPE_DEVICE	0x00
#define PCI_HEADER_TYPE_BRIDGE	0x01
#define PCI_HEADER_TYPE_CARD 	0x02

// Size : 16B
struct pci_conf_pre_def_header {
	u16 vendor_id;
	u16 device_id;
	
	u16 command;
	u16 status;
	
	u32 revision_id:8;
	u32 class_code:24;

	u8 cacheline_size;
	u8 latency_timer;
	u8 header_type;
	u8 bist;	
};

struct pci_conf_header_type0 {
	struct pci_conf_pre_def_header pre_def_hedaer;
};

struct pci_conf_header_type1 {
	struct pci_conf_pre_def_header pre_def_header
}; 

struct pci_conf_device_specific {

};

struct pci_configration_space {
	struct pci_conf_device_specific dev_regs;
};


#endif

#ifndef _PCI_H_
#define _PCI_H_

#include "type.h"
#include "list.h"

#define PCI_HEADER_TYPE_DEVICE	0x00
#define PCI_HEADER_TYPE_BRIDGE	0x01
#define PCI_HEADER_TYPE_CARD 	0x02

#define PCI_BUS_MAX 	(256)
#define PCI_DEV_MAX		(32)
#define PCI_FUNC_MAX	(8)

#define PCI_CONFIG_ADDR		(0xCF8)
#define PCI_CONFIG_DATA		(0xCFC)

#define PCI_CFG_ADDR_ALIGN	(0xFC)
#define PCI_CFG_OFFSET_VEN	(0x00)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_DEV	(0x02)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_CLS	(0x09)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_HDR 	(0x0E)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_BAR0	(0x10)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_BAR1	(0x14)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_BAR2	(0x18)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_BAR3	(0x1C)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_BAR4	(0x20)&(PCI_CFG_ADDR_ALIGN)
#define PCI_CFG_OFFSET_BAR5	(0x24)&(PCI_CFG_ADDR_ALIGN)

struct pci_sys {
	struct list_node buses;
};

struct pci_bus {
	struct list_node devs;
};

struct pci_dev {
	struct list_node funcs;
	u16 bus;
	u16 slot;
	u16 ven_id;
	u16 dev_id;
};

struct pci_func {
	struct list_node node;
	u8 bus;
	u8 dev;
	u8 func;
	u8 cls;
	u8 sub;
	u8 pif;
};

struct pci_func pci_find_dev(const u16 vid, const u16 did);

#endif

#include "bus/pci.h"
#include "string.h"

extern void outd(const u16 addr, const u32 value);
extern u32 inw(const u16 addr);

#define PCI_BUS_MAX 	(256)
#define PCI_DEV_MAX		(32)
#define PCI_FUNC_MAX	(8)

#define PCI_CONFIG_ADDR	(0xCF8)
#define PCI_CONFIG_DATA	(0xCFC)

#define PCI_GET_CONFIG_ADDR(x,y,z) ((0x80000000)|((x)<<(16))|((y)<<(11))|((z)<<(8)))

#define PCI_NO_DEVICE	(0xFFFF)
#define PCI_MULTI_FUNC 	(0x80)
#define PCI_ADDR_ALIGN	(0xFC)

/* PCI Configuration Offset */
#define PCI_OFFSET_VEN	(0x00)&(PCI_ADDR_ALIGN)
#define PCI_OFFSET_DEV	(0x02)&(PCI_ADDR_ALIGN)
#define PCI_OFFSET_HDR 	(0x0E)&(PCI_ADDR_ALIGN)

struct pci_dev {
	u32 bus;
	u32 dev;
	u32 func;
};

struct pci_dev pci[255];

u16 pci_get_dev(const struct pci_dev *dev)
{
	u32 addr = 0;

	addr = PCI_GET_CONFIG_ADDR(dev->bus, dev->dev, dev->func); 
	addr |= PCI_OFFSET_DEV;

	outd(PCI_CONFIG_ADDR, addr);	

	return (u16)((inw(PCI_CONFIG_DATA) >> 16) & 0xFF);
}

u16 pci_get_ven(const struct pci_dev *dev)
{
	u32 addr = 0;
	
	addr = PCI_GET_CONFIG_ADDR(dev->bus, dev->dev, dev->func); 
	addr |= PCI_OFFSET_VEN;

	outd(PCI_CONFIG_ADDR, addr);	

	return (u16)inw(PCI_CONFIG_DATA);
}

u8 pci_get_hdr(const struct pci_dev *dev)
{
	u32 addr = 0;
	
	addr = PCI_GET_CONFIG_ADDR(dev->bus, dev->dev, dev->func); 
	addr |= PCI_OFFSET_HDR;

	outd(PCI_CONFIG_ADDR, addr);	

	return (u8)((inw(PCI_CONFIG_DATA) >> 16) & 0xFF);
}

/*
u8 pci_find_dev(const u8 bus, const u8 slot) {
    u16 vendor, device;
    if ((vendor = pciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
       device = pciConfigReadWord(bus, slot, 0, 2);
       . . .
    } return (vendor);
}
*/

static void pci_enum_func(const u8 bus, const u8 dev, const u8 func)
{
	return 0;
}

static void pci_enum_dev(const u8 bus, const u8 dev)
{
	u16 ven = PCI_NO_DEVICE, hdr = PCI_MULTI_FUNC;
	struct pci_dev pdev;
	int i;

	memset(&pdev, 0, sizeof(pdev));	

	pdev.bus = bus;
	pdev.dev = dev;

	ven = pci_get_ven(&pdev);
	if(ven == PCI_NO_DEVICE)
			return ;
		
	hdr = pci_get_hdr(&pdev);
	if(!(hdr & PCI_MULTI_FUNC))
		return ;

	for(i=1; i<PCI_FUNC_MAX; i++) {
		pci_enum_func(bus, dev, i);
	}
}

static void pci_enumerate()
{
	int i, j;

	for(i=0; i<PCI_BUS_MAX; i++) {
		for(j=0; j<PCI_DEV_MAX; j++) {
			pci_enum_dev(i, j);
		}
	}	
}

void pci_init()
{
	pci_enumerate();
}

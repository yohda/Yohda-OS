#include "bus/pci.h"
#include "string.h"
#include "mm/mmi.h"

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
#define PCI_OFFSET_CLS	(0x09)&(PCI_ADDR_ALIGN)
#define PCI_OFFSET_HDR 	(0x0E)&(PCI_ADDR_ALIGN)

#define PCI_CLS(x) (((x)>>(16))&(0xFF))
#define PCI_SUB(x) (((x)>>(8))&(0xFF))
#define PCI_PIF(x) ((x)&(0xFF))

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
	u16 ret;

	addr = PCI_GET_CONFIG_ADDR(dev->bus, dev->dev, dev->func); 
	addr |= PCI_OFFSET_VEN;

	outd(PCI_CONFIG_ADDR, addr);	
	
	ret = (u16)inw(PCI_CONFIG_DATA);
	return ret;
}

u8 pci_get_hdr(const struct pci_dev *dev)
{
	u32 addr = 0;
	u8 ret;

	addr = PCI_GET_CONFIG_ADDR(dev->bus, dev->dev, dev->func); 
	addr |= PCI_OFFSET_HDR;

	outd(PCI_CONFIG_ADDR, addr);	

	ret = (u8)((inw(PCI_CONFIG_DATA) >> 16) & 0xFF);
	return ret;
}

u32 pci_get_cls(const struct pci_dev *dev)
{
	u32 addr = 0;
	u32 ret;

	addr = PCI_GET_CONFIG_ADDR(dev->bus, dev->dev, dev->func); 
	addr |= PCI_OFFSET_CLS;

	outd(PCI_CONFIG_ADDR, addr);	
	
	ret = (u32)((inw(PCI_CONFIG_DATA) >> 8) & 0x00FFFFFF);
	return ret;
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
	u8 cls = 0, sub = 0, pif = 0;
	struct pci_dev pdev;
	u32 class;
	
	pdev.bus = bus;
	pdev.dev = dev;
	pdev.func = func;

	class = pci_get_cls(&pdev);
	
	pif = (u8)PCI_PIF(class);
	sub = (u8)PCI_SUB(class);
	cls = (u8)PCI_CLS(class);	
}

static void pci_enum_dev(const u8 bus, const u8 dev)
{
	u16 ven = PCI_NO_DEVICE, hdr = PCI_MULTI_FUNC;
	struct pci_dev pdev;
	struct pci_dev *evf;
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

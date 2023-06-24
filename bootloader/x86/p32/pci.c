#include "bus/pci.h"
#include "string.h"
#include "mm/mmi.h"
#include "debug.h"
#include "list.h"
#include "block/ahci.h"
#include "ata.h"

extern void outd(const u16 addr, const u32 value);
extern u32 inw(const u16 addr);
extern u32 ind(const u16 addr);

#define PCI_GET_CONFIG_ADDR(x,y,z) ((0x80000000)|((x)<<(16))|((y)<<(11))|((z)<<(8)))

#define PCI_NO_DEVICE	(0xFFFF)
#define PCI_MULTI_FUNC 	(0x80)

/* PCI Configuration Offset */

#define PCI_CLS(x) (((x)>>(16))&(0xFF))
#define PCI_SUB(x) (((x)>>(8))&(0xFF))
#define PCI_PIF(x) ((x)&(0xFF))

struct pci_sys pci;

u16 pci_get_dev(const u8 bus, const u8 dev, const u8 func)
{
	u32 addr = 0;

	addr = PCI_GET_CONFIG_ADDR(bus, dev, func); 
	addr |= PCI_CFG_OFFSET_DEV;

	outd(PCI_CONFIG_ADDR, addr);	

	return (u16)((ind(PCI_CONFIG_DATA) >> 16) & 0xFFFF);
}

u16 pci_get_ven(const u8 bus, const u8 dev, const u8 func)
{
	u32 addr = 0;
	u16 ret;

	addr = PCI_GET_CONFIG_ADDR(bus, dev, func); 
	addr |= PCI_CFG_OFFSET_VEN;

	outd(PCI_CONFIG_ADDR, addr);	
	
	ret = (u16)inw(PCI_CONFIG_DATA);
	return ret;
}

u8 pci_get_hdr(const u8 bus, const u8 dev, const u8 func)
{
	u32 addr = 0;
	u8 ret;

	addr = PCI_GET_CONFIG_ADDR(bus, dev, func); 
	addr |= PCI_CFG_OFFSET_HDR;

	outd(PCI_CONFIG_ADDR, addr);	

	ret = (u8)((ind(PCI_CONFIG_DATA) >> 16) & 0xFF);
	return ret;
}


u32 pci_get_cls(const u8 bus, const u8 dev, const u8 func)
{
	u32 addr = 0;
	u32 ret;

	addr = PCI_GET_CONFIG_ADDR(bus, dev, func); 
	addr |= PCI_CFG_OFFSET_CLS;

	outd(PCI_CONFIG_ADDR, addr);	
	
	ret = (u32)((ind(PCI_CONFIG_DATA) >> 8) & 0x00FFFFFF);
	return ret;
}

u32 pci_get_bar(const u8 bus, const u8 dev, const u8 func, const u8 bar)
{
	u32 addr = 0;
	u32 ret;

	addr = PCI_GET_CONFIG_ADDR(bus, dev, func); 
	addr |= bar;

	outd(PCI_CONFIG_ADDR, addr);	
	
	ret = (u32)ind(PCI_CONFIG_DATA);
	return ret;
}

struct pci_func pci_find_dev(const u16 vid, const u16 did) 
{

}

static void pci_enum_func(const u8 bus, const u8 dev, const u8 func, struct pci_func *node)
{
	u8 cls = 0, sub = 0, pif = 0;
	u32 class;

	class = pci_get_cls(bus, dev, func);

	node->func = func;	
	node->pif = (u8)PCI_PIF(class);
	node->sub = (u8)PCI_SUB(class);
	node->cls = (u8)PCI_CLS(class);	
}

static void pci_enum_dev(const u8 bus, const u8 slot)
{
	u16 ven = PCI_NO_DEVICE, hdr = PCI_MULTI_FUNC, dev = 0;
	struct pci_dev *pdev = NULL;
	struct pci_func *pfunc = NULL;
	int i;

	ven = pci_get_ven(bus, slot, 0);
	if(ven == PCI_NO_DEVICE)
			return ;

	pdev = yalloc(sizeof(struct pci_dev));
	if(!pdev)
		err_dbg(-1, "failed to create pci device\n");

	list_init_head(&pdev->funcs);

	pdev->bus = bus;
	pdev->slot = slot;
	pdev->ven_id = ven;

	dev = pci_get_dev(bus, slot, 0);	
	hdr = pci_get_hdr(bus, slot, 0);

	pdev->dev_id = dev;

	pfunc = yalloc(sizeof(struct pci_func));
	if(!pfunc)
		err_dbg(-1, "failed to create pci function\n");

	pci_enum_func(bus, slot, 0, pfunc);
	list_add(&pdev->funcs, &pfunc->node);

	debug("bus#%d, dev#%x, vender#0x%x\n", bus, dev, ven);
	if(!(hdr & PCI_MULTI_FUNC))
		return ;

	for(i=1; i<PCI_FUNC_MAX; i++) {
		pci_enum_func(bus, slot, i, NULL); // Not yet, yohdaOS doesn`t support pci device supporting multiple-fucions devic
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
	list_init_head(&pci.buses);
	
	pci_enumerate();
}

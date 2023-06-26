#include "ata.h"
#include "string.h"

extern u8 outb(u16 addr, u8 value);
extern u16 outw(u16 addr, u16 value);
extern u32 outd(u16 addr, u32 value);
extern u8 inb(u16 addr);

// Macro for each bit in registers
#define ATA_DEV_CTRL_RST	(1<<2)

#define ATA_STAT_BSY		(1<<7)
#define ATA_STAT_RDY		(1<<6)

#define ATA_DEV_DEV			(1<<4)

// for conveinece, define macro function
#define ATA_IS_BSY(x) 		((x)&ATA_STAT_BSY)
#define ATA_IS_RDY(x)		((x)&ATA_STAT_RDY)

struct ata_mgmr {
	u8 disk[2];
	struct ata_dev dev[4];
	//struct ata_dev *dev;
};

struct ata_mgmr ata;

int ata_sw_rst(void)
{	
	outb(ATA_REG_DEV_CTRL(PRI_CTRL_BASE), ATA_DEV_CTRL_RST);	
	ata_is_ready();

	outb(ATA_REG_DEV(PRI_CMD_BASE), (1<<6));
}

int ata_diag_disk(void)
{
	u8 err = -1, lm = 0, lh = 0;
	int i;
	
	if(ata_is_ready() < 0)
		return -1;		

	outb(ATA_REG_CMD(PRI_CMD_BASE), ATA_EXE_DIAG);
	for(i=0;i<30000;i++);
			
	if(ata_is_ready() < 0)
		return -1;	

	err = inb(ATA_REG_ERR(PRI_CMD_BASE));
	if(err & ATA_DISK_EXIST)
		ata.disk[0] = 1;		
	
	err = inb(ATA_REG_ERR(SEC_CMD_BASE));
	if(err & ATA_DISK_EXIST)
		ata.disk[1] = 1;

	if(!ata.disk[0] && !ata.disk[1])
		return -1;

	lm = inb(ATA_REG_LBA_MID(PRI_CMD_BASE)); 
	lh = inb(ATA_REG_LBA_HIGH(PRI_CMD_BASE));	

	return -1;
}

int ata_is_ready()
{
	u8 status = 0;
	int i = 0;

	while(1) {
		status = inb(ATA_REG_STAT(PRI_CMD_BASE));
		if(!ATA_IS_BSY(status) && !ATA_IS_RDY(status)) {
			return 1;		
		}
	}

	return -1;
}

int ata_init()
{
	//ata_sw_rst();
	memset(&ata, 0, sizeof(ata));
	ata_diag_disk();
}

static int ata_write(u3)
{

}

int ata_write_cmd()
{

}

int ata_write_data()
{

}

int ata_read()
{

}

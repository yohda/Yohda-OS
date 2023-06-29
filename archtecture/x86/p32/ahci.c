#include "block/ahci.h"
//#include "Utility.h"
#include "error.h"
#include "bus/pci.h"
#include "fs/fat.h"
#include "debug.h"
#include "string.h"

#define	SATA_SIG_ATA		0x00000101	// SATA drive
#define	SATA_SIG_ATAPI		0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB		0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM			0x96690101	// Port multiplier
 
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4
 
#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

/* AHCI Port Memory Region */
#define	AHCI_BASE							0x10000000	// 256MB

#define AHCI_CMD_LIST_BASE_ADDR				AHCI_BASE
#define AHCI_CMD_HEADER_SIZE				(32)
#define AHCI_CMD_HEADER_NUM_PER_CML_LIST	(32)
#define AHCI_CMD_LIST_SIZE					(AHCI_CMD_HEADER_SIZE*AHCI_CMD_HEADER_NUM_PER_CML_LIST)
#define AHCI_CMD_LIST_NUM_PER_PORT			(32)
#define AHCI_MAX_CMD_LIST_SIZE				(AHCI_CMD_LIST_SIZE*AHCI_CMD_LIST_NUM_PER_PORT) 		// 32KB

#define AHCI_RECV_FIS_BASE_ADDR				(AHCI_CMD_LIST_BASE_ADDR+AHCI_MAX_CMD_LIST_SIZE)	
#define AHCI_RECV_FIS_SIZE					(256)
#define AHCI_RECV_FIS_NUM_PER_PORT			(32)
#define AHCI_MAX_RECV_FIS_SIZE				(AHCI_RECV_FIS_SIZE*AHCI_RECV_FIS_NUM_PER_PORT) 		// 8KB

#define AHCI_CMD_TABLE_BASE_ADDR			(AHCI_RECV_FIS_BASE_ADDR+AHCI_MAX_RECV_FIS_SIZE)
#define AHCI_CMD_TABLE_SIZE					(256)
#define AHCI_CMD_TABLE_NUM_PER_CMD_HEADER	(32)
#define AHCI_CMD_TABLE_NUM_PER_PORT			(32)
#define AHCI_MAX_CMD_TABLE_SIZE 			(AHCI_CMD_TABLE_SIZE*AHCI_CMD_TABLE_NUM_PER_CMD_HEADER*AHCI_CMD_TABLE_NUM_PER_PORT) 	// 256KB

#define AHCI_PORT_MAX_MEMORY_SIZE 			(AHCI_MAX_CMD_LIST_SIZE+AHCI_MAX_RECV_FIS_SIZE+AHCI_MAX_CMD_TABLE_SIZE)

/*  */ 
#define HBA_PxCMD_ST    		0x0001
#define HBA_PxCMD_FRE   		0x0010
#define HBA_PxCMD_FR    		0x4000
#define HBA_PxCMD_CR    		0x8000
#define HBA_PxIS_TFES   		(1 << 30)       /* TFES - Task File Error Status */

/*  */
#define ATA_CMD_READ_DMA_EX 	0x25
#define ATA_CMD_WRITE_DMA_EX	0x35

#define AHCI_RECEVIED_FIS_SIZE  256  // unit byte and per port,

/* File System Data Region */
#define AHCI_FILE_SYSTEM_DATA_BASE 0x20000000
struct ahci_manager {
	int init;
	u16 active_port;		
};

struct ahci_manager mgr;
struct ahci_hba_mem_reg *abar;
struct fat_directory_entr {
	u8 name[11];
	u8 attr;
	u8 reserved; 					// must be 0
	u8 create_time_tenth;
	u16 create_time;
	u16 create_date;
	u16 last_access_date;
	u16 start_clu_num_h;
	u16 last_mod_time;
	u16 last_mode_date;
	u16 start_clu_num_l;
	u32 file_size;
}__attribute__((packed));
struct fat_directory_entr yohda;

int ahci_init(void)
{
	// Search disk in implemented ports
	struct ahci_hba_mem_reg *tmp;
	uint32_t pi;
	int i = 0, type, err;
	u64 *buf = (u64 *)AHCI_PORT_MAX_MEMORY_SIZE;
	//u32 *buf = (u32 *)AHCI_PORT_MAX_MEMORY_SIZE;
	//u16 *buf = (u16 *)AHCI_PORT_MAX_MEMORY_SIZE;
	u32 *read_buf = (u32 *)AHCI_FILE_SYSTEM_DATA_BASE;
	
	err = find_ahci_device(&tmp);
	if(err) {
		ahci_debug("failed to find ahci devie on, err#%d\n", err);
		return err; 
	}

	mgr.init = err;
	abar = tmp;	

	ahci_debug("abar#0x%x\n", abar);
	pi = abar->ghc.pi;
	while (i < AHCI_MAX_PORT_NUM)
	{
		if (pi & 1)
		{
			type = ahci_check_type(&abar->port[i]);
			switch(type)
			{
				case AHCI_DEV_SATA:
					ahci_debug("SATA drive found at port %d\n", i);
					ahci_port_rebase(&abar->port[i], i);
					mgr.active_port = i;
				break;
				case AHCI_DEV_SATAPI:
					ahci_debug("SATAPI drive found at port %d\n", i);
				break;
				case AHCI_DEV_SEMB:
					ahci_debug("SEMB drive found at port %d\n", i);
				break;
				case AHCI_DEV_PM:
					ahci_debug("PM drive found at port %d\n", i);
				break;
				default:
					//ahci_debug("No drive found at port %d\n", i);
				break;
			}
		}
 
		pi >>= 1;
		i ++;
	}
	
	return 0;
}
 
// Check device type
int ahci_check_type(struct ahci_hba_port *port)
{
	uint32_t ssts = port->ssts;
 
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;
 
	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	ahci_debug("sig : 0x%x\n", port->sig);
	switch (port->sig)
	{
		case SATA_SIG_ATAPI:
			return AHCI_DEV_SATAPI;
		case SATA_SIG_SEMB:
			return AHCI_DEV_SEMB;
		case SATA_SIG_PM:
			return AHCI_DEV_PM;
		default:
			return AHCI_DEV_SATA;
	}
}
 
void ahci_port_rebase(struct ahci_hba_port *port, int portno)
{
	int i;
	stop_cmd(port);	// Stop command engine

	ahci_debug("port[%d] - 0x%x 0x%x\n", portno, AHCI_CMD_LIST_BASE_ADDR, AHCI_MAX_CMD_LIST_SIZE);
	ahci_debug("port[%d] - 0x%x 0x%x\n", portno, AHCI_RECV_FIS_BASE_ADDR, AHCI_MAX_RECV_FIS_SIZE);
	ahci_debug("port[%d] - 0x%x 0x%x\n", portno, AHCI_CMD_TABLE_BASE_ADDR, AHCI_MAX_CMD_TABLE_SIZE);
	ahci_debug("port[%d] - 0x%x\n", portno, AHCI_PORT_MAX_MEMORY_SIZE);

	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, AHCI_CMD_LIST_SIZE);
 
	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (AHCI_MAX_PORT_NUM << 10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, AHCI_RECEVIED_FIS_SIZE);
 
	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	struct ahci_hba_cmd_header *cmd_header = (struct ahci_hba_cmd_header*)(port->clb);
	for (i=0; i < AHCI_MAX_CMD_HEADER_NUM; i++)
	{
		cmd_header[i].prdtl = 8;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmd_header_index*256
		cmd_header[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmd_header[i].ctbau = 0;
		memset((void*)cmd_header[i].ctba, 0, 256);
	}
 
	start_cmd(port);	// Start command engine

	ahci_debug("port[%d] is initialized\n", portno);
}
 
// Start command engine
void start_cmd(struct ahci_hba_port *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stop_cmd(struct ahci_hba_port *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

int _ahci_read(struct ahci_hba_port *port, uint64_t start_lba, uint32_t count, void *buf)  
{
    int spin = 0;           	// Spin lock timeout counter
	int i;
	uint64_t addr;
    int slot;
	struct ahci_hba_cmd_header *cmd_header;
	struct ahci_hba_cmd_tbl *cmd_tbl;
  
    port->is = (uint32_t)-1;    // Clear pending interrupt bits

	slot = ahci_find_empty_cmd_slot(port);
	if (slot < 0)
    	return slot;

	addr = (((port->clbu) << 32) | port->clb);
    cmd_header = (struct ahci_hba_cmd_header*)addr;
 
    cmd_header += slot;
    cmd_header->cfl = sizeof(struct ahci_hba_fis_reg_h2d)/sizeof(uint32_t);     // Command FIS size
    cmd_header->w = 0;               // Read from device
        // 8K bytes (16 sectors) per PRDT
    cmd_header->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count
    
    cmd_tbl = (struct ahci_hba_cmd_tbl*)(((cmd_header->ctbau) << 32) | cmd_header->ctba);
    memset(cmd_tbl, 0, sizeof(struct ahci_hba_cmd_tbl) + (cmd_header->prdtl-1)*sizeof(struct ahci_hba_prdt_entry));
        // 8K bytes (16 sectors) per PRDT
    for (i=0; i<cmd_header->prdtl-1; i++)
    {
    	cmd_tbl->prdt_entry[i].dba = (u32)buf;
    	cmd_tbl->prdt_entry[i].dbau = 0;
    	cmd_tbl->prdt_entry[i].dbc = 8*1024-1;     // 8K bytes
    	cmd_tbl->prdt_entry[i].i = 1;
    	buf += 2 * 1024;  	// size of buf is 64B and PTR
    	count -= 16;    // 16 sectors = 8KB
    }
        /**If the final Data FIS transfer in a command is for an odd number of 16-bit words, the transmitter�s
Transport layer is responsible for padding the final Dword of a FIS with zeros. If the HBA receives one
more word than is indicated in the PRD table due to this padding requirement, the HBA shall not signal
this as an overflow condition. In addition, if the HBA inserts padding as required in a FIS it is transmitting,
an overflow error shall not be indicated. The PRD Byte Count field shall be updated based on the
number of words specified in the PRD table, ignoring any additional padding.**/
       
	//ahci_debug("1count#%d\n", count);

    // Last entry
    cmd_tbl->prdt_entry[i].dba = (u32)buf;
    cmd_tbl->prdt_entry[i].dbau = 0;
    cmd_tbl->prdt_entry[i].dbc = (count<<9)-1;   // 512 bytes per sector
    cmd_tbl->prdt_entry[i].i = 1;

	//ahci_debug("2count#%d\n", (count << 9));
    // Setup command
    struct ahci_hba_fis_reg_h2d *cmd_fis = (struct ahci_hba_fis_reg_h2d *)(&cmd_tbl->cfis);
 
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1;  // Command
    cmd_fis->device = 1 << 6;  // LBA mode
    cmd_fis->command = ATA_CMD_READ_DMA_EX; // this value is described in `ATA Specification`, not SATA Spec.
 
	cmd_fis->lba0 = (uint8_t)start_lba;		
    cmd_fis->lba1 = (uint8_t)(start_lba>>8);
    cmd_fis->lba2 = (uint8_t)(start_lba>>16);
    cmd_fis->lba3 = (uint8_t)(start_lba>>24);
    cmd_fis->lba4 = (uint8_t)(start_lba>>32);
    cmd_fis->lba5 = (uint8_t)(start_lba>>40);
 
    cmd_fis->countl = (uint8_t)(count & 0xff);
    cmd_fis->counth = (uint8_t)(count >> 8);
 
		// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		ahci_debug("Port is hung\n");
		return false;
	}

    port->ci = 1 << slot;    // Issue command
    // Wait for completion
    while (1)
    {
    	// In some longer duration reads, it may be helpful to spin on the DPS bit 
   		// in the PxIS port field as well (1 << 5)
    	if ((port->ci & (1<<slot)) == 0) 
    		break;
       
		if (port->is & HBA_PxIS_TFES)   // Task file error
		{
			ahci_debug("Task file error\n");
    		return 0;
		}
	}    

	// Check again
	 if (port->is & HBA_PxIS_TFES)
     {
     	return 0;
     }

	return 0;
}

int ahci_read(uint64_t start_lba, uint32_t count, uint64_t *buf)  
{
	if(start_lba < 0 && count < 1)
		return -EINVAL;

	_ahci_read(&abar->port[mgr.active_port], start_lba, count, buf);
}

int ahci_read_sector(uint64_t start_lba, uint64_t *buf)  
{
	if(start_lba < 0)
		return -EINVAL;

	_ahci_read(&abar->port[mgr.active_port], start_lba, 1, buf);
}

/* it`s possible to use 64-bit buf. But, it makes to be difficult to print hex because a print format supports only 32-bit hex. So, i use the 32-bit buf */
int _ahci_write(struct ahci_hba_port *port, uint64_t start_lba, uint32_t count, void *buf)
{
    int spin = 0;           	// Spin lock timeout counter
	int i;
	uint64_t addr;
    int slot;
	struct ahci_hba_cmd_header *cmd_header;
	struct ahci_hba_cmd_tbl *cmd_tbl;
  
    port->is = (uint32_t)-1;    // Clear pending interrupt bits

	slot = ahci_find_empty_cmd_slot(port);
	if (slot < 0)
    	return slot;

	addr = (((port->clbu) << 32) | port->clb);
    cmd_header = (struct ahci_hba_cmd_header*)addr;
 
    cmd_header += slot;
    cmd_header->cfl = sizeof(struct ahci_hba_fis_reg_h2d)/sizeof(uint32_t); 
    cmd_header->w = 1;               // Write to device
    cmd_header->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count
    
    cmd_tbl = (struct ahci_hba_cmd_tbl*)(((cmd_header->ctbau) << 32) | cmd_header->ctba);
    memset(cmd_tbl, 0, sizeof(struct ahci_hba_cmd_tbl) +
        (cmd_header->prdtl-1)*sizeof(struct ahci_hba_prdt_entry));

    // 8K bytes (16 sectors) per PRDT
    for (i=0; i<cmd_header->prdtl-1; i++)
    {
        cmd_tbl->prdt_entry[i].dba = (u32)buf;
        cmd_tbl->prdt_entry[i].dbau = 0; 
        cmd_tbl->prdt_entry[i].dbc = 8*1024-1; // 8K bytes
        cmd_tbl->prdt_entry[i].i = 1;
        buf += 2 * 1024;  	// 
        count -= 16;    // 16 sectors
    }
    // Last entry
    //debugf("TTT%d %x %d\n", sizeof(HBA_PRDT_ENTRY), *(((uint32_t *)&cmd_tbl->prdt_entry[i])+3), count);
    cmd_tbl->prdt_entry[i].dba = (u32)buf;
    cmd_tbl->prdt_entry[i].dbau = 0;
    //debugk("dba & dbau: %p %p\n", cmd_tbl ->prdt_entry[i].dba, cmd_tbl -> prdt_entry[i].dbau);
    //debugf("TT2%d %x %d\n", sizeof(HBA_PRDT_ENTRY), *(((uint32_t *)&cmd_tbl->prdt_entry[i])+3), count);
    cmd_tbl->prdt_entry[i].dbc = (count<<9)-1;   // 512 bytes per sector
    //debugf("TT3%d %x %d\n", sizeof(HBA_PRDT_ENTRY), *(((uint32_t *)&cmd_tbl->prdt_entry[i])+3), count);
    //cmd_tbl->prdt_entry[i].i = 1;
    //debugf("%d %x\n", sizeof(HBA_PRDT_ENTRY), *(((uint32_t *)&cmd_tbl->prdt_entry[i])+3));

    // Setup command
    struct ahci_hba_fis_reg_h2d *cmd_fis = (struct ahci_hba_fis_reg_h2d *)(&cmd_tbl->cfis);

    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1;  // Command
    cmd_fis->command = ATA_CMD_WRITE_DMA_EX;
    cmd_fis->device = 1<<6;  // LBA mode

   	cmd_fis->lba0 = (uint8_t)start_lba;		
    cmd_fis->lba1 = (uint8_t)(start_lba>>8);
    cmd_fis->lba2 = (uint8_t)(start_lba>>16);
    cmd_fis->lba3 = (uint8_t)(start_lba>>24);
    cmd_fis->lba4 = (uint8_t)(start_lba>>32);
    cmd_fis->lba5 = (uint8_t)(start_lba>>40);
 
    cmd_fis->countl = (uint8_t)(count & 0xff);
    cmd_fis->counth = (uint8_t)(count >> 8);

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        //debugk("Port is hung\n");
        return 0;
    }

    port->ci = 1 << slot; // Issue command
    //debugk("PORT INFO: %x %d %d\n", port, port->ci, port->tfd);

    // Wait for completion
    while (1)
    {
        ahci_debug("Writing disk...\n");
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        //debugk("value: %d\n", (port -> ci & (1<<slot) )  );
        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            ahci_debug("1Write disk error\n");
            return 0;
        }
    }

    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        ahci_debug("2Write disk error\n");
        return 0;
    }

    return 0;
} 

int ahci_write(uint64_t start_lba, uint32_t count, uint64_t *buf)  
{
	if(start_lba < 0 && count < 1)
		return -EINVAL;

	_ahci_write(&abar->port[mgr.active_port], start_lba, count, buf);
}

int ahci_write_sector(uint64_t start_lba, uint64_t *buf)  
{
	if(start_lba < 0)
		return -EINVAL;

	_ahci_write(&abar->port[mgr.active_port], start_lba, 1, buf);
}

// Find a empty command slot
int ahci_find_empty_cmd_slot(struct ahci_hba_port *port)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slot = (port->sact | port->ci);
	int num_of_slots= (abar->ghc.cap & 0x1f00) >> 8 ; // Bit 8-12
	int i;

	for(i = 0; i < num_of_slots; i++)
	{
		if ((slot & 1) == 0)
			return i;

		slot >>= 1;
	}

	//trace_ahci("Cannot find free command list entry\n");
	return -EBUSY;
}

// in the time when below the function was called, there is no PCI MMIO. So, you must use PMIO to find AHCI device.
int find_ahci_device(struct ahci_hba_mem_reg **_abar) {
    int bus;
    int slot;
   	u16 vendor,device;

    for(bus = 0; bus < 256; bus++) {
        for(slot = 0; slot < 32; slot++) {
            vendor = pci_get_ven(bus, slot, 0); 
			device = pci_get_dev(bus, slot, 0);			

            if(vendor==PCI_VENDOR_INTEL && device==PCI_DEVICE_AHCI){
                *_abar = (struct ahci_hba_mem_reg *)pci_get_bar(bus, slot, 0, PCI_CFG_OFFSET_BAR5); 
                ahci_debug("BUS[0x%x],DEVICE[0x%x],VENDOR[0x%x],DEVICE[0x%x]\n",bus,slot,vendor,device);
                ahci_debug("AHCI ContBar 5=[0x%x]\n", *_abar); 

				return 0;
            }
        }
    }

    ahci_debug("OS Didn`t find any devices on PCI Bus\n");

    return -ENODEV;
}

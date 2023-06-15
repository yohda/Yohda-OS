#ifndef _ATA_H_
#define _ATA_H_

#include "type.h"

#define PRI_CMD_BASE			(0x1F0)
#define PRI_CTRL_BASE			(0x3F6)
#define SEC_CMD_BASE			(0x170)
#define SEC_CTRL_BASE 			(0x376)

enum {
	TYPE_ATA = 0x00,
	TYPE_ATAPI,
	TYPE_MAX,
};

enum {
	CHAN_PRI = 0x00,
	CHAN_SEC,
	CHAN_MAX,
};

#define ATA_REG_DATA(base) 		((base)|0)
#define ATA_REG_ERR(base) 		((base)|1)
#define ATA_REG_FEATURE(base) 	((base)|1)
#define ATA_REG_SECS(base)		((base)|2)
#define ATA_REG_LBA_LOW(base)	((base)|3)
#define ATA_REG_LBA_MID(base)	((base)|4)
#define ATA_REG_LBA_HIGH(base)	((base)|5)
#define ATA_REG_DEV(base)		((base)|6)
#define ATA_REG_STAT(base)		((base)|7)
#define ATA_REG_CMD(base)		((base)|7)

#define ATA_REG_ALT_STAT(base)	((base)|6)
#define ATA_REG_DEV_CTRL(base)	((base)|6)

// Command Table

#define ATA_IDEN_DEV			0xEC
#define ATA_EXE_DIAG			0x90

// Execute Diagnostic Command
#define ATA_DISK_EXIST 			((0x01)|(0x81))

struct ata_dev {
	u8 type;
	u8 chan;
};

struct ata_cmd_blk_reg {
		
};

struct ata_ctrl_blk_reg {
};

struct ata_io_reg {
};

#endif

#ifndef _AHCI_H_
#define _AHCI_H_

#include "type.h"
#include "file.h"
#include "list.h"
#include "bstree.h"
#include "bitmap.h"

// https://en.wikipedia.org/wiki/Long_filename
// `The long filename system allows a maximum length of 255 UCS-2 characters including spaces and non-alphanumeric characters ...`
// `The maximum length of a pathname is 256 characters, which includes all parent directories and the filename. 255-character mixed-case long filename is possible only for files, or folders with no sub-folders, at the root folder of any drive...`
#define FAT_LFN_MAX_LEN 		(256)

#define FAT_ATTR_READ_ONLY		0x01
#define FAT_ATTR_HIDDEN			0x02
#define FAT_ATTR_SYSTEM			0x04
#define FAT_ATTR_VOLUME_ID		0x08
#define FAT_ATTR_DIRECTORY 		0x10
#define FAT_ATTR_ARCHIVE		0x20
#define FAT_ATTR_LFN (FAT_ATTR_READ_ONLY|FAT_ATTR_HIDDEN|FAT_ATTR_SYSTEM|FAT_ATTR_VOLUME_ID)
#define FAT_LAST_LONG_ENTRY			0x40
#define FAT_ATTR_SFN 
#define FAT_ATTR_NOT_EXIST 

// FAT32 Spec _SDA Contribution_.doc - 3.1
// Size - 36B
struct fat_bios_param_blk {
	u8 		bootjmp[3];
	u8 		oem_name[8];
	u16 	bps;
	u8		spc;
	u16		reserved_sector_count;
	u8		fat_num;
	u16		root_entry_count;
	u16		f16_ts;
	u8		media_type;
	u16		fat16_region_size;
	u16		sectors_per_track;
	u16		head_side_count;
	u32 	hidden_sector_count;
	u32 	f32_ts;
}__attribute__((packed));

struct fat_ext_bios_param_blk32
{
	//extended fat32 stuff
	unsigned int		fat32_region_size;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];
}__attribute__((packed));

struct fat_fsinfo {

}__attribute__((packed));

struct fat_reserved_sector_region {
	struct fat_bios_param_blk bpb;
	struct fat_ext_bios_param_blk32 ext_bpb; 
	struct fat_fsinfo fsi;
};

// FAT32 Spec _SDA Contribution_.doc - Section 6 : Directory Structure
// Size : 32B 
struct fat_sfn {
	char name[11];	// 8 characters and 3 extension
	u8 attr;
	u8 rsvd; 					// must be 0
	u8 crt_time_tenth;
	u16 crt_time;
	u16 crt_date;
	u16 lst_acc_date;
	u16 fst_clus_hi; // 20 21
	u16 wrt_time;
	u16 wrt_date;
	u16 fst_clus_lo; // 26 27
	u32 file_size;
}__attribute__((packed));

// FAT32 Spec _SDA Contribution_.doc - Section 7 : Long File Name Implementation
// Size : 32B
struct fat_lfn {
	// long directory entry
	u8 ord;
	//char name1[10];
	u16 name1[5];
	u8 attr;
	u8 type;
	u8 crc; // checksum
	//char name2[12];
	u16 name2[6];
	u16 rsvd;
	//char name3[4];
	u16 name3[2];
	// short directory entry
}__attribute__((packed));


struct fat_common {
	u32 offset;
	u32 attr;
	char name[FAT_LFN_MAX_LEN];
};

struct fat_file {
	struct list_node li;	
	u32 ost;
	u32 attr;
	char name[FAT_LFN_MAX_LEN];
	u8 crc; // checksum
	u8 lfn;
	u8 removed;
	u32 clus_num;
	u32 clus_chain[50];	
	int *clus;
	u32 size;
	u32 drt_ost; // dirty offset
	void *bp, *cp;
};

struct fat_dir {
	// A directory also is a file.
	struct fat_file f;

	// directory specified information	
	u32 num;
	struct list_node fl;
	struct bst_node node;

	// Each directory has two root directory pointer. a one is base pointer to point a each root directory start address and the another is current pointer to point a location stored on current data. current pointer must be changed whenever data stored. 
	//void *rbp, *rcp;	
};

struct fat_region {
	u32 nums;			// total cluster and entry number per region
	u32 frees; 			// free clusters number		
	void *bp, *cp; 		// base pointer, current pointer
	struct bitmap bit;
};

/*
struct fat ext_bios_param_blk {
#ifdef CONFIG_FAT12_16


#endif
#ifdef CONFIG_FAT32


#endif
};

struct efef {

}; 





struct fat_alloc_table {

};

struct fat_fat_region {
	struct fat_alloc_table tbl1;
	struct fat_alloc_table tbl2;
};

struct fat_data_region {

};

struct fat_region {
	struct fat_reserved_sector_region reserved; 
	struct fat_fat_region fat;
#ifdef CONFIG_FAT12_16
	struct fat_root_directory_region root;
#endif
	struct fat_data_region data;
};

struct fat32_region {

};
*/

int fat_init(u64 *base);
int fat_crt_file(const char *name);
int fat_crt_dir(const char *name);
int fat_sync();

#endif

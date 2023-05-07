// https://elixir.bootlin.com/linux/latest/source/fs/fat
#include "fat.h"
#include "error.h"
#include "Utility.h"
#include "ahci.h"
#include "PIT.h"

#define FAT_FREE(x) 		(((x)==(0x00))|((x)==(0x05))|((x)==(0xE5)))
#define FAT_PARSE_NAME(x)
#define FAT_ENTRY_SIZE		32

#define FAT_32_BASE_MEM 			0x20000000
#define FAT_32_FAT_BASE_MEM 		0x20010000
#define FAT_32_ROOT_DIR_BASE_MEM	0x20400000
#define FAT_32_DATA_BASE_MEM 		0x20500000

/* Default size of a directory */
#define FAT_32_DEF_DIR_SIZE 		0x1000 // 4KB(same as linux)
#define FAT_32_ROOT_DIR_CLUS_NUM 	16
enum {
	TYPE_FAT12 = 0,
	TYPE_FAT16,
	TYPE_FAT32,
	TYPE_EXFAT,
	TYPE_FAT_NUM,
	TYPE_FAT_ERROR,
};

const u8 lfn_offset[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
const u8 fat_offset[TYPE_FAT_NUM] = {2, 2, 4, 0}; // FAT12 , FAT16, FAT32, EXFAT(Not Supported)

struct fat_info {
	u32 total_sectors;
	u32 total_clusters;
	u32 bytes_per_sector; 
	u32 sectors_per_cluster;
	u8 type;
	
	// FAT
	u32 base_fat_sector;
	u32 fat_sec_num;
	u32 fat_base_mem;

	// Root Directory
	u32 base_root_dir_sector;
	u32 root_dir_sec_num;
	u32 root_dir_base_mem;

	// DATA
	u32 base_data_sector;
	u32 data_sec_num;
	u32 data_base_mem;
};

struct fat_dir_manager {
	struct fat_dir_tree tr;	
	struct fat_dir cur_dir;
};

struct fat_dir_manager dir_mgr;
struct fat_info info;

int fat_print_info(void)
{
	// FAT Common Information
	kPrintf("FAT Total Sectors#%d\n", info.total_sectors);
	kPrintf("FAT Total Clusters#%d\n", info.total_clusters);
	kPrintf("FAT Sectors per cluster#%d\n", info.sectors_per_cluster);
	kPrintf("FAT Total bytes per sector#%d\n", info.bytes_per_sector);
	
	// FAT Type
	kPrintf("FAT Type#%d\n", info.type);
	
	// Each first sector of region
	kPrintf("FAT Region first sector#%d\n", info.base_fat_sector);
	kPrintf("Root Directory Region first sector#%d\n", info.base_root_dir_sector);
	kPrintf("DATA first sector#%d\n", info.base_data_sector);

	// Each region size of region
	kPrintf("FAT Region sector number#%d\n", info.fat_sec_num);
	kPrintf("Root Directory Region Sector Number#%d\n", info.root_dir_sec_num);
	kPrintf("Data Region Sector Number#%d\n", info.data_sec_num);
}

int fat_init(u64 *fat_base)
{
	struct fat_reserved_sector_region *reserved;
	struct fat_bios_param_blk bpb;
	struct fat_ext_bios_param_blk32 ext_bpb;

	reserved = (struct fat_reserved_sector_region *)fat_base;

	bpb = reserved->bpb;
	ext_bpb = reserved->ext_bpb;

	info.total_sectors = (bpb.fat16_total_sectors == 0) ? bpb.fat32_total_sectors : bpb.fat16_total_sectors;
	info.bytes_per_sector = bpb.bytes_per_sector;
	info.sectors_per_cluster = bpb.sectors_per_cluster;
	info.type = fat_determine_type(1, info.total_sectors);
	if(info.type < 0)
		return 0;

	// FAT
	info.base_fat_sector = bpb.reserved_sector_count;
	info.fat_sec_num = bpb.fat16_region_size ? bpb.fat16_region_size : ext_bpb.fat32_region_size;
	info.fat_sec_num = info.fat_sec_num * bpb.fat_num;

	// DATA
	info.root_dir_sec_num = ((bpb.root_entry_count * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
	if(info.type == TYPE_FAT32)
		info.root_dir_sec_num = info.sectors_per_cluster * FAT_32_ROOT_DIR_CLUS_NUM;	

	info.base_data_sector = info.base_fat_sector + info.fat_sec_num + info.root_dir_sec_num; 
		
	// Root Directory
	if(info.type == TYPE_FAT32)
		info.base_root_dir_sector = ((ext_bpb.root_cluster - 2) * bpb.sectors_per_cluster) + info.base_data_sector - info.root_dir_sec_num;

	info.data_sec_num = info.total_sectors - info.base_data_sector;	
	info.total_clusters = info.data_sec_num / info.sectors_per_cluster;

	fat_print_info();
	fat_init_rootfs();
}

int fat_init_rootfs()
{
	struct list_head *files_list;

	list_init_head(dir_mgr.cur_dir.list);	
	fat_set_dir(0, dir_mgr.cur_dir.list);

}

int fat_show_dir(u32 offset)
{
	/*
	int i;
	if(offset != dir_mgr.cur_dir.offset) {
	
	} else {
		for(i = 0 ; i < ; i++) {
		
		}
	}	
	*/
}

int fat_set_dir(u32 offset, struct list_head *list)
{
	// allocate a cluster per directory in yohdaOS.
	u8 *base = FAT_32_ROOT_DIR_BASE_MEM + (offset * (info.sectors_per_cluster * info.bytes_per_sector));
	u8 *tmp, *name;
	struct fat_file files[20];
	u8 cnt, i, j, k;

	// rootfs offset is 0 from root dir.
	fat_read_root_dir_region(offset, info.sectors_per_cluster, base);
	tmp = base;
	for(i = 0, cnt = 0 ; ; cnt = 0) {	
		// condition of exiting root directory trasvel
		if(tmp[11] == 0x00)
			break;

		files[i].offset = tmp - base;  
		kPrintf("0x%x, 0x%x\n", (u16)tmp, files[i].offset);
		if(tmp[11] == FAT_ATTR_LFN) {
			// LFN
			files[i].lfn = 1;
			cnt = (tmp[0] & 0xBF);
			
			if(!FAT_FREE((tmp+(cnt*32))[0])) {
				name = tmp + ((cnt-1)*32);
				for(j = 1; j <= cnt; j++) {
					name = tmp + ((cnt-j)*32);
 					
					for(k = 0 ; k < 13 ; k++) {
						if((name[lfn_offset[k]] == 0xFF) && (name[lfn_offset[k+1]] == 0xFF))
							break;

						files[i].name[((j-1)*13)+k] = name[lfn_offset[k]];
					}
				}
				kPrintf("#%s\n", files[i].name);
				i++;
			}
		} else {
			// SFN
			files[i].lfn = 0;
			if(!FAT_FREE(tmp[0])) {
				for(j = 0 ; j < 11 ; j++) {
					files[i].name[j] = tmp[i];	
				}
				kPrintf("#%s\n", files[i].name);
				i++;
			}
		}
	
		tmp += ((cnt+1) * FAT_ENTRY_SIZE); // SFN default size 1
	}
	
	kPrintf("rootfs file number#%d\n", i);
	
	return 0;
}

int fac_chk_sum()
{
	
}

int fat_determine_type(u32 sector_size, u32 total_clus)
{
	if (sector_size == 0) 
	{
   		return TYPE_EXFAT;
	}
	else if(total_clus < 4085) 
	{
   		return TYPE_FAT12;
	} 
	else if(total_clus < 65525) 
	{
   		return TYPE_FAT16;
	} 
	else
	{
   		return TYPE_FAT32;
	}

	return -TYPE_FAT_ERROR;
}

/* In searching all clusters in a file that you want, FAT entries are saccater in many different  sectors in fat region. If you have a file, it consist of 4 clusters. that is, you have four 4 FAT entries. But, these entries are seperated into two sectors. For example, 2 entries are in the end of first sector. And, the others are in the first location of sector after it. */
int fat_check_overflow(u32 cur_sec_num, u32 next_fat_entry)
{
	u32 bytes_per_sector, next_fat_sec_num;

	bytes_per_sector = info.bytes_per_sector;
	next_fat_sec_num = next_fat_entry / bytes_per_sector;

	return next_fat_sec_num - cur_sec_num >= 0 ? next_fat_sec_num : -1;
}

int fat_sec_to_clus()
{

}

// This function is only used in trasfering on cluster number of DATA region to sector number of FAT region.
int fat_clus_to_sec(u32 clus)
{
	u8 type;
	u32 fat_sector_offset;	
	u16 bytes_per_sector;

	type = info.type;
	bytes_per_sector = info.bytes_per_sector;

	fat_sector_offset = clus * fat_offset[type];	
	
	return fat_sector_offset / bytes_per_sector;
}

int fat_clus_to_byte(u32 clus)
{
	u8 type;
	u32 fat_sector_offset;	
	u16 bytes_per_sector;

	type = info.type;
	bytes_per_sector = info.bytes_per_sector;

	fat_sector_offset = clus * fat_offset[type];	

	return fat_sector_offset % bytes_per_sector;
}

int fat_read_data_region(struct fat_file file)
{
	
}

int fat_write_data_region()
{

}

int fat_read_root_dir_region(u32 offset, u32 cnt, struct fat_dir_entry *root_dir_base)
{	
	u32 base, root_dir_secs;
	
	base = info.base_root_dir_sector;
	
	return ahci_read(base + offset, cnt, root_dir_base);
}

int fat_write_root_dir_region(struct fat_dir_entry *root_dir_base)
{	
	u32 base, root_dir_secs;
	
	base = info.base_root_dir_sector;
	root_dir_secs = info.root_dir_sec_num;
	
	return ahci_write(base, 1, root_dir_base);
	//return ahci_write(base, root_dir_secs, root_dir_base);
}

int fat_write_fat_region(u32 sec_num, u8 *fat_table)
{
	u32 base;

	if(sec_num < 0)
		return -1;

	base = info.base_fat_sector;

	ahci_write_sector(base + sec_num, fat_table);

	return 0;
}

int fat_read_fat_region(u32 sec_num, u8 *fat_table)
{
	u32 base;

	if(sec_num < 0)
		return -1;

	base = info.base_fat_sector;

	ahci_read_sector(base + sec_num, fat_table);

	return 0;
}

int fat_update_table(u32 next_fat_sec_num, u8 *fat_table)
{
	if(next_fat_sec_num < 0)
		return -1;

	if(!next_fat_sec_num)
		return 0;

	return fat_read_fat_region(next_fat_sec_num, fat_table);
}

int fat_get_next_entry(u32 clus, struct fat_file *file)
{
	u8 type = info.type;
	u32 fat_sector_number, next_fat_entry, next_fat_sec_num, i;
	u16 bytes_per_sector, fat_byte_offset;

	fat_sector_number = fat_clus_to_sec(clus); 
	fat_byte_offset = fat_clus_to_byte(clus);

	bytes_per_sector = info.bytes_per_sector;

	u8 fat_table[bytes_per_sector];
	if(fat_read_fat_region(fat_sector_number, fat_table))
		return -1;

	while(1) {
		if (type == TYPE_FAT32) {
			next_fat_entry = *(u32*)&fat_table[fat_byte_offset];
			
			next_fat_entry &= 0x0FFFFFFF;
			if ((next_fat_entry >= 0xFFFFFF8) && (next_fat_entry <= 0xFFFFFFF)) {
				file->clus_chain[i] = next_fat_entry;	
				return 0;
			}
				
			file->clus_chain[i++] = next_fat_entry;	
			
			// From below codes, multiple clusters code, not a single cluster. 
			next_fat_sec_num = fat_check_overflow(fat_sector_number, next_fat_entry);
			if(next_fat_sec_num < 0) {
				return next_fat_sec_num;
			} 

			fat_update_table(next_fat_sec_num, fat_table);

			fat_byte_offset = next_fat_entry % bytes_per_sector;	
		}
	}	

	return -ENODEV;
}

int fat_search_clus_chains(u32 clus)
{
	int err;
	struct fat_file file;
	u32 fat_sector_offset, fat_sector_number, fat_byte_offset;

	fat_sector_number = fat_clus_to_sec(clus); 
	fat_byte_offset = (fat_sector_offset / info.bytes_per_sector);
	
	err = fat_get_next_entry(clus, &file);
	if(err < 0)
		return err;

	err = fat_read_data_region(file);	
	if(err < 0)
		return err;	

}

int fat_format()
{
	
}

int fat_create(const u8 attr, const char *name, const u16 size)
{
	struct fat_lfn *lfn;
	struct fat_sfn *sfn;
	struct fat_lfn *base = FAT_32_ROOT_DIR_BASE_MEM ;
	
	//base = info.base_root_dir_sector;
	//root_dir_secs = info.root_dir_sec_num;
	
	//kPrintf("ef 0x%x\n", base);
	//kPrintf("ef 0x%x\n", root_dir_secs);
	ahci_read(8224, 1, base);
	lfn = base + 4;
	sfn = lfn + 1;

	lfn->order = 0x41;
	lfn->name1[0] = '1';
	lfn->name1[2] = '2';
	lfn->name1[4] = '3';
	lfn->name1[6] = '.';
	lfn->name1[8] = 'T';
	lfn->attr = 0x0f;
	lfn->chk = 0xa2; 
	lfn->name2[0] = 'X';
	lfn->name2[2] = 'T';
	lfn->name2[5] = 0xFF;
	lfn->name2[6] = 0xFF;
	
	sfn->name[0] = '1';
	sfn->name[1] = '2';
	sfn->name[2] = '3';
	sfn->name[3] = 0x20;
	sfn->name[4] = 0x20;
	sfn->name[5] = 0x20;
	sfn->name[6] = 0x20;
	sfn->name[7] = 0x20;
	sfn->name[8] = 'T';
	sfn->name[9] = 'X';
	sfn->name[10] = 'T';
	sfn->attr = 0x20;	
	sfn->create_time_tenth = 0x39;
	sfn->create_time = 0x274d;
	sfn->create_date = 0xa356;
	sfn->last_access_date = 0xa356;
	sfn->last_mod_time = 0x274d;
	sfn->last_mode_date = 0x5458;

	ahci_write(8224, 1, base);	
}

int fat_write()
{

}

int _fat_read()
{

}

char *fat_combine_name(struct fat_long_dir_entry *entries)
{
}

int fat_parse_name(char *s, u8 n)
{
	int i;
	for(i = 0 ; i < n; i+=2) {
		if(!s[i])
			return -1;

		
	}

	return ;
}

int fat_read(const char *name)
{
	//struct fat_long_dir_entry *base_root_dir = FAT_32_ROOT_DIR_BASE_MEM;
	//struct fat_file *file = FAT_32_ROOT_DIR_BASE_MEM;	
	u32 root_dir_secs, root_dir_bytes, clu_num, offset;
	int err;

	/*
	root_dir_secs = info.root_dir_sec_num;
	root_dir_bytes = root_dir_secs * info.bytes_per_sector;
		
	err = fat_read_root_dir_region(base_root_dir);
	if(err < 0)
		return err;

	fat_create(0xff, "eef", 3);
	while(1){
		offset = 1; // Added Default SFN Size
		kPrintf("file->sde.name#%s\n", base_root_dir->name1);
		kPrintf("adress#0x%x\n", base_root_dir);

		

		for(i = 0 ; i < n; i+=2)
			if(!base_root_dir->name1[0])
				base_root_dir->name
		;	
		while(1) {
			
		}
			
		if(base_root_dir->attr == FAT_ATTR_LFN) {
			// LFN	
			kPrintf("order#0x%x\n", base_root_dir->order);
			offset += (base_root_dir->order & 0xBF);
			base_root_dir += offset;
		} else if (base_root_dir->attr == 0x00){
			kPrintf("Over!!!#0x%x\n", base_root_dir);
			break;
		}
	}
*/
	//kPrintf("file->lde.order#0x%x\n", file->lde.order);
	//kPrintf("file->lde.attr#0x%x\n", file->lde.attr);
	//kPrintf("file->sde.name#%s\n", file->lde.sde.name);
	//kPrintf("file->sde.attr#%x\n", file->lde.sde.attr);
	//kPrintf("file->attr#0x%x\n", file->lde.attr);
	
	//kPrintf("i#0x%x\n", *i);
	//kPrintf("i#0x%x\n", *i);

	//entry = (struct fat_directory_entry *)entry;	
	//kPrintf("s:%s\n", entry->name);
	/*
	while(root_dir_bytes) {
			
		if(!kMemCmp(entry->name, name, 11))
			break;	

		root_dir_bytes -= sizeof(struct fat_directory_entry);	
		entry += 1;
	}

	if(!root_dir_bytes)
		return -ENOENT;
 	
	clu_num = (entry->start_clu_num_h << 16) | entry->start_clu_num_l;
	if(fat_search_clus_chains(clu_num))
	*/

	return 0;
}

int fat_remove(struct fat_file *file)
{
}

// https://elixir.bootlin.com/linux/latest/source/fs/fat
#include "fat.h"
#include "error.h"
#include "debug.h"
#include "Utility.h"
#include "ahci.h"
#include "PIT.h"
#include "mm.h"
#include "string.h"

// If you wonder the below information, refer to `6.1 File/Directory Name (field DIR_Name)` in Microsoft Word - FAT32 Spec _SDA Contribution_.doc
// Actually, in SFN, if `KANJI` langauge supported, 0x05 in a value of first character of name fieid means that this entry is free. But, yohdaOS doesn`t support `KANJI`.  
#define FAT_FREE(x) 				(((x)==(0x00))||((x)==(0xE5)))
#define FAT_INVAL_SFN_LESS_THAN 	(0x20)
#define FAT_INVAL_SFN_FST(x) 		((FAT_FREE(x))||(x)==(FAT_INVAL_SFN_LESS_THAN))

const int sfn_inval[] = { 0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C };
bool sfn_inval_tbl[128] = { 0x00, };

#define FAT_ENTRY_SIZE		32

#define FAT_32_BASE_MEM 			0x20000000
#define FAT_32_fba 		0x20010000
#define FAT_32_rba	0x20400000
#define FAT_32_dba 		0x20500000

#define F32_SFN_LEN 	11
#define FAT_SFN_NAME	8
#define FAT_SFN_EXT		3
#define F32_LFN_LEN		13

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

const u16 lfn_pad[] = { 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
const u8 lfn_ost[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
const u8 fat_offset[TYPE_FAT_NUM] = {2, 2, 4, 0}; // FAT12 , FAT16, FAT32, EXFAT(Not Supported)

struct fat_info {
	// Information 
	u32 ts;			// total sectors
	u32 tc;			// total clusters
	u32 bps;		// bytes per sector
	u32 spc;		// sectors per cluster
	u32 ds;			// directory size

	u8 type;
	
	// FAT
	u32 ffs;		// FAT region first sector
	u32 fsn;		// FAT region sector number
	u32 fba;		// FAT region base address

	// Root Directory
	u32 rfs;		// root directory region first sector
	u32 rsn;		// root directory region sector number
	u32 rba;		// root directory region base address
	void* rmb, rmc; // root directory memory base & current address

	// DATA
	u32 dfs;		// data region first sector
	u32 dsn;		// data region sector number
	u32 dba;		// data region base address

	bool lfn;		// supported lfn
};

struct fat_dir_manager {
	struct fat_dir_tree tr;	
	struct fat_dir cd; // current directory
};

struct fat_dir_manager dir_mgr;
struct fat_info fat;

static int fat_read_rr(u32 ost, u32 cnt, int size, void *to)
{
	int sc = (size + (fat.bps-1)) / fat.bps	
	return ahci_read(fat.rfs + ost, cnt, to);
}

int fat_print_fat(void)
{
	// FAT Common fatrmation
	fat_debug("FAT Total Sectors#%d\n", fat.ts);
	fat_debug("FAT Total Clusters#%d\n", fat.tc);
	fat_debug("FAT Sectors per cluster#%d\n", fat.spc);
	fat_debug("FAT Total bytes per sector#%d\n", fat.bps);
	
	// FAT Type
	fat_debug("FAT Type#%d\n", fat.type);
	
	// Each first sector of region
	fat_debug("FAT Region first sector#%d\n", fat.ffs);
	fat_debug("Root Directory Region first sector#%d\n", fat.rfs);
	fat_debug("DATA first sector#%d\n", fat.dfs);

	// Each region size of region
	fat_debug("FAT Region sector number#%d\n", fat.fsn);
	fat_debug("Root Directory Region Sector Number#%d\n", fat.rsn);
	fat_debug("Data Region Sector Number#%d\n", fat.dsn);
}

int fat_init(u64 *fat_base)
{
	int i;
	struct fat_reserved_sector_region *reserved;
	struct fat_bios_param_blk bpb;
	struct fat_ext_bios_param_blk32 ext_bpb;

	reserved = (struct fat_reserved_sector_region *)fat_base;

	fat_debug("wefwefwef#0x%x\n", mm_get_ml());

	bpb = reserved->bpb;
	ext_bpb = reserved->ext_bpb;

	fat.ts = (bpb.f16_ts == 0) ? bpb.f32_ts : bpb.f16_ts;
	fat.bps = bpb.bps;
	fat.spc = bpb.spc;
	fat.type = fat_determine_type(1, fat.ts);
	if(fat.type < 0)
		return 0;

	// FAT
	fat.ffs = bpb.reserved_sector_count;
	fat.fsn = bpb.fat16_region_size ? bpb.fat16_region_size : ext_bpb.fat32_region_size;
	fat.fsn = fat.fsn * bpb.fat_num;

	// DATA
	fat.rsn = ((bpb.root_entry_count * 32) + (bpb.bps - 1)) / bpb.bps;
	if(fat.type == TYPE_FAT32)
		fat.rsn = fat.spc * FAT_32_ROOT_DIR_CLUS_NUM;	

	fat.dfs = fat.ffs + fat.fsn + fat.rsn; 
		
	// Root Directory
	if(fat.type == TYPE_FAT32)
		fat.rfs = ((ext_bpb.root_cluster - 2) * bpb.spc) + fat.dfs - fat.rsn;

	fat.dsn = fat.ts - fat.dfs;	
	fat.tc = fat.dsn / fat.spc;

	// ETC
	// `Microsoft Word - FAT32 Spec _SDA Contribution_.doc` "6.5 Directory creation"
	/*
	 * At least one cluster must be allocated
	 */
	fat.ds = fat.bps * fat.spc; 

	fat.rmb = mm_alloc(fat.ds, MM_KL); 
	fat.rmc = fat.rmb;

	memset(sfn_inval_tbl, 0x01, FAT_INVAL_SFN_LESS_THAN);
	for(i=0 ; i < (sizeof(sfn_inval)/sizeof(char)) ; i++) {
		sfn_inval_tbl[sfn_inval[i]] = true;		
	}

	// lfn supported
	fat.lfn = true;

	fat_print_fat();
	fat_init_rootfs();
}

int fat_init_rootfs()
{
	fat_set_dir(0);

}

void fat_show_dir()
{
	int i;

	for(i=0 ; i < dir_mgr.cd.num ;i++) {
		msg("%s ", dir_mgr.cd.files[i].name);
	}	
	msg("\n");
}

void fat_get_cd()
{
	
}

int fat_set_dir(u32 ost, char *path)
{
	// allocate a cluster per directory in yohdaOS.
	u8 *rr = (u8 *)mm_alloc(fat.ds, MM_KL);
	u8 *tmp, *name;
	struct fat_file files[20];
	u8 cnt, i, j, k;

	// rootfs offset is 0 from root dir.
	fat_read_rr(ost, fat.spc, rr);
	tmp = rr;
	for(i = 0, cnt = 0 ; ; cnt = 0) {	
		// condition of exiting root directory trasvel
		if(tmp[11] == 0x00)
			break;

		files[i].offset = tmp - rr;  
		//fat_debug("0x%x, 0x%x\n", (u16)tmp, files[i].offset);
		if(tmp[11] == FAT_ATTR_LFN) {
			// LFN
			files[i].lfn = 1;
			cnt = (tmp[0] & 0xBF);
			
			if(!FAT_FREE((tmp+(cnt*32))[0])) {
				name = tmp + ((cnt-1)*32);
				for(j = 1; j <= cnt; j++) {
					name = tmp + ((cnt-j)*32);
 					
					for(k = 0 ; k < F32_LFN_LEN ; k++) {
						if((name[lfn_ost[k]] == 0xFF) && (name[lfn_ost[k+1]] == 0xFF))
							break;

						files[i].name[((j-1)*F32_LFN_LEN)+k] = name[lfn_ost[k]];
					}
				}
				//fat_debug("#%s\n", files[i].name);
				i++;
			}
		} else {
			// SFN
			files[i].lfn = 0;
			if(!FAT_FREE(tmp[0])) {
				for(j = 0 ; j < 11 ; j++) {
					files[i].name[j] = tmp[i];	
				}
				fat_debug("#%s\n", files[i].name);
				i++;
			}
		}
	
		tmp += ((cnt+1) * FAT_ENTRY_SIZE); // SFN default size 1
	}

	dir_mgr.cd.num = i;
	dir_mgr.cd.files = mm_alloc(sizeof(struct fat_file) * i, MM_KL);
	for(i=0 ; i < dir_mgr.cd.num; i++) {
		memcpy(&(dir_mgr.cd.files[i]), &files[i], sizeof(struct fat_file));		
	}

	fat_debug("rootfs file number#%d\n", i);
	
	return 0;
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
	u32 bps, next_fsn;

	bps = fat.bps;
	next_fsn = next_fat_entry / bps;

	return next_fsn - cur_sec_num >= 0 ? next_fsn : -1;
}

int fat_sec_to_clus()
{

}

// This function is only used in trasfering on cluster number of DATA region to sector number of FAT region.
int fat_clus_to_sec(u32 clus)
{
	u8 type;
	u32 fat_sector_offset;	
	u16 bps;

	type = fat.type;
	bps = fat.bps;

	fat_sector_offset = clus * fat_offset[type];	
	
	return fat_sector_offset / bps;
}

int fat_clus_to_byte(u32 clus)
{
	u8 type;
	u32 fat_sector_offset;	
	u16 bps;

	type = fat.type;
	bps = fat.bps;

	fat_sector_offset = clus * fat_offset[type];	

	return fat_sector_offset % bps;
}

int fat_read_data_region(struct fat_file file)
{
	
}

int fat_write_data_region()
{

}


static int fat_write_rr(int cnt, void *from)
{	
	u32 base, root_dir_secs;
	
	base = fat.rfs;
	root_dir_secs = fat.rsn;

	fat_debug("");	
	return ahci_write(base, cnt, from);
	//return ahci_write(base, root_dir_secs, root_dir_base);
}

int fat_write_fat_region(u32 sec_num, u8 *fat_table)
{
	u32 base;

	if(sec_num < 0)
		return -1;

	base = fat.ffs;

	ahci_write_sector(base + sec_num, fat_table);

	return 0;
}

int fat_read_fat_region(u32 sec_num, u8 *fat_table)
{
	u32 base;

	if(sec_num < 0)
		return -1;

	base = fat.ffs;

	ahci_read_sector(base + sec_num, fat_table);

	return 0;
}

int fat_update_table(u32 next_fsn, u8 *fat_table)
{
	if(next_fsn < 0)
		return -1;

	if(!next_fsn)
		return 0;

	return fat_read_fat_region(next_fsn, fat_table);
}

int fat_get_next_entry(u32 clus, struct fat_file *file)
{
	u8 type = fat.type;
	u32 fat_sector_number, next_fat_entry, next_fsn, i;
	u16 bps, fat_byte_offset;

	fat_sector_number = fat_clus_to_sec(clus); 
	fat_byte_offset = fat_clus_to_byte(clus);

	bps = fat.bps;

	u8 fat_table[bps];
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
			next_fsn = fat_check_overflow(fat_sector_number, next_fat_entry);
			if(next_fsn < 0) {
				return next_fsn;
			} 

			fat_update_table(next_fsn, fat_table);

			fat_byte_offset = next_fat_entry % bps;	
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
	fat_byte_offset = (fat_sector_offset / fat.bps);
	
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

static int fat_parse_sfn(const char *name, struct fat_sfn *sfn)
{
	int dot = strchr(name, '.') ? (strchr(name, '.')-name) : -1;	
	char pad_name[sizeof(char)*F32_SFN_LEN+1]; // `+1` due to NULL.
	
	fat_debug("dot#%d\n", dot);
	memset(pad_name, 0x20, F32_SFN_LEN+1);
	pad_name[F32_SFN_LEN] = 0;
	if(dot > -1) {
		// there is a dot.
		strncpy(pad_name, name, min(dot, 8));
		fat_debug("%s\n", pad_name);
		strncpy(pad_name+8, name+dot+1, min(strlen(name+dot+1), 3));
	} else {
		// no dot.
		strncpy(pad_name, name, strlen(name) >= F32_SFN_LEN ? F32_SFN_LEN : strlen(name));
	}
	
	fat_debug("len#%d, name#%s\n", strlen(pad_name), pad_name);

	strncpy(sfn->name, pad_name, F32_SFN_LEN);
	
	return 0;
}

static int fat_val_sfn(char *name)
{
	int i;
	struct fat_file *fs = dir_mgr.cd.files;
/*	
	for(i=0; i < dir_mgr.cd.num ; i++) {
		if(!strcmp(fs[i].name, name)) {
			fat_debug("efef\n")
			return -1; // There is already file name.
		}
	}
*/
	if(FAT_INVAL_SFN_FST(*name)) {
		fat_debug("efef\n");
		return -1; 
	}

	for(i=0 ; name[i] ; i++) {
		if(sfn_inval_tbl[name[i]] && 0) {
			fat_debug("SFN Invalid character index#%d\n", i);
			return -1;	
		}
	}

	return 0;
}

static int fat_link_clus(struct fat_sfn *sfn)
{
	return 0;
}

static int fat_set_tm_dt(struct fat_sfn *sfn)
{
	return 0; 	
}

static int fat_set_props(const u8 attr, struct fat_sfn *sfn)
{
	struct fat_sfn tmp;

	tmp.attr = attr;
	tmp.crt_time = 0;
	tmp.crt_date = 0;
	tmp.wrt_time = tmp.crt_time;
	tmp.wrt_date = tmp.crt_date;	
	
	fat_link_clus(&tmp);
	fat_set_tm_dt(&tmp);
	
	return 0;
}

static int fat_crt_sfn(char *name, const u8 attr, struct fat_sfn *sfn)
{	
	int i, err;
	struct fat_sfn tmp;

	err = fat_val_sfn(name);	
	if(err)
		return err;

	err = fat_parse_sfn(name, &tmp);	
	if(err)
		return err;

	err = fat_set_props(attr, &tmp);
	if(err)
		return err;

	memcpy(sfn, &tmp, sizeof(struct fat_sfn));	
	return 0;
}

static int fat_get_lfns(u32 n)
{
	return (n + (F32_LFN_LEN-1)) / F32_LFN_LEN;
}

static int fat_pad_lfn(u8 *lfn, int len)
{
	int i = 0, n = len % F32_LFN_LEN;

	if(!lfn)
		return NULL;
	
	for(i=n; i<F32_LFN_LEN; i++) {
		*((u16 *)&(lfn[lfn_ost[i]])) = (u16)lfn_pad[i-n];	
	}
	
	return 0; 
}

static u8 fat_calc_crc(u8 *sfn)
{
	u8 n = F32_SFN_LEN;
 	u8 crc = 0;
 	
	while(n--) {
 		crc = ((crc & 1) ? 0x80 : 0) + (crc >> 1) + *sfn++;
	}

	return (crc);	
}

static int fat_parse_lfn(u8 *dst, u8 *s, u8 n)
{
	int i = 0;
	int len = strlen(s);

	for(i=0 ; i<F32_LFN_LEN; i++) {
		dst[lfn_ost[i]] = s[i+(F32_LFN_LEN*n)];
	}

	return 0;
}

static int fat_crt_lfn(const char *name, u32 attr, u8 crc, struct fat_lfn *lfns)
{
	int i = 0, err = 0, len = strlen(name);
	int lfn_num = fat_get_lfns(len);
	struct fat_lfn tmps[lfn_num];

	memset(tmps, 0, sizeof(tmps));
	for(i=0 ; i < lfn_num; i++) {
		tmps[i].ord = i+1; 
		tmps[i].attr |= FAT_ATTR_LFN;
		tmps[i].crc = crc;
		
		err = fat_parse_lfn((u8 *)(tmps+i), name, i);
	}
	
	fat_pad_lfn(tmps+(i-1), len);	
	tmps[i-1].ord |= FAT_LAST_LONG_ENTRY;
	
	memcpy(lfns, tmps, sizeof(tmps));

	return 0;
}

static int fat_comb(const struct fat_lfn *lfns, const struct fat_sfn *sfn, struct fat_lfn * rde, u32 n)
{
	int i;	
			
	fat_debug("yohdalen2#%d\n", n);
	//memcpy(rde, lfns, n*sizeof(struct fat_lfn));
	for(i=0 ; i<n ;i++) {
		memcpy(rde+i, lfns+(n-i-1), sizeof(struct fat_lfn));	
	}

	memcpy(rde+n, sfn, sizeof(struct fat_sfn));
	return 0;
}

int fat_crt(const u8 attr, const char *name)
{
	int err, lfn_num, len, crc;
	void *rde; // root directory entry

	len = strlen(name);
	lfn_num = fat_get_lfns(len);


	struct fat_lfn lfns[lfn_num];
	struct fat_sfn sfn;

	rde = mm_alloc(sizeof(lfns) + sizeof(sfn), MM_KL); 
	if(!rde)
		return -1;
	
	fat_debug("aaa#%d bbb#%d ccc#%d\n", len, lfn_num, sizeof(lfns) + sizeof(sfn));

	memset(rde, 0, sizeof(lfns) + sizeof(sfn));
	err = fat_crt_sfn(name, attr, &sfn);
	if(err)
		return err;

	crc = fat_calc_crc((u8 *)(sfn.name));
	err = fat_crt_lfn(name, attr, crc, &lfns);
	if(err)
		return err;

	err = fat_comb(lfns, &sfn, rde, lfn_num);	
	if(err | !rde)
		return -1;

	err = fat_write_rr(1, rde);
	free(rde);
	if(err)
		return err;
/*
	//base = fat.rfs;
	//root_dir_secs = fat.rsn;
	
	//fat_debug("ef 0x%x\n", base);
	//fat_debug("ef 0x%x\n", root_dir_secs);
	ahci_read(8224, 1, base);
	lfn = base + 4;
	sfn = lfn + 1;

	lfn->ord = 0x41;
	lfn->name1[0] = '1';
	lfn->name1[2] = '2';
	lfn->name1[4] = '3';
	lfn->name1[6] = '.';
	lfn->name1[8] = 'T';
	lfn->attr = 0x0f;
	lfn->crc = 0xa2; 
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
*/
}

int fat_write()
{

}

int _fat_read()
{

}

int fat_read(const char *name)
{
	//struct fat_long_dir_entry *base_root_dir = FAT_32_rba;
	//struct fat_file *file = FAT_32_rba;	
	u32 root_dir_secs, root_dir_bytes, clu_num, offset;
	int err;

	/*
	root_dir_secs = fat.rsn;
	root_dir_bytes = root_dir_secs * fat.bps;
		
	err = fat_read_rr(base_root_dir);
	if(err < 0)
		return err;

	fat_crt(0xff, "eef", 3);
	while(1){
		offset = 1; // Added Default SFN Size
		fat_debug("file->sde.name#%s\n", base_root_dir->name1);
		fat_debug("adress#0x%x\n", base_root_dir);

		

		for(i = 0 ; i < n; i+=2)
			if(!base_root_dir->name1[0])
				base_root_dir->name
		;	
		while(1) {
			
		}
			
		if(base_root_dir->attr == FAT_ATTR_LFN) {
			// LFN	
			fat_debug("ord#0x%x\n", base_root_dir->ord);
			offset += (base_root_dir->ord & 0xBF);
			base_root_dir += offset;
		} else if (base_root_dir->attr == 0x00){
			fat_debug("Over!!!#0x%x\n", base_root_dir);
			break;
		}
	}
*/
	//fat_debug("file->lde.ord#0x%x\n", file->lde.ord);
	//fat_debug("file->lde.attr#0x%x\n", file->lde.attr);
	//fat_debug("file->sde.name#%s\n", file->lde.sde.name);
	//fat_debug("file->sde.attr#%x\n", file->lde.sde.attr);
	//fat_debug("file->attr#0x%x\n", file->lde.attr);
	
	//fat_debug("i#0x%x\n", *i);
	//fat_debug("i#0x%x\n", *i);

	//entry = (struct fat_directory_entry *)entry;	
	//fat_debug("s:%s\n", entry->name);
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

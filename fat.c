// https://elixir.bootlin.com/linux/latest/source/fs/fat
#include "fat.h"
#include "error.h"
#include "debug.h"
#include "Utility.h"
#include "ahci.h"
#include "PIT.h"
#include "mm.h"
#include "string.h"
#include "bitmap.h"
#include "stack.h"

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
#define FAT_32_cba 		0x20500000

#define F32_SFN_LEN 	11
#define FAT_SFN_NAME	8
#define FAT_SFN_EXT		3
#define F32_LFN_LEN		13

/* Cluster */
#define FAT_CLUS_USED	BIT_SET

/* Default size of a directory */
#define FAT_32_DEF_DIR_SIZE 		0x1000 // 4KB(same as linux)

/* FAT Region */
#define FAT_ENTRY_BYTE 				4

/* Root directory cluster number */
#define FAT_ROOT_DIR_CLUS 			8

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

// The `fat_info` data structure is information storage about FAT32. 
// So, it means that its variable is constants, that is, read-only. 
struct fat_info {
	// Information 
	u32 ts;			// total sectors
	u32 tc;			// total clusters
	u32 bps;		// bytes per sector
	u32 spc;		// sectors per cluster
	u32 ds;			// default a root directory size

	u8 type;
	
	// FAT
	u32 ffs;		// FAT region first sector
	u32 fsn;		// FAT region sector number
	u32 fba;		// FAT region base address

	// Root Directory
	u32 rfs;		// root directory region first sector
	u32 rsn;		// root directory region sector number
	u32 rba;		// root directory region base address

	// Data
	u32 dfs;		// data region first sector
	u32 dsn;		// data region sector number
	u32 dba;		// data region base address

	u32 cfs;		// available first sector in data region except reserved clusters.
	u32 csn;		// aviilable sector number in data region except reserved clusters.

	bool lfn;		// supported lfn
};

// 각 디렉토리는 클러스터 사이즈만큼의 루트 디렉토리 영역에 메모리를 할당받는다. 만약, 특정 디렉토리가 할당받은 영역을 초과하면 아래의 rcp가 가리키는 곳에 새롭게 클러스터 사이즈만큼 영역을 할당 받는다.
struct fat_mem {
	struct fat_dir *cd; 		// current directory

	// Cache
	struct bitmap cc;		// Cache bitmap  
	struct bitmap drt;		// Cache dirty check
	
	struct fat_region r;	// directory region	
	struct fat_region f;	// fat region 
	struct fat_region c; 	// cluster region
};

struct fat_mem fmm;
struct fat_info fat;
struct fat_dir root;
struct stack stk;

static _fat_read_dir(int ost, int n, void *from)
{
	if(!from)
		return -1;

	// If ost is -1, this is access of system reserved cluster.
	if(ost < -2 || n < 1)
		return -2;

	return ahci_read(fat.rfs + ost, n, from);
}

// This function reads the a cluster from secondary storage. 
// The first argument is a offset from base address of root directory. 
// The second argument is a size byte unit. So, it was converted into sector unit.
// The third argument is a pointer to receive data from secondary storage.
static int fat_read_dir(int ost, void *from)
{
	if(!from)
		return -1;

	// If ost is -1, this is access of system reserved cluster.
	if(ost < -2)
		return -2;

	return _fat_read_dir(ost, fat.spc, from);
	//return ahci_read(fat.rfs + ost, fat.spc, from);
}

// This function writes the a cluster to secondary storage. 
static int fat_write_dir(int ost, void *to)
{	
	//fat_debug("start#%d, count#%d\n", fat.rfs + ost, fat.spc);	
	
	if(!to)
		return -1;

	// If ost is -1, this is access of system reserved cluster.
	if(ost < -2)
		return -2;

	return ahci_write(fat.rfs + ost, fat.spc, to);
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
	fat_debug("Available First Data Sector#%d\n", fat.cfs);

	// Each region size of region
	fat_debug("FAT Region sector number#%d\n", fat.fsn);
	fat_debug("Root Directory Region Sector Number#%d\n", fat.rsn);
	fat_debug("Data Region Sector Number#%d\n", fat.dsn);
	fat_debug("Available Data Sector Number#%d\n", fat.csn);
}

int fat_init(u64 *fat_base)
{
	int i, err = -1;
	struct fat_reserved_sector_region *reserved;
	struct fat_bios_param_blk bpb;
	struct fat_ext_bios_param_blk32 ext_bpb;

	reserved = (struct fat_reserved_sector_region *)fat_base;

	bpb = reserved->bpb;
	ext_bpb = reserved->ext_bpb;

	fat.ts = (bpb.f16_ts == 0) ? bpb.f32_ts : bpb.f16_ts;
	fat.bps = bpb.bps;
	fat.spc = bpb.spc;
	fat.type = fat_determine_type(1, fat.ts);
	if(fat.type < 0)
		return 0;

	// FAT
	// After reserved two clusters, a next cluster is yohdaOS system cluster.
	fat.ffs = bpb.reserved_sector_count;	
	fat.fsn = bpb.fat16_region_size ? bpb.fat16_region_size : ext_bpb.fat32_region_size;
	fat.fsn = fat.fsn * bpb.fat_num;

	// DATA
	fat.rsn = (((bpb.root_entry_count * 32) + (bpb.bps - 1)) / bpb.bps) + (fat.spc * FAT_ROOT_DIR_CLUS);
	fat.dfs = fat.ffs + fat.fsn; 
	fat.cfs = fat.dfs + ((fat.spc) * 2) + 1; // After reserved two clusters, a next cluster is yohdaOS system cluster.	

	// Root Directory
	if(fat.type == TYPE_FAT32) {
		fat.rfs = ((ext_bpb.root_cluster - 2) * bpb.spc) + fat.cfs;
		fat.cfs += fat.spc * FAT_ROOT_DIR_CLUS; // After reserved two clusters, a next cluster is yohdaOS system cluster.	
	}

	fat.dsn = fat.ts - fat.dfs;	
	fat.csn = fat.dsn - (((fat.spc) * 2) + 1 + (fat.spc * FAT_ROOT_DIR_CLUS)); // In FAT32, two cluster are reserved and in yohdaOS, a one cluster after these is system cluster. 
	fat.tc = fat.dsn / fat.spc;

	// Re-calculate FAT region clusters because the above fat calculation is just for convenient calculation about data and root directory region. So, from here except back FAT region.
	fat.fsn /= bpb.fat_num;


	// `Microsoft Word - FAT32 Spec _SDA Contribution_.doc` "6.5 Directory creation"
	/*
	 * At least one cluster must be allocated
	 */
	fat.ds = fat.bps * fat.spc; 

	memset(sfn_inval_tbl, 0x01, FAT_INVAL_SFN_LESS_THAN);
	for(i=0 ; i < (sizeof(sfn_inval)/sizeof(char)) ; i++) {
		sfn_inval_tbl[sfn_inval[i]] = true;		
	}

	// lfn supported
	fat.lfn = true;

	// Each region(FAT, Root Directory, DATA) entry and cluster numbers.
	fmm.c.nums = fat.csn / fat.spc;
	fmm.c.frees = fmm.c.nums;

	fmm.r.nums = (fat.rsn*fat.bps)/FAT_ENTRY_SIZE;
	fmm.r.frees = fmm.r.nums;

	fmm.f.nums = (fat.fsn*fat.bps)/FAT_ENTRY_BYTE;
	fmm.r.frees = fmm.f.nums;

	/* bitmap initialize */ 
	// There is exception case about remainder... you must to check bitmap leaks. 
	bitmap_alloc(&fmm.f.bit, 32, (fmm.f.nums+31)/32);
	bitmap_alloc(&fmm.r.bit, 32, (fmm.r.nums+31)/32);
	bitmap_alloc(&fmm.c.bit, 32, (fmm.c.nums+31)/32);

	fat_debug("bitmap %d, %d, %d\n", (((fat.fsn*fat.bps)/FAT_ENTRY_BYTE)+31)/32, (((fat.rsn*fat.bps)/FAT_ENTRY_SIZE)+31)/32, ((fat.csn/fat.spc)+31)/32);

	fmm.r.bp = mm_alloc(FAT_ROOT_DIR_CLUS*fat.ds, MM_KL);
	if(!fmm.r.bp)
		return -10;	

	memset(fmm.r.bp, 0, sizeof(FAT_ROOT_DIR_CLUS*fat.ds));
	//char *bff[FAT_ROOT_DIR_CLUS*fat.ds];
	//memset(bff, fat.ds, 0);
	//fat_write_dir(2, bff);

	/* Initialize rootfs */
	err = fat_init_rootfs();
	if(err < 0) {
		fat_debug("Failed to initialize root filesystem#%d\n", err);
		return err;
	}
	/* Print FAT32 information  */
	//fat_print_fat();
}

static int fat_crt_dir_tree(const struct bst_node *root)
{
	struct fat_dir *dirs;
	int err = -1, size = 0;

	if(!fmm.r.bp)
		return -1;
	
	size = (FAT_ROOT_DIR_CLUS*fat.ds) / FAT_ENTRY_SIZE;
	dirs = mm_alloc(sizeof(struct fat_dir)*size, MM_KL);
	if(!dirs)
		return -4;	
	
	err = _fat_read_dir(0, FAT_ROOT_DIR_CLUS*fat.spc, fmm.r.bp);		
	if(err < 0)
		return -8;	

	//fat_conv_e2f();

	return 0;	
}

static int fat_search_comp(struct bst_node *node, void *key)
{
	struct fat_dir *dir = container_of(node, struct fat_dir, node);
	char *name = (char *)key;
	int diff = 0;

	diff = strcmp(name, (dir->f).name);
	if(diff)
		return (diff < 0) ? BST_LEFT : BST_RIGHT;
		
	return BST_MATCH;
}

static int fat_insert_comp(struct bst_node *_root, struct bst_node *_nw)
{
	struct fat_dir *root = container_of(_root, struct fat_dir, node);
	struct fat_dir *nw = container_of(_nw, struct fat_dir, node);
	int diff = 0;

	diff = strcmp(nw->f.name, root->f.name);
	if(diff)
		return (diff < 0) ? BST_LEFT : BST_RIGHT;
		
	return BST_MATCH;
}

static int fat_get_ost(u8 *ent)
{	
	bool is_lfn;
	if(!ent)
		return NULL;
	
	is_lfn = ((*ent) & FAT_LAST_LONG_ENTRY) == FAT_LAST_LONG_ENTRY;
	return is_lfn ? ((*ent) & 0xBF) : 1;
}

static int fat_get_entries(const int clu)
{
	int n = 0, ost = 0, err = -1;
	u8 *ent = NULL;

	if(clu < 0)
		return -1;

	ent = mm_alloc(fat.ds, MM_KL);
	if(!ent)
		return -2;

	memset(ent, 0, fat.ds);
	err = fat_read_dir(clu, ent);	
	if(err)
		return -3;

	for( ; *ent ; ent+=((ost+1)*FAT_ENTRY_SIZE)) {
		ost = fat_get_ost(ent);
		
		if(FAT_FREE(*ent))
			continue;	
	
		n++;	
	}

	mm_free(ent);
	
	return n;
}

void fat_show_dir()
{
	struct list_node *node;
	struct fat_file *file;
	int i;

	msg("Total files number#%d\n", fmm.cd->num);
	list_for_each(&fmm.cd->fl, node) {
		file = container_of(node, struct fat_file, li);
		if(file->attr == 0x14)
			continue;	
	
		msg("%s ", file->name);
	}
	msg("\n");
}

void fat_get_cd()
{
	
}

static int fat_get_fn()
{

}

int _fat_set_dir(struct fat_dir *dir)
{
	int i = 0;
	
	if(!dir)
		return -1;

	fmm.cd = dir;
	
	return 0;
}

int fat_set_dir(const char *path)
{
	// allocate a cluster per directory in yohdaOS.
	int len = 0;
	u8 *rr = (u8 *)mm_alloc(fat.ds, MM_KL);
	u8 *tmp, *name;
	struct fat_file files[20];
	u8 cnt, i, j, k;

	if(!path)
		return -1;

	len = strlen(path);
	if(len < 1)
		return -2;	
	
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

static int _fat_crt_sfn(const char *name, struct fat_sfn *sfn)
{
	int dot = strchr(name, '.') ? (strchr(name, '.')-name) : -1;	
	char pad_name[sizeof(char)*F32_SFN_LEN+1]; // `+1` due to NULL.
	
	memset(pad_name, 0x20, F32_SFN_LEN+1);
	pad_name[F32_SFN_LEN] = 0;
	if(dot > -1) {
		// there is a dot.
		strncpy(pad_name, name, min(dot, 8));
		strncpy(pad_name+8, name+dot+1, min(strlen(name+dot+1), 3));
	} else {
		// no dot.
		strncpy(pad_name, name, strlen(name) >= F32_SFN_LEN ? F32_SFN_LEN : strlen(name));
	}

	strncpy(sfn->name, pad_name, F32_SFN_LEN);
	
	return 0;
}

static int fat_vali_sfn(const char *name)
{
	int i;
	//struct fat_file *fs = fmm.cd->files;

	if(!name)
		return -1;
/*	
	for(i=0; i < fmm.cd->num ; i++) {
		if(!strcmp(fs[i].name, name)) {
			fat_debug("efef\n")
			return -1; // There is already file name.
		}
	}
	
	if(FAT_INVAL_SFN_FST(*name)) {
		fat_debug("SFN Invalid first name#(%d)\n", *name);
		return -1; 
	}
*/
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

static int fat_crt_sfn(const struct fat_file *file, struct fat_sfn *sfn)
{	
	int i, err;

	err = fat_vali_sfn(file->name);	
	if(err)
		return err;

	err = _fat_crt_sfn(file->name, sfn);	
	if(err)
		return err;

	sfn->attr = file->attr;

	return 0;
}

static int fat_get_lfns(u32 n)
{
	return (n + (F32_LFN_LEN-1)) / F32_LFN_LEN;
}

static int fat_pad_lfn(u8 *lfn, int len)
{
	int i = 0, n = 0;

	if(!lfn || len<1)
		return -1;

	n = len%F32_LFN_LEN;
	if(!n) 
		return 0;

	for(i=n ; i<F32_LFN_LEN ; i++) {
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

static int _fat_crt_lfn(u8 *dst, u8 *s, u8 n)
{
	int i = 0;
	int len = strlen(s);

	for(i=0 ; i<F32_LFN_LEN; i++) {
		dst[lfn_ost[i]] = s[i+(F32_LFN_LEN*n)];
	}

	return 0;
}

static int fat_crt_lfn(const char *name, u8 crc, struct fat_lfn *lfns)
{
	int i = 0, err = 0, len = strlen(name);
	int lfn_num = fat_get_lfns(len);
	struct fat_lfn tmps[lfn_num];

	memset(tmps, 0, sizeof(tmps));
	for(i=0 ; i < lfn_num; i++) {
		tmps[i].ord = i+1; 
		tmps[i].attr |= FAT_ATTR_LFN;
		tmps[i].crc = crc;
		
		err = _fat_crt_lfn((u8 *)(tmps+i), name, i);
	}
	
	fat_pad_lfn(tmps+i-1, len);	
	tmps[i-1].ord |= FAT_LAST_LONG_ENTRY;

	memcpy(lfns, tmps, sizeof(tmps));

	return 0;
}

static int fat_comb(const struct fat_lfn *lfns, const struct fat_sfn *sfn, struct fat_lfn * rde, u32 n)
{
	int i;	
			
	for(i=0 ; i<n ;i++) {
		memcpy(rde+i, lfns+(n-i-1), sizeof(struct fat_lfn));	
	}

	memcpy(rde+n, sfn, sizeof(struct fat_sfn));
	return 0;
}

// return cluster offset in data region
static int fat_alloc_clus(struct fat_region *rgn)
{	
	int ost = bitmap_get_free(&rgn->bit);
	if(!rgn->frees) {
		debug("Exhaust all clusters in data region.\n");
		return -ENOMEM;
	}

	if(ost < 0)
		return -1;

	if(bitmap_set(&rgn->bit, ost, FAT_CLUS_USED) < 0)
		return -1;

	(rgn->frees)--;

	return ost;	
}

/*
 * If the path passed by user doesn`t have directory path, add it to front filename. For example, if the path is just `name.txt`, add the current directory path to front of `name.txt`.   
 * */
static int fat_add_d2p(char *path)
{
	int i;
	char *tmp = NULL;

	if(!path)
		return -1;

	tmp = strchr(path, '/');
	if(!tmp)
		return 0;

	//strcat(fmm.cd->f.name, path);

	return 0;
}

/*
 * This function add a file to directory. 
 * */
static int fat_add_f2d(struct fat_dir *dir, const struct fat_file *file)
{
	struct fat_dir dir1;
	void *rde;
	int n;
	if(!rde || n<1)
		return -1;
/*
	if(fmm.r.cp + (n*FAT_ENTRY_SIZE) > fmm.r.bp + (fmm.r.clus * fat.rsn * fat.bps))
		return -1;
*/
	memcpy(fmm.c.cp, rde, n*FAT_ENTRY_SIZE);	
	fmm.c.cp += n*FAT_ENTRY_SIZE;
	
	return 0;
}

static int fat_parse_path(const char *path, char **dir, char **name)
{
	if(!path)
		return -1;

	*name = strrchr(path, '/');	
	if(!(*name))
		return 0;
	
	(*name)++;
	
	strncpy(dir, path, *name-path);
	*dir = strrchr(dir, '/');	
	if(!(*dir))
		return 0;

	// The strrchr function returns address of delim included.
	// For example, you passed `123/456/tmp.txt` into strrchr and it returns `/tmp.txt`. So, i think there is need to remove the delim. 	
	(*dir)++;

	return 0;
}

static int fat_parse_lfn(u8 *base, int ost, char *name)
{
	int i = 0, j = 0;
	u8 tmp[FAT_LFN_MAX_LEN];
	u8 *ent = base+((ost-1)*FAT_ENTRY_SIZE);
	
	// 아래 코드에서 LFN의 패딩으로 존재하는 0xFFFF 코드까지 모두 복사한다.
	// 하지만 상관없다. C언어의 스트링 처리들은 NULL 종료 기반이기 때문에 큰 무리가 없을 것으로 판단된다.
	memset(tmp, 0, sizeof(tmp));
	for(i=0 ; i<ost ; i++) {
		for(j=0 ; j<F32_LFN_LEN ; j++) {
			tmp[(i*F32_LFN_LEN)+j] = ent[lfn_ost[j]];		
		}
		
		ent -= FAT_ENTRY_SIZE;
	}

	memcpy(name, tmp, ost*F32_LFN_LEN);

	return 0;
}

static int fat_parse_sfn(u8 *base, char *name)
{
	
	return 0;
}

// LFN이 있으면 SFN은 필요가 없다. LFN이 없을 때만, SFN을 사용한다.
static int fat_parse_name(u8 *base, int ost, struct fat_file *file)
{
	if(!base || !file)
		return NULL;

	if(ost < -1)
		return -1;

	if((*base) & FAT_LAST_LONG_ENTRY) {
		fat_parse_lfn(base, ost, file->name);
	} else {
		fat_parse_sfn(base, file->name);
	} 

	return 0;
}

static int fat_conv_e2f(u8 *base, int ost, struct fat_file *file)
{
	struct fat_sfn *sfn;
	int err = -1;

	if(!base || !file)
		return -1;

	if(ost < 0)
		return -2;

	err = fat_parse_name(base, ost, file);
	if(err)
		return -3;

	sfn = (struct fat_sfn *)base+ost;
	file->attr = sfn->attr;
	file->clus[0] = sfn->fst_clus_hi<<16 | sfn->fst_clus_lo;

	return 0;
}

static int fat_conv_f2e(void *de, struct fat_file *file)
{
	int err, lfn_num, len;
	struct fat_sfn sfn;
	len = strlen(file->name);
	lfn_num = fat_get_lfns(len);

	struct fat_lfn lfns[lfn_num];

	memset(lfns, 0, sizeof(lfns));
	memset(&sfn, 0, sizeof(struct fat_sfn));
	err = fat_crt_sfn(file, &sfn);
	if(err < 0)
		return -2;

	/* calcuate a checksum */
	err = fat_crt_lfn(file->name, file->crc, &lfns);
	if(err < 0)
		return -3;

	/* 
	 * `Microsoft Word - FAT32 Spec _SDA Contribution_.doc` "6.5 Directory creation"
	 * At least one cluster must be allocated - the contents of the `DIR_FstClusLO` and `DIR_FstClusHI` fields must refer to the first allcated cluster number.
	 * 
	 */
	sfn.fst_clus_hi = ((file->clus)[0]>>16) & 0xFFFF;
	sfn.fst_clus_lo = (file->clus)[0] & 0xFFFF;

	err = fat_comb(lfns, &sfn, de, lfn_num);	
	if(err)
		return -4;

	return lfn_num+1;
}

int fat_sync()
{
	struct fat_file *file;
	struct list_node *node;
	int err = -1, ost = 0;
	void *de, *base;		
	
	de = mm_alloc(fat.ds*4, MM_KL);
	if(!de)
		return -1;

	base = de;
	memset(de, 0, fat.ds);
	list_for_each(&fmm.cd->fl, node) {
		file = container_of(node, struct fat_file, li);
		ost = fat_conv_f2e(de, file);
		if(ost < 1)
			return -1;

		de+=(ost*FAT_ENTRY_SIZE);
	}

	err = fat_write_dir(0, base);
	if(err)
		return -2;

	mm_free(base);

	return 0;
}

static int fat_add_e2d(const void *entry, const char *name)
{
	struct bst_node *node;
	struct fat_dir *dir;

	if(!entry || !name)
		return -1;

	node = bst_search(&root.node, name, fat_search_comp);
	if(!node)
		return -2;

	dir = container_of(node, struct fat_dir, node);
	
		
}

// This function returns offset of root directory from after added file.
static int fat_crt(const char *path, struct fat_file *file, struct fat_region *rgn)
{
	int err, crc, clus, len = strlen(path)+1;
	char *dir = NULL, *name = NULL;
	char full_path[strlen(path)+1];

	memset(full_path, 0, len);
	full_path[0] = '/';
	strncpy(full_path+1, path, len-1);

	memset(name, 0, FAT_LFN_MAX_LEN);
	memset(dir, 0, FAT_LFN_MAX_LEN);
	err = fat_parse_path(full_path, &dir, &name);
	if(err)
		return err;

	/* calcuate a checksum */
	crc = fat_calc_crc((u8 *)name);

	/* allocate clusters */
	clus = fat_alloc_clus(rgn);
	if(clus < 0)
		return -1;

	file->clus = mm_alloc(sizeof(u32), MM_KL);
	if(!file->clus)
		return -1;

	/* Create a logical(software) file with inforamtion */
	(file->clus)[0] = clus;
	file->crc = crc;

	memcpy(file->name, name, strlen(name));

	list_add(&fmm.cd->fl, &file->li);
	(fmm.cd->num)++;

	return clus;
}

int fat_crt_file(const char *path)
{
	struct fat_file file;

	memset(&file, 0, sizeof(struct fat_file));

	file.attr |= FAT_ATTR_ARCHIVE;
	fat_crt(path, &file, &fmm.c);	
	
	return 0;
}

static int _fat_crt_dir(const char *path, struct fat_dir *dir, const u8 attr)
{
	struct fat_dir *tmp = NULL;
	int clus = 0; // root directory offset

	if(!path)
		return -1;	

	/*
	 * Add a character `/` to The end of directory path. 
 	* */

	tmp = mm_alloc(sizeof(struct fat_dir), MM_KL);
	if(!tmp)
		return -2;

	memset(tmp, 0, sizeof(struct fat_dir));

	/* 
	 * `Microsoft Word - FAT32 Spec _SDA Contribution_.doc` "6.5 Directory creation"
	 * The `ATTR_DIRECTORY` bit must be set to 0
	 * The `DIR_FileSize` must be set to 0
	*/ 
	(tmp->f).attr = FAT_ATTR_DIRECTORY;
	(tmp->f).attr |= attr;
	(tmp->f).size = 0;

	clus = fat_crt(path, &tmp->f, &fmm.r);
	if(clus < 0)
		return -5;

	if(dir)
		memcpy(dir, tmp, sizeof(struct fat_dir));

	return 0;
}

int fat_crt_dir(const char *path)
{
	_fat_crt_dir(path, NULL, 0);
}

int fat_write()
{

}

int _fat_read(const char *path)
{
			
}

static int fat_parse_entry(const void* base, struct fat_file *files)
{
	int err = -1, ost = 0, n = 0;
	u8 *ent = NULL;

	if(!base || !files)
		return -1;

	for(ent=base ; *ent ; ent+=((ost+1)*FAT_ENTRY_SIZE)) {
		ost = fat_get_ost(ent);
		
		if(FAT_FREE(*ent))
			continue;	
	
		// parse data from disk and convert entry into logical file data.
		fat_conv_e2f(ent, ost, files+n);	

		fat_debug("n#%d, addr#0x%x, name#%s attr#0x%x, clus#%d\n", n, ent, files[n].name, files[n].attr, files[n].clus[0]);
		n++;	
	}
	
	return n;
}
/*
 * This function reads a directory entry data from secodary storage and parse it. 
 * The time to call this function is just on `cd command`.
 */ 
int fat_parse_dir(struct fat_dir *dir)
{
	struct fat_file tmps[FAT_LFN_MAX_LEN];
	int err = -1, i = 0;
	u8 *base = NULL;

	if(!dir)
		return -1;

	base = mm_alloc(fat.ds, MM_KL);
	if(!base)
		return -2;

	memset(tmps, 0, sizeof(tmps));
	memset(base, 0, sizeof(fat.ds));
	err = fat_read_dir(dir->f.clus[0], base);
	if(err < 0)
		return err;

	err = fat_parse_entry(base, tmps);
	if(err < 0)
		return -10;

	dir->num = err;
	for(i=0 ; i<dir->num ; i++)
		list_add(&dir->fl, tmps+i);
	
	return 0;
}

int fat_init_rootfs()
{
	void *ent;
	int err = -1;

	memset(&root, 0, sizeof(struct fat_dir));
	err = _fat_set_dir(&root); // set current directory to root.
	if(err < 0)
		return -1;
	
	if(!stack_init(&stk, sizeof(struct fat_dir)*200))
		return -3;
	
	ent = mm_alloc(fat.ds, MM_KL);
	if(!ent)
		return -7;

	err = _fat_crt_dir("$", &root, FAT_ATTR_SYSTEM);
	if(err)
		return -10;

	//bst_root_init(&root.node);

	err = fat_conv_f2e(ent, &root.f);
	if(err < 0)
		return -13;

	err = fat_write_dir(-1, ent);
	if(err < 0)
		return -15;

	mm_free(ent);	

	list_init_head(&root.fl);
	err = fat_parse_dir(&root); // parse root directory 
	if(err < 0)
		return -21;

	//fat_crt_dir_tree(&root.node);
	
	return 0;
}

int fat_remove(struct fat_file *file)
{

}

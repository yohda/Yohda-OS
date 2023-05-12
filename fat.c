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

	// Cluster(Data)
	u32 cfs;		// cluster region first sector
	u32 csn;		// cluster region sector number
	u32 cba;		// cluster region base address

	bool lfn;		// supported lfn
};

// 각 디렉토리는 클러스터 사이즈만큼의 루트 디렉토리 영역에 메모리를 할당받는다. 만약, 특정 디렉토리가 할당받은 영역을 초과하면 아래의 rcp가 가리키는 곳에 새롭게 클러스터 사이즈만큼 영역을 할당 받는다.
struct fat_mem {
	struct fat_dir cd; // current directory
	struct bst_node root;
	
	// FAT memory management
	u32 f_size;
	void *fbp, *fcp;
	struct bitmap fbit;

	// root directory memory management 
	u32 r_size;
	void *rbp, *rcp;
	struct bitmap rbit;
	
	// cluster(data) memory management
	u32 c_size;
	void *cbp, *ccp;
	struct bitmap cbit;
};

struct fat_mem fmm;
struct fat_info fat;

static int fat_read_rr(u32 ost, int size, void *to)
{
	int sc = (size + (fat.bps-1)) / fat.bps;	
	return ahci_read(fat.rfs + ost, sc, to);
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
	fat_debug("DATA first sector#%d\n", fat.cfs);

	// Each region size of region
	fat_debug("FAT Region sector number#%d\n", fat.fsn);
	fat_debug("Root Directory Region Sector Number#%d\n", fat.rsn);
	fat_debug("Data Region Sector Number#%d\n", fat.csn);
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

	fat.cfs = fat.ffs + fat.fsn + fat.rsn; 
		
	// Root Directory
	if(fat.type == TYPE_FAT32)
		fat.rfs = ((ext_bpb.root_cluster - 2) * bpb.spc) + fat.cfs - fat.rsn;

	fat.csn = fat.ts - fat.cfs;	
	fat.tc = fat.csn / fat.spc;

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

	// root directory memory management 
	fmm.r_size = fat.ds;
	fmm.rbp = mm_alloc(fmm.r_size, MM_KL);
	fmm.rcp = fmm.rbp;
	
	/* bitmap initialize */ 
	// There is exception case about remainder... you must to check bitmap leaks. 
	bitmap_alloc(&fmm.fbit, 32, ((fat.fsn/fat.spc)+31)/32);
	bitmap_alloc(&fmm.rbit, 32, ((fat.rsn/fat.spc)+31)/32);
	bitmap_alloc(&fmm.cbit, 32, ((fat.csn/fat.spc)+31)/32);

	/* Initialize rootfs */
	fat_init_rootfs();
	
	/* Print FAT32 information  */
	fat_print_fat();
}

int fat_init_rootfs()
{
	fat_read_dir("123");
	//fat_set_dir(0);

}

void fat_show_dir()
{
	int i;

	for(i=0 ; i < fmm.cd.num ;i++) {
		msg("%s ", fmm.cd.files[i].name);
	}	
	msg("\n");
}

void fat_get_cd()
{
	
}

static int fat_get_fn()
{

}

static int fat_get_ost(u8 *ent)
{	
	bool is_lfn;
	if(!ent)
		return NULL;
	
	is_lfn = ((*ent) & FAT_LAST_LONG_ENTRY) == FAT_LAST_LONG_ENTRY;
	return is_lfn ? ((*ent) & 0xBF) + 1 : 1;
}

int fat_set_dir(u32 ost, char *path)
{
	// allocate a cluster per directory in yohdaOS.
	u8 *rr = (u8 *)mm_alloc(fat.ds, MM_KL);
	u8 *tmp, *name;
	struct fat_file files[20];
	u8 cnt, i, j, k;
	
	// rootfs offset is 0 from root dir.
	//fat_read_rr(ost, fat.spc, rr, 0);
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

	fmm.cd.num = i;
	fmm.cd.files = mm_alloc(sizeof(struct fat_file) * i, MM_KL);
	for(i=0 ; i < fmm.cd.num; i++) {
		memcpy(&(fmm.cd.files[i]), &files[i], sizeof(struct fat_file));		
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

static int fat_val_sfn(char *name)
{
	int i;
	struct fat_file *fs = fmm.cd.files;
/*	
	for(i=0; i < fmm.cd.num ; i++) {
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

static int fat_set_props(struct fat_sfn *sfn)
{
	struct fat_sfn tmp;
	
	tmp.crt_time = 0;
	tmp.crt_date = 0;
	tmp.wrt_time = tmp.crt_time;
	tmp.wrt_date = tmp.crt_date;	

	fat_link_clus(&tmp);
	fat_set_tm_dt(&tmp);
	
	return 0;
}

static int fat_crt_sfn(char *name, struct fat_sfn *sfn)
{	
	int i, err;
	struct fat_sfn tmp;

	err = fat_val_sfn(name);	
	if(err)
		return err;

	err = _fat_crt_sfn(name, &tmp);	
	if(err)
		return err;

	err = fat_set_props(&tmp);
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
	
	fat_pad_lfn(tmps+(i-1), len);	
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

static int fat_alloc_clus()
{
		
}

static int fat_crt_file(const char *name)
{
	struct fat_file file;
	struct fat_sfn sfn;

	sfn.attr |= FAT_ATTR_ARCHIVE;

	fat_crt(name);	
}

static int fat_crt_dir(const char *name)
{
	struct fat_dir dir;
	struct fat_sfn sfn;

	if(fmm.rcp + fat.ds > fmm.rbp + fmm.r_size) {
		debug("Allocaed memory size of root directory exceed.\n");
		return -ENOMEM;
	}

	// `Microsoft Word - FAT32 Spec _SDA Contribution_.doc` "6.5 Directory creation" 
	// The `ATTR_DIRECTORY` bit must be set to 0
	// The `DIR_FileSize` must be set to 0
	sfn.attr |= FAT_ATTR_DIRECTORY;
	sfn.file_size = 0;

	// Whenever created a directory, allocate a cluster to it.
	fmm.rcp += fat.ds;
	dir.rbp = fmm.rcp;
	dir.rcp = dir.rbp;
		
	fat_crt(name);
}

int fat_crt(const char *name)
{
	int err, lfn_num, len, crc;
	void *rde; // root directory entry
	struct fat_file file;

	len = strlen(name);
	lfn_num = fat_get_lfns(len);

	struct fat_lfn lfns[lfn_num];
	struct fat_sfn sfn;

	memset(lfns, 0, sizeof(lfns));
	memset(&sfn, 0, sizeof(sfn));
	rde = mm_alloc(sizeof(lfns) + sizeof(sfn), MM_KL); 
	if(!rde)
		return -1;

	memset(rde, 0, sizeof(lfns) + sizeof(sfn));
	err = fat_crt_sfn(name, &sfn);
	if(err)
		return err;

	crc = fat_calc_crc((u8 *)(sfn.name));
	err = fat_crt_lfn(name, crc, &lfns);
	if(err)
		return err;

	err = fat_comb(lfns, &sfn, rde, lfn_num);	
	if(err | !rde)
		return -1;

	err = fat_write_rr(1, rde);
	free(rde);
	if(err)
		return err;
}

int fat_write()
{

}

static void fat_parse_path()
{
	
}

int _fat_read(const char *path)
{
			
}

static int fat_parse_lfn(u8 *base, int ost, int n, char *name)
{
	int i = 0, j = 0;
	u8 tmp[FAT_LFN_MAX_LEN];
	u8 *ent = base+(ost*FAT_ENTRY_SIZE);
	
	// 아래 코드에서 LFN의 패딩으로 존재하는 0xFFFF 코드까지 모두 복사한다.
	// 하지만 상관없다. C언어의 스트링 처리들은 NULL 종료 기반이기 때문에 큰 무리가 없을 것으로 판단된다.
	memset(tmp, 0, sizeof(tmp));
	for(i=0 ; i<n; i++) {
		for(j=0 ; j<F32_LFN_LEN ; j++) {
			tmp[(i*F32_LFN_LEN)+j] = ent[lfn_ost[j]];		
		}
		
		ent -= FAT_ENTRY_SIZE;
	}

	memcpy(name, tmp, n*F32_LFN_LEN);

	return 0;
}

static int fat_parse_sfn(u8 *base, char *name)
{
	
	return 0;
}

// LFN이 있으면 SFN은 필요가 없다.LFN이 없을 때만, SFN을 사용한다.
static void fat_parse_name(u8 *base, int ost, struct fat_file *file)
{
	if(!base)
		return NULL;

	if(ost < -1)
		return -1;

	if((*base) & FAT_LAST_LONG_ENTRY) {
		fat_parse_lfn(base, ost-2, ost, file->name);
	} else {
		fat_parse_sfn(base, file->name);
	} 

	return 0;
}

static int fat_alloc_dir(struct fat_file files, int n)
{
	int i;
	for(i=0 ; i<n ; i++) {
	
	}	
}

// name으로 전달된 경로의 파일들을 쉘에 뿌리는 역할.
int fat_read_dir(const char *name)
{
	int ost = 0, n = 0;
	u8 *base = mm_alloc(fat.ds, MM_KL), *ent;
	u32 end = (u32)(base+(fat.ds));
	struct fat_file files[FAT_LFN_MAX_LEN];

	if(!base)
		return -1;
	
	memset(base, 0, sizeof(fat.ds));
	fat_read_rr(0, fat.ds, base);
	
	memset(files, 0, sizeof(files));
	for(ent=base ; (u32)(ent)<end ; ent+=(ost*FAT_ENTRY_SIZE)) {
		ost = fat_get_ost(ent);
		
		if(FAT_FREE(*ent))
			continue;	
		
		// PARSE
		fat_parse_name(ent, ost, files+n);
		
		fat_debug("ost#%d, addr#0x%x, name#%s\n", ost, ent, files[n].name);
		n++;	
	}

	fmm.cd.num = n;
	fmm.cd.files = mm_alloc(sizeof(struct fat_file)*n, MM_KL);
	
	memset(fmm.cd.files, 0, sizeof(struct fat_file)*n);
	memcpy(fmm.cd.files, files, n);	

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

//#include "Console.h"
#include "io.h"

#define err_dbg(err, fmt, ...) ({ 	\
		debug(fmt, ##__VA_ARGS__);	\
		err;		\
})

#define msg(fmt, ...) kPrintf(fmt, ##__VA_ARGS__);
#define debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define AHCI_DEBUGx
#ifdef AHCI_DEBUG
#define ahci_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define ahci_debug(fmt, ...) 
#endif

#define MM_DEBUG
#ifdef MM_DEBUG
#define mm_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define mm_debug(fmt, ...) 
#endif

#define PL_DEBUG
#ifdef PL_DEBUG
#define pl_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define pl_debug(fmt, ...) 
#endif

#define BUD_DEBUGx
#ifdef BUD_DEBUG
#define bud_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define bud_debug(fmt, ...) 
#endif

#define FAT_DEBUGx
#ifdef FAT_DEBUG
#define fat_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define fat_debug(fmt, ...) 
#endif

void mdebug(void *addr, int n);

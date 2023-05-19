#include "Console.h"

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

#define MM_DEBUGx
#ifdef MM_DEBUG
#define mm_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define mm_debug(fmt, ...) 
#endif

#define FAT_DEBUG
#ifdef FAT_DEBUG
#define fat_debug(fmt, ...) kPrintf("[f:%s][l:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define fat_debug(fmt, ...) 
#endif

void mdebug(void *addr, int n);

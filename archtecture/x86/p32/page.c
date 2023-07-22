#include "page.h"
#include "sys.h"
#include "debug.h"
#include "vmm.h"

#define PAGE_MAX_ENTRY 1024
#define PAGE_TABLE_SIZE 256 // For 1GB

//extern union pde pdes[PAGE_MAX_ENTRY];
//extern union pde *pdes;
//static union pte __attribute__((aligned(PAGE_MAX_ENTRY*4))) ptes[PAGE_TABLE_SIZE][PAGE_MAX_ENTRY];

//static union pte __attribute__((aligned(PAGE_MAX_ENTRY*4))) tests[PAGE_TABLE_SIZE][PAGE_MAX_ENTRY];

/*
 * Reload page with size 4KB 
 * */
/*
int page_init(void)
{
	uint32_t page_dir_offset = vmm_get_dir_offset((uint32_t)&_virt_base);
	int i, j, n = PAGE_SIZE;
	
	// Check alignment
	if(((uint32_t)pdes & 0xFFF) || ((uint32_t)ptes & 0xFFF))
		return -1;

	vmm_init();

	memset(ptes, 0, sizeof(ptes));
	for(i=0; i<PAGE_TABLE_SIZE; i++) {
		for(j=0; j<PAGE_MAX_ENTRY; j++) {
			ptes[i][j].entry = (n|P+RW);
			n += PAGE_SIZE;
		}
	}

	if(((uint32_t)tests & 0xFFF))
		return -1;
		
	memset(tests, 0, sizeof(tests));
	n = PAGE_SIZE;
	for(i=0; i<PAGE_TABLE_SIZE; i++) {
		for(j=0; j<PAGE_MAX_ENTRY; j++) {
			tests[i][j].entry = (n|P+RW);
			n += PAGE_SIZE;
		}
	}

	for(i=0; i<PAGE_TABLE_SIZE; i++) {
		pdes[i].entry = (uint32_t)vmm_virt_to_phy((((uint32_t)&tests[i])|P+RW));	
	}

	for(i=page_dir_offset; i<PAGE_MAX_ENTRY; i++) {
		pdes[i].entry = (uint32_t)vmm_virt_to_phy((((uint32_t)&ptes[i-page_dir_offset])|P+RW));	
	} 
	
}
*/

#include "cr.h"
 #include <malloc.h>


/* Initialize the whole memory system */
void mm_init(list *l) {
	// enable paging
	set_cr0(get_cr0() | 1<<32);
	// allocate 4k memory for kernel page directory
	int *PD = (int *)smemalign(4, 1024 * 4); // should mark as global
	set_cr3(PD);

	// allocate PTs
	int i =0;
	for (i = 0; i < 1024; ++i)
	{
		void *PTE = smemalign(4, 1024*4);
		lprintf("%p", PTE);
		*(PD + i) = PTE;		
	}
	int *k = semealign(4, 65536*8);
	for (i = 0; i < 65536; ++i)
	{
		
	}


}

void kerelmap() {

}


/* Map a virtual memory to physical memory */
uint32_t virtual2physical(uint32_t virtual) {

}

/* allocate memory and keep track the allocation */
uint32_t *allocate(size_t size) {

}

/* The reverse for allocate, free the memory out of the memory system */
void free(uint32_t *addr) {

}

void 


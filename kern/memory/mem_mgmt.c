#include "cr.h"
 #include <malloc.h>
#include "simics.h"
#include "mem_internals.h"
/* Initialize the whole memory system */
KF *mm_init() {

	// allocate 4k memory for kernel page directory
	uint32_t *PD = (uint32_t *)smemalign(4096, 1024 * 4); // should mark as global
	set_cr3((uint32_t)PD);
	lprintf("the pd uint32_t is %u", (unsigned int)PD);
	lprintf("the pd  is %p", PD);
	lprintf("cr3 is :%u", ((unsigned int)get_cr3()));
	// allocate PTs
	int i =0;
	for (i = 0; i < 1024; ++i)
	{
		void *PTE = smemalign(4096, 1024*4);
		
		*(PD + i) = (uint32_t)PTE | 0x107;	
		// lprintf("the page table is in %p", PTE);
		// lprintf("check we get the correct thign: %x", (unsigned int)*(PD + i));	
	}

	KF *frame_base = (KF*)smemalign(4096, 8 * 65536); // bunch of pointers that points to pages

	for (i = 0; i < 65536; ++i)
	{
		(*(frame_base + i)).flag=0;

		(*(frame_base + i)).address = (void *)(0x0+i*4096);
	}

	lprintf("the address for frame is %p", frame_base);

	// maping kernel address spaces

	// map first 4 entries in page directory
	int j;
	for ( i = 0; i < 4; ++i)
	{	
		uint32_t PT = *(PD + i)& 0xFFFFF000;
		lprintf("the pt is %u", (unsigned int)PT);
		for (j = 0; j < 1024; ++j)
		{
			uint32_t k = (uint32_t)((*(frame_base + 1024*i+j)).address);
			(*(frame_base + 1024*i+j)).flag = 1;
			// lprintf("the page address is %x", (unsigned int)k);
			*((uint32_t*)(PT) + j) =  k| 0x107;
		}
	}



	// enable paging
	lprintf("the cr0 %u", (unsigned int)get_cr0);
	set_cr4(get_cr4() | CR4_PGE);
	set_cr0(get_cr0() | CR0_PG);
	// MAGIC_BREAK;
	return frame_base;
}

void kerelmap() {

}


/* Map a virtual memory to physical memory */
uint32_t virtual2physical(uint32_t virtual_addr) {

	return 0;
}

/* allocate memory and keep track the allocation */
uint32_t *allocate(size_t size) {
	return 0;
}

/* The reverse for allocate, free the memory out of the memory system */
void freeff(uint32_t *addr) {

}



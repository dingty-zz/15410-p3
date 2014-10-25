#include "cr.h"
#include <malloc.h>
#include "simics.h"
#include "mem_internals.h"

static KF *frame_base;
/* Initialize the whole memory system */
KF *mm_init()
{

    // allocate 4k memory for kernel page directory
    PD* kern_PD = (PD *)smemalign(4096, 1024 * 4); // should mark as global
    PT** kern_pd = kern_PD -> pd;
    set_cr3((uint32_t)kern_pd);
    lprintf("the pd uint32_t is %u", (unsigned int)kern_pd);
    lprintf("the pd  is %p", kern_pd);
    lprintf("cr3 is :%u", ((unsigned int)get_cr3()));
    //initialize the first four entries of the kernel pd    
    int i;
    for (i = 0; i < 4; ++i)
    {
        kern_pd[i] = (PT*)smemalign(4096, 1024 * 4);
        // lprintf("the page table is in %p", PTE);
        // lprintf("check we get the correct thign: %x", (unsigned int)*(PD + i));
    }
    //initizlie the free frame array
    frame_base = (KF *)smemalign(4096, 8 * 65536); // bunch of pointers that points to pages
    for (i = 0; i < 65536; ++i)
    {
        frame_base[i].flag = 0;
        frame_base[i].address = (void *)(0x0 + i * 4096);
    }

    lprintf("the address for frame is %p", frame_base);

    // map first 4 entries in page directory, for kernel address space
    int j;
    for ( i = 0; i < 4; ++i)
    {
        void** current_pt = kern_pd[i]->pt;
        kern_pd[i] = (PT *) ((uint32_t)(kern_pd[i]) | 0x107);
        lprintf("the pt is %x", (unsigned int)current_pt);
        for (j = 0; j < 1024; ++j)
        {
            uint32_t k = (uint32_t)(frame_base[1024 * i + j].address);
            frame_base[1024 * i + j].flag = 1;
            // lprintf("the page address is %x", (unsigned int)k);
            *((uint32_t *)(current_pt) + j) =  k | 0x103;
        }
    }

    // enable paging
    lprintf("the cr0 %u", (unsigned int)get_cr0);
    set_cr4(get_cr4() | CR4_PGE);
    set_cr0(get_cr0() | CR0_PG);
    // MAGIC_BREAK;
    return frame_base;
}

void kerelmap()
{

}


/* Map a virtual memory to physical memory */
uint32_t virtual2physical(uint32_t virtual_addr)
{

    return 0;
}

/* Map this virtual address to a physical page */
void allocate_page(uint32_t virtual_addr, size_t size)
{
    int i ;
    lprintf("the address is %x", (unsigned int)virtual_addr);

    uint32_t pd = virtual_addr >> 22;
    lprintf("the pd is %x", (unsigned int)pd);
    uint32_t pt = (virtual_addr & 0x3ff000) >> 12;
    lprintf("the pt is %x", (unsigned int)pt);

    uint32_t offset = (virtual_addr & 0xfff) + (uint32_t)size;
    lprintf("the offset is %x", (unsigned int)offset);

    uint32_t times = offset % 4096 == 0 ? offset / 4096 : offset / 4096 + 1;
    if (times ==0)
    {
        return;
    }
    int j = 0;
    for (i = 0; i < 65536; ++i)
    {
        if (frame_base[i].flag == 0)
        {
            lprintf("times to allocation a page: %u", (unsigned int)times);
            times--;
            frame_base[i].flag = 1;

            uint32_t *PD = (uint32_t *)get_cr3();
            uint32_t pde = PD[pd];
            lprintf("page table entry: %x", (unsigned int)pde);

            uint32_t *PT = (uint32_t *)(pde & 0xfffffff8);
            lprintf("page table entry: %x", (unsigned int)PT);

            PT[pt + j] = (uint32_t)frame_base[i].address | 0x7;
            lprintf("this is adjusted address %x  ", (unsigned int)PT[pt + j]);
            pde |= 0x7;
            lprintf("pde: %x", (unsigned int)pde);
            PD[pd] = pde;
            j++;
            // break;
            if (times == 0)
            {
            	lprintf("finally this is pde: %x", (unsigned int)PD[pd]);
            	// lprintf("finally this is pte: %x", (unsigned int)PD[pd]+pt);
                break;
            }

        }
    }


}

// void allocate_page(uint32_t virtual_addr, size_t size) {
//  int i ;
//  uint32_t pd = virtual_addr >> 22;
//  uint32_t pt = virtual_addr & 0x3ff000;
//  // uint32_t offset = virtual_addr & 0x111;
//  for (i = 0; i < 65536; ++i)
//  {
//      if (frame_base[i].flag == 0) {
//          frame_base[i].flag = 1;
//          uint32_t *page_table = (uint32_t*)get_cr3()+pd;
//          *(page_table + pt) = (uint32_t)frame_base[i].address|0x107;
//          //lprintf("so therefore %x, %d", (unsigned int)*(page_table + pt), i);
//          break;
//      }
//  }


// }

/* The reverse for allocate, free the memory out of the memory system */
void freeff(uint32_t *addr)
{

}



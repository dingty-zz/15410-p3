/** @file mem_memt.c
 *
 *  @brief This file includes paging handling routines
*          1. General design, PD, PT descrptions
           2. How free list works
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#include <syscall.h>
#include "cr.h"
#include <malloc.h>
#include "simics.h"
#include "mem_internals.h"
#include "string.h"
static KF *frame_base;      // always fixed
static KF *free_frame;      // points to the first free frame where refcount = 0
void init_free_frame();
uint32_t acquire_free_frame();
void release_free_frame(uint32_t address);
int virtual_map_physical(PD *PD, uint32_t pd_index, uint32_t pt_index);
int virtual_unmap_physical(PD *PD, uint32_t pd_index, uint32_t pt_index);
/** @brief Initialize the whole memory system, immediately
 *         called when the kernel enters to enable paging
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
KF *mm_init()
{
    init_free_frame();

    // allocate 4k memory for kernel page directory
    PD *kern_PD = (PD *)smemalign(4096, 1024 * 4); // should mark as global
    PT **kern_pd = kern_PD -> pd;
    set_cr3((uint32_t)kern_pd);
    lprintf("the pd uint32_t is %u", (unsigned int)kern_pd);
    lprintf("the pd  is %p", kern_pd);
    lprintf("cr3 is :%u", ((unsigned int)get_cr3()));

    //initialize the first four entries of the kernel pd, note that
    // the first 4 pts (kernel pts)
    // are fixed at the beginning, it is then copied to the pts
    //for the first running program.
    int i;
    for (i = 0; i < 4; ++i)
    {
        kern_pd[i] = (PT *)smemalign(4096, 1024 * 4);
        // lprintf("the page table is in %p", PTE);
        // lprintf("check we get the correct thign: %x",
        //  (unsigned int)*(PD + i));
    }


    lprintf("the address for frame is %p", frame_base);

    // map first 4 entries in page directory, for kernel address space
    int j;
    for ( i = 0; i < 4; ++i)
    {
        void **current_pt = kern_pd[i]->pt;
        kern_pd[i] = (PT *) ((uint32_t)(kern_pd[i]) | 0x107);
        lprintf("the pt is %x", (unsigned int)current_pt);
        for (j = 0; j < 1024; ++j)
        {
            uint32_t k = acquire_free_frame();
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

/* Map an unmapped virtual memory to physical memory */
int virtual_map_physical(PD *PD, uint32_t pd_index, uint32_t pt_index)
{
    lprintf("In virtual2physical:");

    // Now cr3 may points to a process's PD
    uint32_t pde = PD[pd_index];

    if (pde != 0)
    {
        uint32_t *PT = (uint32_t *)(pde & 0xfffffff8);
        lprintf("page table address: %x", (unsigned int)PT);

        uint32_t pte = PT[pt_index];
        if (pte == 0)
        {
            uint32_t free_frame_addr = acquire_free_frame();
            PT[pt_index] = free_frame_addr | 0x7;
        }
        else return -1;     // Already mapped

    }
    else
    {
        uint32_t free_frame_addr = acquire_free_frame();
        uint32_t *PT = (PT *)smemalign(4096, 1024 * 4);
        lprintf("page table addr: %x", (unsigned int)PT);

        PT[pt_index] = free_frame_addr | 0x7;
        lprintf("this is physical address %x  ", (unsigned int)PT[pt_index]);

        PD[pd_index] = ((uint32_t)PT) | 0x7;
    }

    lprintf("The freed address is %x", (unsigned int)(pd_index << 22 | pt_index << 12));
    /* ZFOD the virtual address but not the physical address */
    memset((void *)(pd_index << 22 | (pt_index) << 12), 0, 4096);
    lprintf("Return virtual2physical");
    return 0;
}

/* Unmap an mapped virtual memory to physical memory */
int virtual_unmap_physical(PD *PD, uint32_t pd_index, uint32_t pt_index)
{
    lprintf("In virtual_erase_physical:");

    // Now cr3 may points to a process's PD
    uint32_t pde = PD[pd_index];

    if (pde == 0)
    {
        return -1;        // Ok, this virtual memory is already unmapped
    }
    else
    {
        uint32_t *PT = (uint32_t *)(pde & 0xfffffff8);
        lprintf("page table address: %x", (unsigned int)PT);

        uint32_t pte = PT[pt_index];
        lprintf("page table entry: %x", (unsigned int)pde);

        if (pte == 0)
        {
            return -1;    // Ok, this virtual memory is already unmapped
        }
        else
        {
            uint32_t physical_addr = (uint32_t *)(pte & 0xfffffff8);
            release_free_frame(physical_addr);
            PT[pt_index] = 0;       // Unmap this page
        }
    }
    return 0;
}

/** @brief Map this virtual address to a physical pages, for kernel use
 *
 *  Note for very COMPLEX cases, when the size is excessively
 LARGE, we have to handle the
 *  case when allocation accross multiple page tables!!!!!!!!!!!!!!!!!!!!!!!
 *   Also, if the address is already mapped, we just return
 *
 *  @param q The pointer to the queue
 *  @return 0 on success, -1 error
 **/
void allocate_pages(PD *pd, uint32_t virtual_addr, size_t size)
{

    int i = 0;
    lprintf("the address is %x", (unsigned int)virtual_addr);

    uint32_t pd_index = virtual_addr >> 22;
    lprintf("the pd is %x", (unsigned int)pd_index);
    uint32_t pt_index = (virtual_addr & 0x3ff000) >> 12;
    lprintf("the pt is %x", (unsigned int)pt_index);

    uint32_t total_size = (virtual_addr & 0xfff) + (uint32_t)size;
    lprintf("the total_size is %x", (unsigned int)total_size);

    uint32_t times = total_size % 4096 == 0 ? total_size / 4096 : total_size / 4096 + 1;
    lprintf("times to allocation a page: %u", (unsigned int)times);

    for (i = 0; i < times; ++i)
    {
        virtual_map_physical(pd, pd_index, pt_index + i);
    }
    lprintf("Finished allocation pages.");

}

/** @brief The reverse for allocate, free the memory out of the
memory system, for kernel use
 *
 *         Not only decrement the refcount, but also clear the page table entry
 *  Note for very COMPLEX cases, when the size is excessively
 LARGE, we have to handle the
 *  case when allocation accross multiple page tables!!!!!!!!!!!!!!!!!!!!!!!
 *    If the address is not mapped, we return -1
 *  @param q The pointer to the queue
 *  @return 0 on success, -1 error
 **/
void free_pages(uint32_t virtual_addr, size_t size)
{

    int i = 0;
    lprintf("the address is %x", (unsigned int)virtual_addr);

    uint32_t pd_index = virtual_addr >> 22;
    lprintf("the pd is %x", (unsigned int)pd_index);
    uint32_t pt_index = (virtual_addr & 0x3ff000) >> 12;
    lprintf("the pt is %x", (unsigned int)pt_index);

    uint32_t total_size = (virtual_addr & 0xfff) + (uint32_t)size;
    lprintf("the total_size is %x", (unsigned int)total_size);

    uint32_t times = total_size % 4096 == 0 ? total_size / 4096 : total_size / 4096 + 1;
    lprintf("times to allocation a page: %u", (unsigned int)times);
    for (i = 0; i < times; ++i)
    {
        virtual_unmap_physical(pd, pd_index, pt_index + i);
    }
    return ;
}

void init_pd(PD *pd)
{
    void *old_cr3 = (void *)get_cr3();
    pd = (PD *)smemalign(4096, 1024 * 4); // Allocate pd for process
    memset(pd, 0, 1024 * 4);  // clean
    memcpy((void *)pd, old_cr3, 4 * 4); // Copy kernel pt mapping
    set_cr3((uint32_t) pd);  // should we free old page directory?
}

void copy_page_directory(PD *pd)
{


}

void copy_page_table(PT *pt)
{


}

void destroy_page_directory(PD *pd)
{


}

void destroy_page_table(PT *pt)
{


}

















/** @brief Initialize the free list, which keeps track the current free frames.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
void init_free_frame()
{
    int i ;
    //initizlie the free frame array
    // bunch of pointers that points to pages
    frame_base = (KF *)smemalign(4096, 8 * 65536);
    for (i = 0; i < 65536; ++i)
    {
        frame_base[i].refcount = 0;
        if (i != 65535) frame_base[i].next = &(frame_base[i + 1]);
    }
    free_frame = frame_base;
}

/** @brief Returns a frame frame from the current free list and
set the next free list
 *         to be the struct where refcount = 0;
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return the physical free frame address, always 4KB aligned
 **/
uint32_t acquire_free_frame()
{
    uint32_t offset = (uint32_t)free_frame - (uint32_t)frame_base;
    uint32_t index = offset / 8;
    uint32_t physical_frame_addr = index * 4096;
    frame_base[index].refcount++;

    // Loop until free_frame points to a free physical page
    while (free_frame -> refcount != 0)
    {
        free_frame = free_frame -> next;
    }
    return physical_frame_addr;
    // Note that it's possible that there is no free physical
    // page, so in this case we
    // need to handle this situation
}

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical
  address and 4KB aligned (really ?)
 **/
void release_free_frame(uint32_t address)
{
    uint32_t index = address / 4096;
    frame_base[index].refcount--;
    // If there is no reference count, let free_frame point to it
    if (frame_base[index].refcount == 0)
    {
        *free_frame = frame_base[index];
    }
}


// parsing the virtual address
// struct virtual_addr parse(uint32_t virtual_addr);


/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both
 physical address and 4KB aligned (really ?)
 **/
int _new_pages(void *addr, int len)
{
    /* If either the address is invalid or len is not aligned,
     * return a negative number */
    if (addr == NULL ||                     // addr is null
            (uint32_t)addr < 0x01000000 ||      // addr is in kernel memory
            (((uint32_t)addr) & 0xfff) != 0 ||   // addr is not aligned
            len < 0)                         // len is negative
        return -1;

    return 0;
}

/** @brief Release a frame frame and mark it as
freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
int _remove_pages(void *addr)
{
    if (addr == NULL ||                     // addr is null
            (uint32_t)addr < 0x01000000 ||      // addr is in kernel memory
            ((uint32_t)addr & 0xfff) != 0)    // addr is not aligned
        return -1;

    return 0;

}
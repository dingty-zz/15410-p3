/** @file vm_routines.c
 *
 *  @brief This file includes paging handling routines
*          1. General design, PD, PT descrptions
           2. How free list works
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#include "vm_routines.h"
#include "cr.h"
#include <malloc.h>
#include "simics.h"
#include "mem_internals.h"
#include "string.h"
#include <common_kern.h>
#include "control_block.h"
#include <page.h>

#define PAGE_LEN (PAGE_SIZE>>2)             //1024
#define TOTAL_PHYS_FRAMES (PAGE_SIZE<<4)    //65536

static KF *frame_base;      // always fixed
static KF *free_frame;      // points to the first free frame where refcount = 0

/** @brief Initialize the whole memory system, immediately
 *         called when the kernel enters to enable paging
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
void mm_init()
{
    init_free_frame();

    // allocate 4k memory for kernel page directory
    uint32_t *kern_pd = (uint32_t *)memalign(PAGE_SIZE, 4 * 4);
    set_cr3((uint32_t)kern_pd);

    //initialize the first four entries of the kernel pd, note that
    //the first 4 pts (kernel pts)
    //are fixed at the beginning, it is then copied to the pts
    //for the first running program.
    //map first 4 entries in page directory, for kernel address space

    int i, j;
    for (i = 0; i < 4; ++i)
    {
        uint32_t current_pt = (uint32_t)memalign(PAGE_SIZE, PAGE_SIZE);
        for (j = 0; j < PAGE_LEN; ++j)
        {
            uint32_t k = acquire_free_frame();
            // should mark as global
            *((uint32_t *)(current_pt) + j) =  k | 0x103;
        }

        kern_pd[i] = current_pt | 0x107;
    }

    set_cr4(get_cr4() | CR4_PGE);
    set_cr0(get_cr0() | CR0_PG);

}

/** @brief Map an unmapped virtual memory to physical memory
 *
 *  Look for a valid physical address in the free frame list
 *
 *  @param page directory address, page directory index, and page table index
 *  @return 0 on success, -1 on failure
 **/
int virtual_map_physical(uint32_t *PD, uint32_t pd_index, uint32_t pt_index)
{
    // Now cr3 may points to a process's PD
    uint32_t pde = PD[pd_index];
    uint32_t free_frame_addr = 0;
    if (pde != 0)
    {
        uint32_t *PT = (uint32_t *)DEFLAG_ADDR(pde);

        uint32_t pte = PT[pt_index];

        if (pte == 0)
        {
            free_frame_addr = acquire_free_frame();
            if (free_frame_addr == 0)
            {
                return -1;
            }
            PT[pt_index] = free_frame_addr | 0x7;
        }
        else return -1;     // Already mapped

    }
    else
    {
        free_frame_addr = acquire_free_frame();
        if (free_frame_addr == 0)
        {
            return -1;
        }
    
        uint32_t *PT = (uint32_t *)memalign(PAGE_SIZE, PAGE_SIZE);
        memset((void *)PT, 0, PAGE_SIZE);

        PT[pt_index] = free_frame_addr | 0x7;

        PD[pd_index] = ((uint32_t)PT) | 0x7;
    }

    /* ZFOD the virtual address but not the physical address */
    memset((void *)(pd_index << 22 | pt_index << 12), 0, PAGE_SIZE);
    return 0;
}

/** @brief Unmap an mapped virtual memory from physical memory
 *
 *  Add back to the physial free frame list
 *
 *  @param page directory address, page directory index, and page table index
 *  @return 0 on success, -1 on failure
 **/
int virtual_unmap_physical(uint32_t *PD, uint32_t pd_index, uint32_t pt_index)
{
    uint32_t pde = PD[pd_index];
    //This virtual memory is already unmapped
    if (pde == 0)
    {
        return -1;        
    }
    else
    {
        /* Get the page table address*/
        uint32_t *PT = (uint32_t *)DEFLAG_ADDR(pde);
        uint32_t pte = PT[pt_index];
        /* Already mapped*/
        if (pte == 0)
        {
            return -1;   
        }
        else
        {
            uint32_t physical_addr = DEFLAG_ADDR(pte); 
            release_free_frame(physical_addr);
            // Unmap this page
            PT[pt_index] = 0;
        }
    }
    return 0;
}

/** @brief Map this virtual address to a physical pages, for kernel use
 *
 *  Note for very COMPLEX cases, when the size is excessively
 *  LARGE, we have to handle the case when allocation accross multiple
 *  page tables!
 *  Also, if the address is already mapped, we just return
 *
 *  @param page directory address, virtual address, and size to be allocated
 *  @return 0 on success, -1 error
 **/
int allocate_pages(uint32_t *pd, uint32_t virtual_addr, size_t size)
{
    int i = 0;
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr & 0x3ff000) >> 12;
    uint32_t total_size = (virtual_addr & 0xfff) + (uint32_t)size;
    uint32_t times = total_size % PAGE_SIZE == 0 ?
                     total_size / PAGE_SIZE : total_size / PAGE_SIZE + 1;
    for (i = 0; i < times; ++i)
    {
        int actual_offset = pt_index + i;
        int result = 0;
        virtual_map_physical(pd, pd_index + actual_offset / PAGE_LEN, 
                                 actual_offset % PAGE_LEN);
        if (result == -1)
        {
            return -1;
        }
    }
    return 0;

}

/** @brief The reverse for allocate, free the memory out of the
memory system, for kernel use
 *
 *  Not only decrement the refcount, but also clear the page table entry
 *  Note for very COMPLEX cases, when the size is excessively
 *  LARGE, we have to handle the case when allocation accross multiple page
 *  tables! If the address is not mapped, we return -1
 *  @param page directory address, virtual address, and size to be freed
 *  @return 0 on success, -1 error
 **/
int free_pages(uint32_t *pd, uint32_t virtual_addr, size_t size)
{

    int i = 0;
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr & 0x3ff000) >> 12;
    uint32_t total_size = (virtual_addr & 0xfff) + (uint32_t)size;
    uint32_t times = total_size % PAGE_SIZE == 0 ?
                     total_size / PAGE_SIZE : total_size / PAGE_SIZE + 1;
    for (i = 0; i < times; ++i)
    {
        int actual_offset = pt_index + i;
        int result=0;
        virtual_unmap_physical(pd, pd_index + actual_offset / PAGE_LEN, 
                                   actual_offset % PAGE_LEN);
        if (result == -1)
        {
            return -1;
        }
    }
    return 0;
}


/** @brief Initialize the page directory
 *
 *  Allocate a new page directory. We need to copy the whole kernel stack 
 *  memory mapping  
 *
 *  @param nothing
 *  @return the page directory address
 **/
uint32_t *init_pd()
{
    uint32_t *pd = (uint32_t *)memalign(PAGE_SIZE, PAGE_SIZE); // Allocate pd for process
    memset(pd, 0, PAGE_SIZE);  // clean
    int i = 0;
    for (i = 0; i < 4; ++i)
    {
        pd[i] = DEFLAG_ADDR((((uint32_t *)get_cr3())[i])) | 0x107;

    }
    //memcpy((void *)pd, old_cr3, 4 * 4); // Copy kernel pt mapping
    set_cr3((uint32_t) pd);

    //lprintf("after calling initpd, the pd is %x", (unsigned int)get_cr3());

    return pd;
}

/** @brief Destroy the page directory
 *
 *  @param page dorectory address
 *  @return nothing
 *  @bug this doesn't work that well so right now 
 *  we are not using it when vanishing
 **/
void destroy_page_directory(uint32_t *pd)
{
    int i;
    for (i = 4; i < PAGE_LEN; ++i)
    {
        if (pd[i]==0)
        {
            continue;
        }
        destroy_page_table(DEFLAG_ADDR(pd[i]));
        pd[i] = 0;
    }

}

/** @brief Destroy the page table
 *
 *  @param page table address
 *  @return nothing
 *  @bug this doesn't work that well so right now 
 *  we are not using it when vanishing
 **/
void destroy_page_table(uint32_t pt)
{
    int i;
    for (i = 0; i < PAGE_LEN; ++i)
    {
        uint32_t pte = ((uint32_t *)pt)[i];
        if (pte==0)
        {
            continue;
        }
        uint32_t physical_addr = DEFLAG_ADDR(pte);
        release_free_frame(physical_addr);
        ((uint32_t *)pt)[i] = 0;      // Unmap this page
    }
    sfree((uint32_t *)pt, PAGE_SIZE);
    return;
}


/** @brief Initialize the free frame list, which keeps track the
 *  current free frames, also explained in the readme.dox
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param nothing
 *  @return nothing
 **/
void init_free_frame()
{
    int i ;
    free_frame_num = machine_phys_frames();
    //initizlie the free frame array
    //bunch of pointers that points to pages
    frame_base = (KF *)memalign(PAGE_SIZE, 8 * TOTAL_PHYS_FRAMES);
    for (i = 0; i < TOTAL_PHYS_FRAMES; ++i)
    {
        frame_base[i].refcount = 0;
        if (i != TOTAL_PHYS_FRAMES-1) frame_base[i].next = &(frame_base[i + 1]);
    }
    free_frame = frame_base;
    return;
}

/** @brief Returns a frame frame from the current free list and
set the next free list to be the struct where refcount = 0;
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param nothing
 *  @return the physical free frame address, always 4KB aligned
 **/
uint32_t acquire_free_frame()
{
    if (free_frame == NULL)
    {
        return -1;
    }
    uint32_t offset = (uint32_t)free_frame - (uint32_t)frame_base;
    uint32_t index = offset / 8;
    uint32_t physical_frame_addr = index * PAGE_SIZE;
    frame_base[index].refcount++;

    // Loop until free_frame points to a free physical page
    while (free_frame -> refcount != 0)
    {
        free_frame = free_frame -> next;
    }

    free_frame_num --;
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
 *  @param address 
 *  @return nothing
 **/
void release_free_frame(uint32_t address)
{
    uint32_t index = address / PAGE_SIZE;

    frame_base[index].refcount--;
    // If there is no reference count, let free_frame point to it
    if (frame_base[index].refcount == 0)
    {
        free_frame = &frame_base[index];
    }

    free_frame_num++;
    return;
}


/** @brief Map the virtual address with size of size to be read only
 *
 *  @param page directory address
 *  @return nothing
 **/
void map_readonly(uint32_t *pd, uint32_t virtual_addr, size_t size)
{
    int i = 0;
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr & 0x3ff000) >> 12;

    uint32_t total_size = (virtual_addr & 0xfff) + (uint32_t)size;

    uint32_t times = total_size % PAGE_SIZE == 0 ?
                     total_size / PAGE_SIZE : total_size / PAGE_SIZE + 1;
    for (i = 0; i < times; ++i)
    {

        int actual_offset = pt_index + i;
        map_readonly_pages(pd, pd_index + actual_offset / PAGE_LEN, 
                               actual_offset % PAGE_LEN);
    }
}

/** @brief helper function for map_readonly, map this entry to be readonly
 *
 *  @param page directory, page directory index and page table index
 *  @return nothing
 **/
void map_readonly_pages(uint32_t *PD, uint32_t pd_index, uint32_t pt_index)
{
    uint32_t pde = PD[pd_index];
    uint32_t *PT = (uint32_t *)(pde & 0xfffff000);
    // Turn off read bit for page table entry
    PT[pt_index] &= 0xfffffffd;
    // Turn off read bit for page directory entry
    PD[pd_index] &= 0xfffffffd;

}


/** @brief check if an address is in user space
 *
 *  @param address
 *  @return 0 fail, a positive number on success */
int is_user_addr(void *addr) {
    if (addr == NULL) return 0;
    return ((unsigned int)addr >= 0x01000000);
}

/** @brief check if an address has mapping
 *
 *  @param address
 *  @return 0 fail, a positive number on success
 **/
int addr_has_mapping(void *addr) {
    if (addr == NULL) return 0;
    
    uint32_t *PD, *PT;
    PD = current_thread -> pcb -> PD;
    if (PD == NULL) return 0;

    uint32_t pd_index = ((uint32_t)addr) >> 22;
    uint32_t pt_index = ((uint32_t)addr & 0x3ff000) >> 12;
    PT = (uint32_t *) DEFLAG_ADDR(PD[pd_index]);
    /*No mapped page table*/
    if (PT == NULL) return 0;
    
    uint32_t pt_entry = DEFLAG_ADDR(PT[pt_index]);
    /*No mapped page table entry*/
    if (pt_entry == 0) return 0;
    /*passed all tests*/
    return 1;
}
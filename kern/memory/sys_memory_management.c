/** @file _memory_management.c
 *
 *  @brief This file defines process manipulation routines
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "vm_routines.h"
#include "control_block.h"
#include <stddef.h>
#include <malloc.h>
#include <simics.h>  
#include <page.h>

#define DEFLAG_ADDR(x)           (x & 0xfffff000)
#define ADDFLAG(x,flag)          (x | flag)
#define GET_FLAG(x)              (x & 0xfff)

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both
 *  @return 1 on success, 0 on failure
 **/
int sys_new_pages(void *addr, int len)
{

    /* step 1: check if valid to new_pages*/
    if (addr == NULL) 
        return -1;
    // addr is not page aligned;
    if (((uint32_t)addr & 0xfff) != 0)
        return -1;
    // len is not a positive multiple of the page size
    if (len < 0 || (len & 0xfff) != 0) 
        return -1;
    // portion reserved by kernel;
    if ((uint32_t)addr < 0x01000000)
        return -1;
    // If os has insufficient resources to satisfy the request
    int requested_page_num = len/4096;
    lprintf("requested:%d", requested_page_num);
    lprintf("now, I have this free: %d", free_frame_num);
    MAGIC_BREAK;
    if (requested_page_num > free_frame_num) return -1;
    // if any portion already in task's address space;
    int i;
    uint32_t cur_addr, cur_pd_index, cur_pt_index, phys_adddr;
    uint32_t *PD, *PT;
    PD = current_thread -> pcb -> PD;
    for (i=0; i<requested_page_num; i++)
    {
        cur_addr = (uint32_t)(addr)+PAGE_SIZE*i;
        cur_pd_index = ((uint32_t)addr) >> 22;
        cur_pt_index = ((uint32_t)addr & 0x3ff000) >> 12;
        PT = (uint32_t*) DEFLAG_ADDR(PD[cur_pd_index]);
        phys_adddr = DEFLAG_ADDR(PT[cur_pt_index]);
        /*check if already mapped*/
        if ( PT != NULL && phys_adddr != 0) return -1;
    }

    /* step 2: allocate*/
    lprintf("FINISHED CHECKING, all passed");
    MAGIC_BREAK;

    allocate_pages(PD, (uint32_t)addr, len);
    //Lastly, insert the va node into the list
    VA_INFO* current_va_info = malloc(sizeof(VA_INFO));
    current_va_info -> virtual_addr = (uint32_t)addr;
    current_va_info -> len = len;
    
    list_insert_last(&current_thread->pcb->va, &current_va_info->va_node);
    if ((uint32_t)addr == 0x40000000)
    {
        lprintf("HEREHEREHERE4");
        MAGIC_BREAK;
    }
    return 0;
}

/** @brief Release a frame frame and mark it as
freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical
 ddress and 4KB aligned (really ?)
 **/
int sys_remove_pages(void *addr)
{
    /* step 1: check if removable by inspecting virtual address*/
    // addr is null
    if (addr == NULL) 
        return -1;
    // addr is in kernel memory
    if ((uint32_t)addr < 0x01000000)
        return -1;
    // addr is not aligned
    if (((uint32_t)addr & 0xfff))
        return -1;

    /* step 2: check if removable by inspecting the mapped address's flags*/
    uint32_t* PD = current_thread -> pcb -> PD;
    uint32_t pd_index = ((uint32_t)addr) >> 22;
    uint32_t pt_index = ((uint32_t)addr & 0x3ff000) >> 12;
    uint32_t phys_addr_raw = ((uint32_t*)(PD[pd_index]))[pt_index];
    if ((phys_addr_raw & 0x7) != 0x7) return 0;

    /* step 3: search for the allocation info */
    node* current_node = list_begin(&current_thread->pcb->va);
    int current_len;
    uint32_t curent_virtual_addr;
    /* try to find the address allocation info in the list*/
    while (current_node!=NULL)
    {
        VA_INFO* current_struct = list_entry(current_node,VA_INFO,va_node);
        curent_virtual_addr = current_struct -> virtual_addr;
        current_len = current_struct->len;
        if (curent_virtual_addr == (uint32_t) addr)
        {
            free_pages(current_thread -> pcb -> PD, (uint32_t)addr, current_len);
            return 1;
        }
        current_node = current_node->next;
    }

    /* failed to search for the allocation info. 
    i.e address not allocated by new_pages */
    return 0;
}

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

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both
 physical address and 4KB aligned (really ?)
 **/
int sys_new_pages(void *addr, int len)
{
    /* If either the address is invalid or len is not aligned,
     * return a negative number */
    if (addr == NULL ||                     // addr is null
            (uint32_t)addr < 0x01000000 ||      // addr is in kernel memory
            (((uint32_t)addr) & 0xfff) != 0 ||   // addr is not aligned
            len < 0 ||                           // len is negative
            (len & 0xfff) != 0) // len is not integral multiple of sys page size
        return -1;
    uint32_t* PD = current_thread -> pcb -> PD;
    uint32_t pd_index = ((uint32_t)addr) >> 22;
    uint32_t pt_index = ((uint32_t)addr & 0x3ff000) >> 12;
    /*check if already mapped*/
    if (((uint32_t*)(PD[pd_index]))[pt_index] != 0) return -1;

    allocate_pages(PD, (uint32_t)addr, len);
    //Lastly, insert the va node into the list
    VA_INFO* current_va_info = malloc(sizeof(VA_INFO));
    current_va_info -> virtual_addr = (uint32_t)addr;
    current_va_info -> len = len;
    list_insert_last(&current_thread->pcb->va, &current_va_info->va_node);
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
    /* step 1: check if removable */
    if (addr == NULL ||                     // addr is null
            (uint32_t)addr < 0x01000000 ||      // addr is in kernel memory
            ((uint32_t)addr & 0xfff) != 0)    // addr is not aligned
        return -1;
    // uint32_t* PD = current_thread -> pcb -> PD;
    // uint32_t pd_index = ((uint32_t)addr) >> 22;
    // uint32_t pt_index = ((uint32_t)addr & 0x3ff000) >> 12;
    // uint32_t phys_addr_raw = ((uint32_t*)(PD[pd_index]))[pt_index];
    

    node* current_node = list_begin(&current_thread->pcb->va);
    int current_len;
    uint32_t curent_virtual_addr;
    /* try to find the address allocation info in the list*/
    while (current_node!=NULL)
    {
        VA_INFO* current_struct = list_entry(current_node,VA_INFO,va_node);
        curent_virtual_addr = current_struct -> virtual_addr;
        current_len = current_struct->virtual_addr;
        if (curent_virtual_addr == (uint32_t) addr)
        {
            free_pages(current_thread -> pcb -> PD, (uint32_t)addr, current_len);
            return 0;
        }
    }

    return -1;

}

/** @file sys_memory_management.c
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
#include <cr.h>

/** @brief new pages system call system land implementation
 *
 *  check if this new pages request is valid first
 *
 *  @param the address to be allocated and its length
 *  @return 1 on success, 0 on failure
 **/
int sys_new_pages(void *addr, int len)
{
    /* step 1: check if valid to new_pages*/
    if (!is_user_addr(addr)) return -1;
    // addr is not page aligned;
    if (((uint32_t)addr & 0xfff) != 0)
        return -1;
    // len is not a positive multiple of the page size
    if (len < 0 || (len & 0xfff) != 0)
        return -1;
    // If os has insufficient resources to satisfy the request
    int requested_page_num = len / 4096;
    if (requested_page_num > free_frame_num) return -1;
    // if any portion already in task's address space;
    int i;
    uint32_t cur_addr, cur_pd_index, cur_pt_index, phys_adddr;
    uint32_t *PD, *PT;
    PD = current_thread -> pcb -> PD;
    for (i = 0; i < requested_page_num; i++)
    {
        cur_addr = (uint32_t)(addr) + PAGE_SIZE * i;
        cur_pd_index = ((uint32_t)cur_addr) >> 22;
        cur_pt_index = ((uint32_t)cur_addr & 0x3ff000) >> 12;
        PT = (uint32_t *) DEFLAG_ADDR(PD[cur_pd_index]);
        /*not mapped yet*/
        if (PT == NULL) continue;
        phys_adddr = DEFLAG_ADDR(PT[cur_pt_index]);
        /*already mapped, reject*/
        if (phys_adddr != 0) return -1;
    }
    /* step 2: allocate*/

    allocate_pages(PD, (uint32_t)addr, len);
    //Lastly, insert the va node into the list
    VA_INFO *current_va_info = malloc(sizeof(VA_INFO));
    current_va_info -> virtual_addr = (uint32_t)addr;
    current_va_info -> len = len;

    list_insert_last(&current_thread->pcb->va, &current_va_info->va_node);
    return 0;
}

/** @brief remove pages system call
 *
 *  remove the address. the length of that address is specified in VA_INFO struct
 *
 *  @param the address to be removed
 *  @return 1 on success, 0 on failure
 **/
int sys_remove_pages(void *addr)
{
    /* step 1: check if removable by inspecting virtual address*/
    if (!is_user_addr(addr)) return -1;
    // addr is not aligned
    if (((uint32_t)addr & 0xfff)) return -1;
    // no mapping at all
    if (!addr_has_mapping(addr)) return -1;

    /* step 2: check if removable by inspecting the mapped address's flags*/
    uint32_t *PD, *PT;
    PD = current_thread -> pcb -> PD;
    uint32_t pd_index = ((uint32_t)addr) >> 22;
    uint32_t pt_index = ((uint32_t)addr & 0x3ff000) >> 12;
    PT = (uint32_t *) DEFLAG_ADDR(PD[pd_index]);
    uint32_t phys_addr_raw = PT[pt_index];

    if ((phys_addr_raw & 0x7) != 0x7) return -1;

    /* step 3: search for the allocation info */
    node *current_node = list_begin(&current_thread->pcb->va);
    int current_len;
    uint32_t current_virtual_addr;
    /* try to find the address allocation info in the list*/
    while (current_node != NULL)
    {
        VA_INFO *current_struct = list_entry(current_node, VA_INFO, va_node);
        current_virtual_addr = current_struct -> virtual_addr;
        current_len = current_struct->len;
        // lprintf("wanna remove addr: %x this number: %d,",
        //         (unsigned int)current_virtual_addr, current_len);
        if (current_virtual_addr == (uint32_t) addr)
        {
            free_pages(current_thread -> pcb -> PD, (uint32_t)addr, current_len);
            list_delete(&current_thread->pcb->va, current_node);
            set_cr3((uint32_t)PD);
            return 0;
        }
        current_node = current_node->next;
    }

    /* failed to search for the allocation info.
    i.e address not allocated by new_pages */
    return -1;
}

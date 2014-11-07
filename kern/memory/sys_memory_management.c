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

extern TCB *current_thread;

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
    allocate_pages(current_thread -> pcb -> PD, (uint32_t)addr, len);
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
    if (addr == NULL ||                     // addr is null
            (uint32_t)addr < 0x01000000 ||      // addr is in kernel memory
            ((uint32_t)addr & 0xfff) != 0)    // addr is not aligned
        return -1;

    // This is not that easy, remeber every calling newpages instance in a
    // possibly linked list????
    // free_pages(current_thread -> pcb -> PD, (uint32_t)addr, len);

    return 0;

}
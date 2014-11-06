/** @file thread.c
 *
 *  @brief This file includes some kernel thread routines.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "control_block.h"
#include "linked_list.h"
#include "seg.h"
#include "cr.h"
#include "simics.h"
#include "malloc.h"
#include <elf/elf_410.h>
#include "common_kern.h"
#include "string.h"
#include "eflags.h"
#include "mutex_type.h"


/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
int thr_init()
{
    list_init(&runnable_queue);
    next_tid = 1;
    return 0;
}

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
TCB *thr_create(simple_elf_t *se_hdr, int run)
{
    // set up tcb for this program

    TCB *tcb = (TCB *)malloc(sizeof(TCB));
    tcb -> tid = next_tid;
    next_tid++;
    mutex_init(&tcb -> tcb_mutex);

    tcb -> state = THREAD_RUNNING;

    tcb -> stack_size = 4096;
    tcb -> stack_base = smemalign(4096, tcb->stack_size);

    tcb -> registers.ds = SEGSEL_USER_DS;
    tcb -> registers.es = SEGSEL_USER_DS;
    tcb -> registers.fs = SEGSEL_USER_DS;
    tcb -> registers.gs = SEGSEL_USER_DS;

    tcb -> registers.edi = 0;
    tcb -> registers.esi = 0;
    tcb -> registers.ebp = 0xffffffff;  // set up frame pointer
    tcb -> registers.ebx = 0;
    tcb -> registers.edx = 0;
    tcb -> registers.ecx = 0;
    tcb -> registers.eax = 0;

    tcb -> esp = (uint32_t)tcb -> stack_size + (uint32_t)tcb -> stack_base -4;

    tcb -> registers.eip = se_hdr -> e_entry;
    lprintf("The dip is %x", (unsigned int)se_hdr->e_entry);
    tcb -> registers.cs = SEGSEL_USER_CS;
    tcb -> registers.eflags = (get_eflags() | EFL_RESV1) & ~EFL_AC;
    tcb -> registers.esp = 0xffffff10;  // set up user stack pointer
    tcb -> registers.ss = SEGSEL_USER_DS;
    lprintf("The kernel stack is : %p", tcb -> stack_base + 4096);
    if (!run)
    {
        // if not run, we put it in the run queue
        list_insert_last(&runnable_queue, &tcb -> thread_list);
        tcb -> state = THREAD_INIT;
    }
    return tcb;


}

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
int thr_exit()
{
    // probably be vanish??
    return 0;
}
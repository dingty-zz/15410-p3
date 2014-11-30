/** @file thread_basic.c
 *
 *  @brief This file includes some kernel thread routines.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "control_block.h"
#include "datastructure/linked_list.h"
#include "seg.h"
#include "cr.h"
#include "simics.h"
#include "malloc.h"
#include <elf/elf_410.h>
#include "common_kern.h"
#include "string.h"
#include "eflags.h"
#include "locks/mutex_type.h"
#include "thread_basic.h"
#include <page.h>

/** @brief initialize threads
 *
 *  initialize the global runnable queue and block queue for threads;
 *
 *  @param nothing
 *  @return nothing
 **/
void thr_init()
{
    list_init(&runnable_queue);
    list_init(&blocked_queue);
    mutex_init(&blocked_queue_lock);
    mutex_init(&runnable_queue_lock);
    mutex_init(&deschedule_lock);
    current_thread = NULL;
    next_tid = 1;
}

/** @brief create a thread
 *
 *  allocate a new tcb and then initialize all the fields, and add the tcb into
 *  the list managing all the thread
 *
 *  @param eip and run or not flag
 *  @return the tcb created
 **/
TCB *thr_create(unsigned int eip, int run)
{
    // set up tcb for this program
    TCB *tcb = (TCB *)malloc(sizeof(TCB));
    if (tcb == NULL)
    {
        return NULL;
    }
    tcb -> tid = next_tid;
    next_tid++;
    mutex_init(&tcb -> tcb_mutex);
    tcb -> state = THREAD_RUNNING;
    // Allocate kernel stack for this thread
    tcb -> stack_size = PAGE_SIZE;
    tcb -> stack_base = smemalign(PAGE_SIZE, tcb->stack_size);
    tcb -> esp = (uint32_t)tcb -> stack_size + (uint32_t)tcb -> stack_base - sizeof(int);
    
    tcb -> start_ticks = 0;
    tcb -> duration = 0;

    tcb -> registers.ds = SEGSEL_USER_DS;
    tcb -> registers.es = SEGSEL_USER_DS;
    tcb -> registers.fs = SEGSEL_USER_DS;
    tcb -> registers.gs = SEGSEL_USER_DS;

    tcb -> registers.edi = 0;
    tcb -> registers.esi = 0;
    // set up frame pointer
    tcb -> registers.ebp = 0xffffffff;
    tcb -> registers.ebx = 0;
    tcb -> registers.edx = 0;
    tcb -> registers.ecx = 0;
    tcb -> registers.eax = 0;
    
    tcb -> registers.eip = eip;

    tcb -> registers.cs = SEGSEL_USER_CS;
    tcb -> registers.eflags = ((get_eflags() | EFL_RESV1) & ~EFL_AC )| EFL_IF;
    // set up user stack pointer
    tcb -> registers.esp = 0xffffff10;  // Predefined stack pointer
    tcb -> registers.ss = SEGSEL_USER_DS;
    if (!run)
    {
        list_insert_last(&runnable_queue, &tcb -> thread_list_node);
        tcb -> state = THREAD_INIT;
    }
    return tcb;
}
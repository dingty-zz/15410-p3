/** @file scheduler.c
 *
 *  @brief This file the scheduler and context switch routine.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "datastructure/linked_list.h"
#include "control_block.h"
#include "simics.h"
#include "seg.h"
#include "cr.h"
#include "simics.h"
#include "malloc.h"
#include <elf/elf_410.h>
#include "common_kern.h"
#include "string.h"
#include "eflags.h"
#include <x86/asm.h>
#include "do_switch.h"
#include "enter_user_mode.h"
#include "hardware/timer.h"
#include "scheduler.h"


unsigned int seconds;
extern list runnable_queue;
extern TCB *current_thread;  // indicates the current runnign thread

void tick(unsigned int numTicks)
{

    if (numTicks % 5 == 0)
    {
        ++seconds;
        if (seconds % 5 == 0)
        {
            lprintf("5 seconds, let's context switch\n");
            schedule(-1);     // schedule
            lprintf("\nNow we are running in a different thread");
        }

    }

}
// have the priority to schedule the blocked thread?  i think so
void schedule(int tid)
{
    disable_interrupts();
    // Before actual context switching, we make sleeping thread to be
    // runnable if it's the time to wake up
    node *n;
    for (n = list_begin(&blocked_queue); n != NULL; n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, thread_list_node);
        if (tcb -> state == THREAD_SLEEPING)
        {
            if (tcb -> start_ticks + tcb -> duration < sys_get_ticks())
            {
                tcb -> state = THREAD_RUNNABLE;
                list_delete(&blocked_queue, n);
                list_insert_last(&runnable_queue, n);
            }
        }
    }

    // Unless the current thread is non-schedulable, and there is no
    // runnable thread, calling schedule must
    // result in a context switch, finding a schedulable thread as much
    // as possible

    lprintf("return or not");
    // TODO, schedule halt for spinning
     if(current_thread -> tid == 1 && runnable_queue.length==0)
    {
       lprintf("reach here");
       MAGIC_BREAK;
       return;
    }

    TCB *next_thread = NULL;
    if (tid == -1)
    {
        // pop a thread from the runnable_queue
        node *n  = list_delete_first(&runnable_queue);
        next_thread = list_entry(n, TCB, thread_list_node);
    }
    else      // Search for a specific thread
    {
        next_thread = list_search_tid(&runnable_queue, tid);
    }

    lprintf("The next thread addr is %p", next_thread);
    lprintf("Before switching, the current getcr3 is %x", (unsigned int)get_cr3());

    switch (current_thread -> state)
    {
        case THREAD_EXIT:
            break;      // we don't put the current thread back to queue

        case THREAD_BLOCKED:
        case THREAD_WAITING:
            list_insert_last(&blocked_queue, &current_thread->thread_list_node);
            break;

        default:
            list_insert_last(&runnable_queue, &current_thread->thread_list_node);
    }

    // MAGIC_BREAK;
    // lprintf("Switch from current: %p, to next: %p\n", current_thread, next_thread);
    current_thread = context_switch(current_thread, next_thread);

    lprintf(" current running: %p\n", current_thread);
    enable_interrupts();
}


TCB *context_switch(TCB *current, TCB *next)
{
    lprintf("Switch from current: %p, to next: %p\n", current, next);

    // MAGIC_BREAK;
    set_cr3((uint32_t)next -> pcb -> PD);
    // MAGIC_BREAK;


    set_esp0((uint32_t)(next -> stack_base + next -> stack_size));
    do_switch(current, next, next -> state);
    // TCB *temp = next;
    // next = current;
    // current = temp;
    lprintf("(^_^)Switch from current: %p, to next: %p\n", current, next);



    return current;
}

void prepare_init_thread(TCB *next)
{
    lprintf("%p", next);
    lprintf("105, run this thread");
    // set_cr3((uint32_t)next -> pcb -> PD);
    // set_esp0((uint32_t)(next -> stack_base + next -> stack_size));
    next -> state = THREAD_RUNNING;
    current_thread = next;
    enable_interrupts();
    enter_user_mode(next -> registers.edi,
                    next -> registers.esi,
                    next -> registers.ebp,
                    next -> registers.ebx,
                    next -> registers.edx,
                    next -> registers.ecx,
                    next -> registers.eax,
                    next -> registers.eip,
                    next -> registers.cs,
                    next -> registers.eflags,
                    next -> registers.esp,
                    next -> registers.ss);
}

/** @brief The generic function to search for a specific tid in a list
 *
 *  @param l the pointer to the node with that specific tid
 *  @return the node that matches the tid
 */
TCB *list_search_tid(list *l, int tid)
{
    if (l == NULL) return NULL;
    node *temp = l -> head;
    while (temp)
    {
        TCB *thread = list_entry(temp, TCB, thread_list_node);
        if (thread -> tid == tid)
            return thread;
        temp = temp -> next;
    }
    return NULL;
}
/** @file scheduler.c
 *
 *  @brief This file the scheduler and context switch routines.
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
#include "exception/sys_aswexn.h"
// This is the idle pid, we assign it to be 1
#define IDLE_PID 1

// We invoke context switch every 100 ticks
#define SCHEDULE_INTERVAL 100
extern uint32_t get_esp();
void tick(unsigned int numTicks)
{
    // For thread execution time
    if (current_thread -> virtual_period == MODE_ON)
    {
        current_thread -> virtual_tick = \
                                         ++current_thread -> virtual_tick % current_thread -> virtual_period;
        if (current_thread -> virtual_tick == 0 && \
                current_thread -> signals[SIGVTALRM - MIN_SIG] != SIGNAL_ENQUEUED && \
                ((current_thread -> mask >> SIGVTALRM) & 0x1) == 1)
        {
            /* send it a SIGVTALRM signal */
            signal_t *vtalrm_sig = make_signal_node(0, SIGVTALRM);
            list_insert_last(&current_thread -> pending_signals, &vtalrm_sig -> signal_list_node);
        }
    }

    // For threads wall time
    node *n;
    for (n = list_begin(&alarm_list); n != NULL; n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, alarm_list_node);
        tcb -> real_tick = \
                           ++tcb -> real_tick % tcb -> real_period;
        if (tcb -> real_tick == 0 && \
                tcb -> signals[SIGALRM - MIN_SIG] != SIGNAL_ENQUEUED && \
                ((tcb -> mask >> SIGALRM) & 0x1) == 1)
        {
            /* send it a SIGALRM signal */
            signal_t *alrm_sig = make_signal_node(0, SIGALRM);
            list_insert_last(&tcb -> pending_signals, &alrm_sig -> signal_list_node);
        }
    }

    if (numTicks % SCHEDULE_INTERVAL == 0)
    {
        // let's context switch
        schedule(-1);
        // Now we are running in a different thread
    }

}

/** @brief Do the scheduling by switch from the current thread's kernel
 *         stack to the next thread's kernel stack. Determine which queue
 *         should the current thread goes by checking its status
 *
 *  @param tid schedule to a specific thread, -1 schedule in FIFO manner
 **/
void schedule(int tid)
{
    disable_interrupts();
    // Before actual context switching, we make sleeping thread to be
    // runnable if it's the time to wake it up
    node *n;
    for (n = list_begin(&blocked_queue); n != NULL; n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, thread_list_node);
        if (tcb -> state == THREAD_SLEEPING)
        {
            // find a sleeping thread that is able to be woken up
            if (tcb -> start_ticks + tcb -> duration < sys_get_ticks())
            {
                tcb -> state = THREAD_RUNNABLE;
                list_delete(&blocked_queue, &tcb->thread_list_node);
                list_insert_last(&runnable_queue, &tcb->thread_list_node);
            }
        }
    }

    TCB *target = NULL;

    // If there is no runnable thread in the runnable queue and the current
    // thread is idle, we won't do context switch. Instead, we will let idle
    // to run again

    if (current_thread -> tid == IDLE_PID && runnable_queue.length == 0)
    {
        lprintf("only idle is running");
        return;
    }

    TCB *next_thread = NULL;
    if (tid == -1)
    {
        // pop a thread from the runnable_queue
        node *n  = list_delete_first(&runnable_queue);
        next_thread = list_entry(n, TCB, thread_list_node);
    }
    else
    {
        // Search for a specific target thread
        next_thread = list_search_tid(&runnable_queue, tid);
        list_delete(&runnable_queue, &next_thread->thread_list_node);
    }
    lprintf("The next tthread will run is %d", next_thread->tid);
    /* Checks the current state of the current thread, and decide which queue
       should the process goes to */
    switch (current_thread -> state)
    {
    // If the current thread is exited, we don't put the it back to any queue
    case THREAD_EXIT:
        break;

    /* If the current thread is blocked by calling deschedule, we put it into the block
       queue after deschedule_lock is unlocked */
    case THREAD_BLOCKED:
        list_insert_last(&blocked_queue, &current_thread->thread_list_node);
        mutex_unlock(&deschedule_lock);
        break;
    // If the current thread is blocked, we put it into the block queue
    case THREAD_WAITING:
    case THREAD_READLINE:
    case THREAD_SLEEPING:
    case THREAD_SIGNAL_BLOCKED:
    lprintf("Eneuq blocked wuquu");
        list_insert_last(&blocked_queue, &current_thread->thread_list_node);
        break;
    // The thread is runnable by default, we put it into the runnable queue
    default:
        list_insert_last(&runnable_queue, &current_thread->thread_list_node);
        break;
    }
    target = NULL;
    // Do the context switch between two threads
    current_thread = context_switch(current_thread, next_thread);
    lprintf("Now switch to tid %d with the esp is %x", current_thread->tid, (unsigned int)current_thread -> esp);
    // MAGIC_BREAK;
    lprintf("The pending %p", &current_thread -> pending_signals);
    // Check if the current thread has pending signals
    if (current_thread -> pending_signals.length > 0)
    {
        // Invoke the signal handler by calling the wrapper first
        lprintf("scheduler wants to call the signal handler for tid: %d", current_thread->tid);
        node *n = NULL;
        for (n = list_begin(&current_thread -> pending_signals);
                n != NULL;
                n = n -> next)
        {
            signal_t *sig = list_entry(n, signal_t, signal_list_node);
            lprintf("The mask is %x",(unsigned int) current_thread -> mask);
            if (((current_thread -> mask >> sig -> cause) & 0x1) == 1)
            {
                // MAGIC_BREAK;
                current_thread -> saved_esp = get_esp();
                lprintf("Let's call signal_handler_wrapper current esp is %x", (unsigned int)current_thread -> saved_esp);
                set_esp0((uint32_t)(current_thread -> saved_esp+8));
                signal_handler_wrapper(n);
                current_thread -> saved_esp = 0;
                set_esp0((uint32_t)(current_thread -> stack_base + current_thread -> stack_size));

                break;
            }
        }
        // Assume by calling swexn in handler, we can return here, the
        // thread can run as normal
        lprintf("Signal handler finished in the scheduler");
        // MAGIC_BREAK;

    }

    enable_interrupts();
}

/** @brief Do the real context switch by changing the kernel stack pointer
 *         from the current thread's kernel stack to the next thread's kernel stack
 *
 *  @param current points to the current running thread tcb
 *  @param next points to the next threads tcb
 *  @return TCB * the pointer to the current thread after context switch
 **/
TCB *context_switch(TCB *current, TCB *next)
{
    set_cr3((uint32_t)next -> pcb -> PD);
    // Set esp0 for the next thread
    set_esp0((uint32_t)(next -> stack_base + next -> stack_size));
    lprintf("Before do switch");
    // MAGIC_BREAK;
    do_switch(current, next, next -> state);
        lprintf("after do switch the esp0 is %x",(unsigned int)get_esp0());
    // MAGIC_BREAK;
    return current;
}

/** @brief Run the thread that hasn't been run before, whose state is
 *         THREAD_INIT
 *         We push its initial registers on to its kernel stack and using
 *         iret to enter user space and let it run.
 *
 *  @param next points to the next thread tcb whose state is THREAD_INIT
 **/
void prepare_init_thread(TCB *next)
{
    next -> state = THREAD_RUNNING;
    current_thread = next;
    enable_interrupts();
    lprintf("we init %d", next -> tid);
    // Enter user space
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

/** @brief Search a specific thread from a queue by tid
 *
 *  @param l the pointer to the queue
 *  @param tid the tid to be searched for
 *  @return TCB * that points to the matching thread
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
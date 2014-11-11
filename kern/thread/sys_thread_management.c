
/** @file control_block.h
 *
 *  @brief This file includes paging handling routines
*          1. General design, PD, PT descrptions
           2. How free list works
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <stddef.h>
#include <syscall.h>
#include "control_block.h"
#include "locks/mutex_type.h"
#include "process/scheduler.h"
#include "hardware/timer.h"
#include "simics.h"
// extern TCB *current_thread;
// extern list runnable_queue;
// extern list blocked_queue;

int sys_yield(int tid)
{

    // If tid is invalid
    if (tid < -1) return -1;

    if (tid == -1)
    {
        schedule(-1);
        return 0;   // can return back here??
    }
    else
    {
    lprintf("(^_^)_inside yeild");

        // If the target thread is blocked
        mutex_lock(&blocked_queue_lock);
        node *n;
        for (n = list_begin(&blocked_queue); 
             n != NULL; 
             n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, thread_list_node);
            if (tcb -> tid == tid)
            {
                return -1;
            }
        }
        mutex_unlock(&blocked_queue_lock);

        // If try to yield to itself
        mutex_lock(&current_thread -> tcb_mutex);
        if (current_thread -> tid == tid)
        {
            mutex_unlock(&current_thread -> tcb_mutex);
            return -1;
        }
        mutex_unlock(&current_thread -> tcb_mutex);


        // If the target thread does not exit
        int exist = 0;
        mutex_lock(&runnable_queue_lock);
        for (n = list_begin (&runnable_queue); 
             n != NULL; 
             n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, thread_list_node);
            if (tcb -> tid == tid)
            {
                exist = 1;
                break;
            }
        }
        mutex_unlock(&runnable_queue_lock);
        if (!exist)
        {
            return -1;
        }
        lprintf("(^_^)_schedule in yield");
        // Finally, tid is valid, we schedule to this thread
        schedule(tid);
    }

    return 0; // can return back here?
}


int sys_deschedule(int *reject)
{
    // Check pointer
    if (*reject != 0)
    {
        return 0;
    }
    // atomicity??
    mutex_lock(&current_thread -> tcb_mutex);
    // Set status to blocked
    current_thread -> state = THREAD_BLOCKED;
    mutex_unlock(&current_thread -> tcb_mutex);

    // Call the scheduler
        lprintf("(^_^)_deschedule call schedule");

    schedule(-1);

    return 0;
}

int sys_make_runnable(int tid)
{
    lprintf("(^_^)_before make runnable, tid: %d", tid);
    if (tid <= 0)
    {
        return -1;
    }

    node *n;
    // Find if the target is runnable, if so, return -1
    mutex_lock(&runnable_queue_lock);
    for (n = list_begin (&runnable_queue); n != NULL; n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, thread_list_node);
        if (tcb -> tid == tid)
        {
            if (tcb -> state == THREAD_RUNNABLE)
            {
                mutex_unlock(&blocked_queue_lock);
                lprintf("sys_make_runnable:[ohoh, it's runnable]");
                return -1;
            }
        }
    }
    mutex_unlock(&blocked_queue_lock);

    // If the target thread is blocked and exist
    int exist = 0;
    TCB *target = NULL;
    mutex_lock(&blocked_queue_lock);
    for (n = list_begin(&blocked_queue); n != NULL; n = n -> next)
    {
        target = list_entry(n, TCB, thread_list_node);
        if (target -> tid == tid && target -> state == THREAD_BLOCKED)
        {

            exist = 1;
        }
    }
    mutex_unlock(&blocked_queue_lock);

    if (!exist)
    {
       lprintf("sys_make_runnable[ohoh, it doens't exist]");

        return -1;
    }

    // Make the target thread runnable
    mutex_lock(&target -> tcb_mutex);
    target -> state = THREAD_RUNNABLE;
    mutex_unlock(&target -> tcb_mutex);
        lprintf("(^_^)_now make runnable");

    return 0;
}

int sys_gettid()
{
    // return the tid from the currenspoding entry in tid

    mutex_lock(&current_thread -> tcb_mutex);
    int tid =  current_thread -> tid;
    mutex_unlock(&current_thread -> tcb_mutex);
    lprintf("called gettid: [The tid is %d]", tid);
    return tid;

}

int sys_sleep(int ticks)
{
    if (ticks < 0)
    {
        return -1;
    }
    else if (ticks == 0)
    {
        return 0;
    }
    else
    {
        mutex_lock(&current_thread -> tcb_mutex);
        current_thread -> duration = ticks;
        current_thread -> start_ticks = sys_get_ticks();
        current_thread -> state = THREAD_SLEEPING;
        mutex_unlock(&current_thread -> tcb_mutex);

        // Put this thread to sleep
        schedule(-1);
    }
    return 0;
}

// int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
// {
//     return 0; /* FALSE, but placates assert() in crt0.c */
// }

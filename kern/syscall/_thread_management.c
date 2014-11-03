
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
#include <syscall.h>
#include "control_block.h"
#include "mutex_type.h"

extern TCB *current_thread;
extern list runnable_queue;

mutex_t sched_lock;
int _yield(int tid)
{

    if (tid == -1)
    {
        schedule();
        return 0;   // can return back here??
    }
    else
    {
        node *n;
        for (n = list_begin (&blocked_thread_queue); n != list_end(&blocked_thread_queue); n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, all_threads);
            if (tcb -> tid == tid)
            {
                return -1;
            }
        }

        for (n = list_begin (&runnable_queue); n != list_end(&runnable_queue); n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, all_threads);
            if (tcb -> tid == tid)
            {
                list_insert_last(current_thread, tid)
            }
        }
    }

    return schedule(tid);
}

int _deschedule(int *reject)
{
    // check pointer
    if (*reject != 0)
    {
        return 0;
    }
    current_thread -> state = THREAD_DESCHEDULED;
    schedule(-1);
    return 0;
}

int _make_runnable(int tid)
{
    if (tid <= 0)
    {
        return -1;
    }
    // find the descheduled thread and see if tid = tid
    for (n = list_begin (&runnable_queue); n != list_end(&runnable_queue); n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, all_threads);
        if (tcb -> tid == tid)
        {
            if (tcb -> state == THREAD_RUNNABLE ||
            	tcb -> state == THREAD_BLOCKED)
            {
                return -1;
            }
        }
    }
    thread -> state = THREAD_RUNNABLE;
    return 0;
}

int _gettid()
{
    // return the tid from the currenspoding entry in tid

    return current_thread -> tid;


}

int _sleep(int ticks)
{
	if (ticks < 0)
	{
	    return -1;	
	} else if (ticks == 0)
	{
		return 0;
	}else{
		current_thread -> ticks = ticks;
		current_thread -> state = THREAD_SLEEPING;
		schedule(-1);
	}
	return 0;
}

// int _swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
// {
//     return 0; /* FALSE, but placates assert() in crt0.c */
// }

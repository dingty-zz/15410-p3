
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
extern TCB *current_thread;

int _yield(int tid)
{	
	// COMPLEX enough
	if (tid == -1)
	{
		schedule();
	} else {
        TCB *dest_thread = list_find();
        list_insert_last(current_thread, tid);
        // run dest_thread
    }
    return -1;
}

int _deschedule(int *flag)
{
    current_thread -> state = THREAD_DESCHEDULED;
    next_thread = list_delete_first(threads);
    run next_thread;
    return 0;
}

int _make_runnable(int pid)
{
    list_find(thrads) // find the descheduled thread and see if tid = tid
    thread -> state = THREAD_RUNNABLE;
    return -1;
}

int _gettid()
{
    // return the tid from the currenspoding entry in tid
    return current_thread -> tid;

}

int _sleep(int ticks)
{
    return -1;
}

int _swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    return 0; /* FALSE, but placates assert() in crt0.c */
}

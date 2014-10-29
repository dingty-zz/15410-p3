
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
		/* code */
	}
    return -1;
}

int _deschedule(int *flag)
{
    return -1;
}

int _make_runnable(int pid)
{
    return -1;
}

int _gettid()
{
    // return the tid from the currenspoding entry in tid
    return current_thread -> tid_;

}

int _sleep(int ticks)
{
    return -1;
}

int _swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    return 0; /* FALSE, but placates assert() in crt0.c */
}

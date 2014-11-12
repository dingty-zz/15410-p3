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
#include <string.h>
#include "control_block.h"
#include "locks/mutex_type.h"
#include "process/scheduler.h"
#include "hardware/timer.h"
#include "handler_install.h"
#include "seg.h"
#include "simics.h"
#include <ureg.h>
#include <eflags.h>
#include <cr.h>
// extern TCB *current_thread;
// extern list runnable_queue;
// extern list blocked_queue;
 
#define ALLOWED_BITS    (EFL_CF | EFL_PF | EFL_AF | EFL_ZF | EFL_SF | \
                        EFL_DF | EFL_OF | EFL_AC)


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
    schedule(-1);

    return 0;
}

int sys_make_runnable(int tid)
{
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
        return -1;
    }

    // Make the target thread runnable
    mutex_lock(&target -> tcb_mutex);
    target -> state = THREAD_RUNNABLE;
    mutex_unlock(&target -> tcb_mutex);

    return 0;
}

int sys_gettid()
{
    // return the tid from the currenspoding entry in tid

    mutex_lock(&current_thread -> tcb_mutex);
    int tid =  current_thread -> tid;
    mutex_unlock(&current_thread -> tcb_mutex);
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
        current_thread -> state= THREAD_SLEEPING;
        mutex_unlock(&current_thread -> tcb_mutex);

        // Put this thread to sleep
        schedule(-1);
    }
    return 0;
}

//return 0 if invalid and 1 if valid;
static int is_valid_newureg(ureg_t *newureg)
{
    unsigned int kern_stack_high, changedbit, mask;
    uint32_t current_eflags;
    if (newureg == NULL) return 1;
    else
    {
        /*still needs to add stuff*/
        kern_stack_high = get_esp0();
        current_eflags = get_eflags();
        if (newureg -> cause != SWEXN_CAUSE_DIVIDE      &&
            newureg -> cause != SWEXN_CAUSE_DEBUG       &&
            newureg -> cause != SWEXN_CAUSE_BREAKPOINT  &&
            newureg -> cause != SWEXN_CAUSE_OVERFLOW    &&
            newureg -> cause != SWEXN_CAUSE_BOUNDCHECK  &&
            newureg -> cause != SWEXN_CAUSE_OPCODE      &&
            newureg -> cause != SWEXN_CAUSE_NOFPU       &&
            newureg -> cause != SWEXN_CAUSE_SEGFAULT    &&
            newureg -> cause != SWEXN_CAUSE_PROTFAULT   &&
            newureg -> cause != SWEXN_CAUSE_PAGEFAULT   &&
            newureg -> cause != SWEXN_CAUSE_FPUFAULT    &&
            newureg -> cause != SWEXN_CAUSE_ALIGNFAULT  &&
            newureg -> cause != SWEXN_CAUSE_SIMDFAULT) 
            return 0;
        if (newureg -> cs != SEGSEL_USER_CS) return 0;
        if (newureg -> ds != SEGSEL_USER_DS) return 0;
        if (newureg -> ebp <= kern_stack_high) return 0;
        if (newureg -> esp <= kern_stack_high) return 0;
        if (newureg -> eip <= kern_stack_high) return 0;
        //check changed bits of eflags are only those allowed 
        changedbit = newureg -> eflags ^ current_eflags;
        mask = changedbit ^ ALLOWED_BITS;
        //There is(are) unallowed bit(s) that's been changed;
        if ( (changedbit | mask) != ALLOWED_BITS ) return 0;
    }
    return 1;
}

int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    lprintf("starting swexn...");
    MAGIC_BREAK;
    //newureg not valid;
    if (!is_valid_newureg(newureg)) return -1;
    //deregister a handler if exists;
    if (esp3 == NULL || eip == NULL) 
    {
        bzero(&current_thread->swexn_info, sizeof(swexninfo));
        return 0;
    }
    //else register;
    lprintf("installing...esp:%x, arg:%x,newureg:%x,eip:%x",
                (unsigned int)esp3, (unsigned int)arg,(unsigned int)newureg,(unsigned int)eip);
    current_thread -> swexn_info.esp3 = esp3;
    current_thread -> swexn_info.eip = eip;
    current_thread -> swexn_info.arg = arg;
    current_thread -> swexn_info.newureg = newureg;
    current_thread -> swexn_info.installed_flag = 1;
    if (newureg!=NULL)
        lprintf("I have this newureg! its ss is:%x",newureg->ss);
    else
        lprintf("there is no newureg");
    MAGIC_BREAK;
    return 0;
}
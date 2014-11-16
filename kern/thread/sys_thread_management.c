/** @file control_block.h
 *
 *  @brief This file includes implementation of some of the
 *         thread management system calls:
           gettid, deschedule, make_runnable, get_ticks and sleep
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
#include "memory/vm_routines.h"


#define ALLOWED_BITS    (EFL_CF | EFL_PF | EFL_AF | EFL_ZF | EFL_SF | \
                         EFL_DF | EFL_OF | EFL_AC)

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int sys_yield(int tid)
{

    // If tid is invalid
    if (tid < -1) return -1;

    // If tid is -1, directly schedule it
    if (tid == -1)
    {
        schedule(-1);
        return 0;
    }
    else
    {
        lprintf("(x_x)_inside yeild");

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
                lprintf("(x_x)_yield to a blocked thread");
                mutex_unlock(&blocked_queue_lock);

                return -1;
            }
        }
        mutex_unlock(&blocked_queue_lock);

        // If try to yield to itself
        mutex_lock(&current_thread -> tcb_mutex);
        if (current_thread -> tid == tid)
        {
            lprintf("(x_x)_yield to itself");

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
            lprintf("(x_x)_doesn't exist");

            return -1;
        }
        lprintf("(x_x)_schedule in yield");
        // Finally, tid is valid, we schedule to this thread
        schedule(tid);
    }

    return 0; // can return back here?
}

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int sys_deschedule(int *reject)
{
    if (!is_user_addr(reject) || !addr_has_mapping(reject)) return -1;
    mutex_lock(&deschedule_lock);
    // Check reject pointer
    if (*reject != 0)
    {
        mutex_unlock(&deschedule_lock);
        return 0;
    }
    // atomicity??
    mutex_lock(&current_thread -> tcb_mutex);
    // Set status to blocked
    current_thread -> state = THREAD_BLOCKED;
    mutex_unlock(&current_thread -> tcb_mutex);

    // Call the scheduler
    lprintf("(x_x)_deschedule call schedule");

    schedule(-1);

    return 0;
}


/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int sys_make_runnable(int tid)
{
    lprintf("(x_x)_before make runnable, tid: %d", tid);
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
                mutex_unlock(&runnable_queue_lock);
                lprintf("sys_make_runnable:[ohoh, it's runnable]");
                return -1;
            }
        }
    }
    mutex_unlock(&runnable_queue_lock);
    lprintf("143");
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
            break;
        }
    }
    mutex_unlock(&blocked_queue_lock);
    lprintf("159");

    if (!exist)
    {
        lprintf("sys_make_runnable[ohoh, it doens't exist]");

        return -1;
    }
    lprintf("167");

    mutex_lock(&deschedule_lock);

    // Make the target thread runnable
    mutex_lock(&target -> tcb_mutex);
    target -> state = THREAD_RUNNABLE;
    mutex_unlock(&target -> tcb_mutex);

    // Remove the target out of the blocked queue
    mutex_lock(&blocked_queue_lock);
    list_delete(&blocked_queue, &target->thread_list_node);
    mutex_unlock(&blocked_queue_lock);


    // Insert the target into the runnable queue
    mutex_lock(&runnable_queue_lock);
    list_insert_last(&runnable_queue, &target->thread_list_node);
    mutex_unlock(&runnable_queue_lock);

    mutex_unlock(&deschedule_lock);


    lprintf("(x_x)_now make runnable");
    return 0;
}


/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int sys_gettid()
{
    // return the tid from the currenspoding entry in tid
    int tid =  current_thread -> tid;
    lprintf("called gettid: [The tid is %d]", tid);
    return tid;

}


/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
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
        lprintf("let's sleep by schedule");
        schedule(-1);
    }
    return 0;
}

//return 0 if invalid and 1 if valid;

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
static int is_valid_newureg(ureg_t *newureg)
{
    unsigned int kern_stack_high, changedbit, mask;
    uint32_t original_eflags;
    if (newureg == NULL) return 1;
    else
    {
        /*still needs to add stuff*/
        kern_stack_high = get_esp0();
        original_eflags = current_thread -> swexn_info.eflags;
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
        {
            lprintf("newureg failed becuase of cause");
            return 0;
        }
        if (newureg -> cs != SEGSEL_USER_CS)
        {
            lprintf("newureg failed becuase of cs");
            return 0;
        }
        if (newureg -> ds != SEGSEL_USER_DS)
        {
            lprintf("newureg failed becuase of ds");
            return 0;
        }
        if (newureg -> ebp <= kern_stack_high)
        {
            lprintf("newureg failed becuase of ebp");
            return 0;
        }
        if (newureg -> esp <= kern_stack_high)
        {
            lprintf("newureg failed becuase of esp");
            return 0;
        }
        if (newureg -> eip <= kern_stack_high)
        {
            lprintf("newureg failed becuase of ebp");
            return 0;
        }
        //check changed bits of eflags are only those allowed
        changedbit = newureg -> eflags ^ original_eflags;
        mask = changedbit ^ ALLOWED_BITS;
        //There is(are) unallowed bit(s) that's been changed;
        if ( (changedbit | mask) != ALLOWED_BITS )
        {
            lprintf("eflags from the struct:%x", (unsigned int)newureg->eflags);
            lprintf("eflags from the current eflags: %x", (unsigned int)original_eflags);
            lprintf("newureg failed becuase of eflags bits");
            return 0;
        }
    }
    return 1;
}


/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    lprintf("starting swexn...");
    // MAGIC_BREAK;
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
            (unsigned int)esp3, (unsigned int)arg, (unsigned int)newureg, (unsigned int)eip);
    current_thread -> swexn_info.esp3 = esp3;
    current_thread -> swexn_info.eip = eip;
    current_thread -> swexn_info.arg = arg;
    current_thread -> swexn_info.newureg = newureg;
    current_thread -> swexn_info.installed_flag = 1;
    if (newureg != NULL)
        lprintf("I have this newureg! its ss is:%x", newureg->ss);
    else
        lprintf("there is no newureg");
    // MAGIC_BREAK;
    return 0;
}

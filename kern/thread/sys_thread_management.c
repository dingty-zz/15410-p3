/** @file sys_thread_management.c
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


/* allowed user modifiable bits for eflags*/
#define ALLOWED_BITS    (EFL_CF | EFL_PF | EFL_AF | EFL_ZF | EFL_SF | \
                         EFL_DF | EFL_OF | EFL_AC)



/** @brief yield to a target thread via calling schedule
 *
 *  @param tid the target thread id or -1 yield to an arbitrary thread
 *  @return int 0 on success, -1 when can't yield to the target thread
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
                mutex_unlock(&blocked_queue_lock);

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

    return 0;
}

/** @brief Atomically deschedule the current thread after checking reject
 *         pointer
 *
 *  @param reject pointer to decide whether to deschedule or not
 *  @return int 0 on success, -1 otherwise
 **/
int sys_deschedule(int *reject)
{
    if (!is_user_addr(reject) || !addr_has_mapping(reject)) return -1;
    // Achieve atomicity so that other thread can't make runnable to this thread
    mutex_lock(&deschedule_lock);
    // Check reject pointer
    if (*reject != 0)
    {
        mutex_unlock(&deschedule_lock);
        return 0;
    }

    mutex_lock(&current_thread -> tcb_mutex);
    // Set status to be blocked
    current_thread -> state = THREAD_BLOCKED;
    mutex_unlock(&current_thread -> tcb_mutex);

    // Call the scheduler
    schedule(-1);

    return 0;
}


/** @brief Make the target thread be runnable if the target thread is able
 *         to be made runnable
 *
 *  @param tid the target thread id
 *  @return int 0 on success and -1 otherwise
 **/
int sys_make_runnable(int tid)
{
    if (tid <= 0)
    {
        return -1;
    }

    node *n;
    // Find if the target is already runnable, if so, return -1
    mutex_lock(&runnable_queue_lock);
    for (n = list_begin (&runnable_queue); n != NULL; n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, thread_list_node);
        if (tcb -> tid == tid)
        {
            if (tcb -> state == THREAD_RUNNABLE)
            {
                mutex_unlock(&runnable_queue_lock);
                return -1;
            }
        }
    }
    mutex_unlock(&runnable_queue_lock);
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

    // If such thread doesn't exist, return -1
    if (!exist)
    {
        return -1;
    }

    // Achieves the atomicity
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


    return 0;
}


/** @brief Return the current thread id
 *
 *  @return int the current thread id
 **/
int sys_gettid()
{
    // return the tid from the current thread
    int tid =  current_thread -> tid;
    return tid;

}


/** @brief Deschedules the calling thread until at least ticks timer
 *         interrupts have occurred after the call
 *  @param ticks The number of ticks that the thread needs to sleep
 *  @return int 0 on success and -1 otherwise
 **/
int sys_sleep(int ticks)
{
    if (ticks < 0)
    {
        return -1;
    }
    else if (ticks == 0)
    {
        return 0;  // returns immediately when ticks is 0
    }
    else
    {
        mutex_lock(&current_thread -> tcb_mutex);
        current_thread -> duration = ticks;
        current_thread -> start_ticks = sys_get_ticks();
        current_thread -> state = THREAD_SLEEPING;
        mutex_unlock(&current_thread -> tcb_mutex);

        // Put this thread to sleep by calling schedule
        schedule(-1);
    }
    return 0;
}

/** @brief Determine if the newureg is valid or not
 *
 *  helper function for sys_swexn
 *
 *  @param newureg
 *  @return int 0 if invalid and 1 if valid;
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
                newureg -> cause != SWEXN_CAUSE_SIMDFAULT   &&
                newureg -> cause != SIGKILL      &&
                newureg -> cause != SIGALRM       &&
                newureg -> cause != SIGVTALRM  &&
                newureg -> cause != SIGDANGER    &&
                newureg -> cause != SIGUSR1  &&
                newureg -> cause != SIGUSR2      &&
                newureg -> cause != SIGUSR3       )
        {
            // lprintf("296");

            return 0;
        }
        if (newureg -> cs != SEGSEL_USER_CS)
        {
            // lprintf("302");

            return 0;
        }
        if (newureg -> ds != SEGSEL_USER_DS)
        {
            // lprintf("308");

            return 0;
        }
        if (newureg -> ebp <= kern_stack_high)
        {
            // lprintf("314");

            return 0;
        }
        if (newureg -> esp <= kern_stack_high)
        {
            // lprintf("320");

            return 0;
        }
        if (newureg -> eip <= kern_stack_high)
        {
            // lprintf("326");

            return 0;
        }
        //check changed bits of eflags are only those allowed
        changedbit = newureg -> eflags ^ original_eflags;
        mask = changedbit ^ ALLOWED_BITS;
        //There is(are) unallowed bit(s) that's been changed;
        if ( (changedbit | mask) != ALLOWED_BITS )
        {
            // lprintf("336");

            return 0;
        }
    }
    return 1;
}


/** @brief system land implementation of swexn
 *
 *  check if any of eip and esp3 is null first; basic idea is to store
 *  the handler information in the swexn_info sturcture in tcb
 *
 *  @param esp3, eip, argumet, and the newureg structure built
 *  @return 0 on success and -1 on failure
 **/
int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    // lprintf("starting swexn... cause %d, signaler %d", newureg -> cs, newureg -> signaler);
    //newureg not valid;
    if (!is_valid_newureg(newureg)) ;
    //deregister a handler if exists;
    if (esp3 == NULL || eip == NULL)
    {
        lprintf("all null");
        bzero(&current_thread->swexn_info, sizeof(swexninfo));
        return 0;
    }
    //else register;
    // lprintf("installing...esp:%x, arg:%x,newureg:%x,eip:%x",
    //         (unsigned int)esp3, (unsigned int)arg, (unsigned int)newureg, (unsigned int)eip);
    current_thread -> swexn_info.esp3 = esp3;
    current_thread -> swexn_info.eip = eip;
    current_thread -> swexn_info.arg = arg;
    current_thread -> swexn_info.newureg = newureg;
    current_thread -> swexn_info.installed_flag = 1;
    // if (current_thread -> saved_esp != 0)
    // {
    //     lprintf("The saved esp is %x",(unsigned int)current_thread -> saved_esp);
    //     set_esp0(current_thread -> saved_esp);
    //     
    // }
    // lprintf("The si %x",(unsigned int)get_esp0());
    // lprintf("End swexn %x", (unsigned int)newureg -> esp);
    // MAGIC_BREAK;
    return 0;
}
/** @file sys_aswexn.c
 *
 *  @brief Defines Asynchronous Software Exceptions system calls
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
#include <ureg.h>
#include "sys_aswexn.h"
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
#include "memory/vm_routines.h"
#include "process/scheduler.h"


static int sys_real_asignal(TCB *tcb, int signum)
{
    // If the signal is not in the queue and the 1<<signum bit in
    // the mask is turned on, we enqueue this signal
    if (tcb -> signals[signum - MIN_SIG] != SIGNAL_ENQUEUED && \
            ((tcb -> mask >> signum) & 0x1) == 1)
    {
        signal_t *sig = make_signal_node(current_thread -> tid, signum);
        mutex_lock(&tcb -> tcb_mutex);
        list_insert_last(&tcb -> pending_signals, &sig -> signal_list_node);
        if (tcb -> state == THREAD_SIGNAL_BLOCKED || tcb -> state == THREAD_READLINE ||
                tcb -> state == THREAD_WAITING || tcb -> state == THREAD_SLEEPING)
        {
            if (tcb -> state != THREAD_SIGNAL_BLOCKED) 
                tcb -> has_aborted_sys_flag = 1;
            tcb -> state = THREAD_RUNNABLE;
            mutex_lock(&blocked_queue_lock);
            list_delete(&blocked_queue, &tcb -> thread_list_node);
            mutex_unlock(&blocked_queue_lock);

            mutex_lock(&runnable_queue_lock);
            list_insert_last(&runnable_queue, &tcb -> thread_list_node);
            mutex_unlock(&runnable_queue_lock);

        }
        mutex_unlock(&tcb -> tcb_mutex);
    }
    return 0;
}


/** @brief The system land asignal implementation
 *
 *  @param thread id, and signal nuber
 *  @return -1 on failure, 0 on success
 **/
int sys_asignal(int tid, int signum)
{
    if (signum < MIN_SIG || signum > MAX_SIG)
    {
        lprintf("no good");
        return -1;
    }

    // Can the thread send the signal to itself?
    if (current_thread -> tid == tid)
    {
        lprintf("Send signal to itself okaaaaaay???");
        // return -1;
    }

    node *n;
        lprintf("The tid is %d and signum is %d", tid, signum);

    if (tid > 0)
    {
        lprintf("Tid is > 0");
        // If the target tid is in the blocked queue
        mutex_lock(&blocked_queue_lock);
        for (n = list_begin(&blocked_queue);
                n != NULL;
                n = n -> next)
        {
            lprintf("Lopo");
            TCB *tcb = list_entry(n, TCB, thread_list_node);
            if (tcb -> tid == tid)
            {
                lprintf("Find tid");
                mutex_unlock(&blocked_queue_lock);

                return sys_real_asignal(tcb, signum);
            }
        }

        mutex_unlock(&blocked_queue_lock);

        // If the target tid is in the runnable queue
        mutex_lock(&runnable_queue_lock);
        for (n = list_begin(&runnable_queue);
                n != NULL;
                n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, thread_list_node);
            if (tcb -> tid == tid)
            {
                mutex_unlock(&runnable_queue_lock);
                lprintf("Find tid");
                
                return sys_real_asignal(tcb, signum);
            }
        }
                mutex_unlock(&runnable_queue_lock);
        
    }
    // Since our implementation rejects process make syscalls like fork or exec when
    // it's already multi-threaded, we will assume
    // that the tid can be only possibly found in the current process's threads
    else if (tid < 0)
    {
        lprintf("Tid is < 0");

        PCB *target_process = NULL;
        TCB *target_thread = NULL;
        int exist = 0;
        node *n;
        mutex_lock(&blocked_queue_lock);

        for (n = list_begin(&blocked_queue);
                n != NULL;
                n = n -> next)
        {
            target_thread = list_entry(n, TCB, thread_list_node);
            if (target_thread -> tid == -tid)
            {
                exist = 1;
                break;
            }
        }
        mutex_unlock(&blocked_queue_lock);
        if (!exist)
        {
            mutex_lock(&runnable_queue_lock);
            for (n = list_begin(&runnable_queue);
                    n != NULL;
                    n = n -> next)
            {
                target_thread = list_entry(n, TCB, thread_list_node);
                if (target_thread -> tid == -tid)
                {
                    exist = 1;
                    break;
                }
            }
        }
        mutex_unlock(&runnable_queue_lock);
        // If such thread exists, we send signal to all threads in the list
        if (exist)
        {
            target_process = target_thread -> pcb;
            for (n = list_begin(&target_process -> threads);
                    n != NULL;
                    n = n -> next)
            {
                TCB *tcb = list_entry(n, TCB, peer_threads_node);
                /* send signal to each of the tcb in the process*/
                sys_real_asignal(tcb, signum);
            }
            return 0;
        }
    }
    lprintf("Finally, tid doens't exist");
    // When there is no valid condition satisfied, return -1
    return -1;
}

/** @brief The system land await implementation
 *
 *
 *  @param mask
 *  @return -1 on failure, 0 on success
 **/
int sys_await(sigmask_t mask)
{
    if (mask == 0 && !current_thread -> swexn_info.installed_flag)
    {
        return -1;
    }
    // to achieve atmoitity (possibly vs amask) please think of if deadlock can happen???
    mutex_lock(&current_thread -> tcb_mutex);
    sigmask_t old_mask = current_thread -> mask;
    current_thread -> mask = mask;
    current_thread -> state = THREAD_SIGNAL_BLOCKED;
    lprintf("sys_await will call schedule!");
    mutex_unlock(&current_thread -> tcb_mutex);

    schedule(-1);
    mutex_lock(&current_thread -> tcb_mutex);

    lprintf("After schedule in await");
    current_thread -> mask = old_mask;
    mutex_unlock(&current_thread -> tcb_mutex);
    return 0;
}

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_amask(sigaction_t action, sigmask_t mask, sigmask_t *oldmaskp)
{
    // TODO, check if oldmaskp is in userspace and valid
    mutex_lock(&current_thread -> tcb_mutex);
    if (oldmaskp != NULL)
    {
        /* check valid address first*/
        if (!is_user_addr(oldmaskp) || !addr_has_mapping(oldmaskp)) return -1;
        *oldmaskp = current_thread -> mask;
    }
    switch (action)
    {
    case ASWEXN_SET:
        current_thread -> mask = mask;
        break;

    case ASWEXN_ADD:
        current_thread -> mask |= mask;
        break;

    case ASWEXN_SUBTRACT:
        current_thread -> mask &= (~mask);
        break;

    default:
        mutex_unlock(&current_thread -> tcb_mutex);
        return -1;
    }
    mutex_unlock(&current_thread -> tcb_mutex);

    return 0;
}

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_atimer(int mode, int period)
{
    if (period < 0)
    {
        return -1;
    }
    mutex_lock(&current_thread -> tcb_mutex);

    if (period == 0)
    {
        switch (mode)
        {
        case ASWEXN_VIRTUAL:
            current_thread -> virtual_mode = MODE_OFF;
            current_thread -> virtual_period = 0;
            current_thread -> virtual_tick = 0;
            break;

        case ASWEXN_REAL:
            if (list_search_tid(&alarm_list, current_thread -> tid) != NULL)
            {
                list_delete(&alarm_list, &current_thread -> alarm_list_node);
                current_thread -> real_period = 0;
                current_thread -> real_tick = 0;
            }
            break;

        default:
            mutex_unlock(&current_thread -> tcb_mutex);

            return -1;
        }
    }
    else
    {
        switch (mode)
        {
        case ASWEXN_VIRTUAL:
            current_thread -> virtual_mode = MODE_ON;
            current_thread -> virtual_period = period;
            current_thread -> virtual_tick = 0;
            break;

        case ASWEXN_REAL:
            list_insert_last(&alarm_list, &current_thread -> alarm_list_node);
            current_thread -> real_period = period;
            current_thread -> real_tick = 0;
            break;

        default:
            mutex_unlock(&current_thread -> tcb_mutex);
            return -1;

        }
    }
    mutex_unlock(&current_thread -> tcb_mutex);
    return 0;
}

/** @brief Pack the sender and the signal number into a node
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
signal_t *make_signal_node(int sender, int signum)
{
    signal_t *sig = (signal_t *)malloc(sizeof(signal_t));
    sig -> cause = signum;
    sig -> signaler = sender;
    sig -> signal_list_node.prev = NULL;
    sig -> signal_list_node.next = NULL;
    return sig;
}
/** @file sys_life_cycle.c
 *
 *  @brief This file includes the implementation of the life cycle system calls
 *         such as thread_fork, wait, set_status and vanish
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
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
#include "scheduler.h"


#define INIT_PID 2

/** @brief The thread_fork implementation
 *
 *  similar to fork, other than without any memory mangement
 *  @param nothing
 *  @return -1 on failure, 0 to child, child tid to parent
 **/
int sys_thread_fork(void)
{
    // Get all registers pushed on parent thread's kernel stack
    unsigned int *kernel_stack =
        (unsigned int *)(current_thread -> stack_base +
                         current_thread->stack_size - 60);
    current_thread -> registers.ss = kernel_stack[14];
    current_thread -> registers.esp = kernel_stack[13];
    current_thread -> registers.eflags = kernel_stack[12];
    current_thread -> registers.cs = kernel_stack[11];
    current_thread -> registers.eip = kernel_stack[10];
    current_thread -> registers.ebp = kernel_stack[9];
    current_thread -> registers.edi = kernel_stack[8];
    current_thread -> registers.esi = kernel_stack[7];
    current_thread -> registers.edx = kernel_stack[6];
    current_thread -> registers.ecx = kernel_stack[5];
    current_thread -> registers.ebx = kernel_stack[4];
    PCB *parent_pcb = current_thread -> pcb;
    list threads = parent_pcb -> threads;

    /* create a new thread*/
    TCB *child_tcb = (TCB *)malloc(sizeof(TCB));
    child_tcb -> pcb = parent_pcb;
    child_tcb -> tid = next_tid;
    next_tid++;
    child_tcb -> start_ticks = 0;
    child_tcb -> duration = 0;
    mutex_init(&child_tcb->tcb_mutex);
    /* copy thread info*/
    child_tcb -> state = THREAD_INIT;
    /*each thread has its own kernel stack*/

    child_tcb -> stack_size = current_thread -> stack_size;
    child_tcb -> stack_base = smemalign(PAGE_SIZE, child_tcb -> stack_size);
    child_tcb -> esp =
        (uint32_t)child_tcb->stack_base + (uint32_t)child_tcb->stack_size;
    child_tcb -> registers = current_thread -> registers;

    child_tcb -> registers.eax = 0;
    current_thread -> registers.eax = child_tcb -> tid;
    /* put into our global list*/
    list_insert_last(&threads, &child_tcb->peer_threads_node);
    list_insert_last(&runnable_queue, &child_tcb->thread_list_node);
    return child_tcb -> tid;
}

/** @brief Set the return status for the current thread
 *
 *  @param status the return status
 **/
void sys_set_status(int status)
{
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> pcb -> return_state = status;
    mutex_unlock(&current_thread -> tcb_mutex);
    return;
}

/** @brief vanish the current thread, and reap all the exited thread
 *         if the current thread is the last thread in the process.
 *         The current thread, although vanished, is reaped by its
 *         parent process
 **/
void sys_vanish(void)
{
    // Get pcb for current process
    mutex_lock(&current_thread -> tcb_mutex);
    PCB *current_pcb = current_thread -> pcb;

    // Get thread list for the current process
    list threads = current_pcb -> threads;
    node *n;

    // Count how many peers haven't already exited
    int live_count = 0;
    for (n = list_begin (&threads); n != NULL; n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, peer_threads_node);
        if (tcb -> state != THREAD_EXIT)
        {
            live_count++;
        }
    }

    if (live_count == 1) // if this is the last thread
    {
        for (n = list_begin(&threads); n != NULL; n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, peer_threads_node);
            // We only reap threads that is not the current thread
            if (tcb -> tid != current_thread -> tid)
            {
                list_delete(&threads, n);
                list_delete(&blocked_queue, n);
                list_delete(&runnable_queue, n);
                // free kernel stack and tcb
                sfree(tcb -> stack_base, tcb -> stack_size);
                free(tcb);
            }
        }

        // Set the state to be exit
        current_pcb -> state = PROCESS_EXIT;
        mutex_lock(&process_queue_lock);
        list_delete(&process_queue, &current_pcb -> all_processes_node);
        mutex_unlock(&process_queue_lock);
        PCB *parent = current_pcb -> parent;

        // If the parent has already exited, wake up init process
        if (parent == NULL || parent -> state == PROCESS_EXIT)
        {
            TCB *init = NULL;
            for (n = list_begin(&blocked_queue);
                    n != NULL;
                    n = n -> next)
            {
                init = list_entry(n, TCB, thread_list_node);
                if (init -> tid == INIT_PID)
                {
                    // Find init and make it runnable
                    mutex_lock(&blocked_queue_lock);
                    list_delete(&blocked_queue, &init->thread_list_node);
                    mutex_unlock(&blocked_queue_lock);

                    mutex_lock(&init -> tcb_mutex);
                    init -> state = THREAD_RUNNABLE;
                    mutex_unlock(&init -> tcb_mutex);

                    mutex_lock(&runnable_queue_lock);
                    list_insert_last(&runnable_queue, &init->thread_list_node);
                    mutex_unlock(&runnable_queue_lock);
                    break;
                }
            }
        }
        else
        {
             /* Because the parent is still alive, find the waiting parent
               from the block queue that wants to reap this process
               We can ensure that there is only one thread left
               for this process, so it's safe to do so */
            list parent_threads = parent -> threads;
            TCB *waiting_thread = NULL;
            for (n = list_begin(&parent_threads);
                    n != NULL;
                    n = n -> next)
            {
                waiting_thread = list_entry(n, TCB, peer_threads_node);
                // If we find a thread that is waiting, we delete it from the waiting
                // queue and make it runnable
                if (waiting_thread -> state == THREAD_WAITING)
                {
                    mutex_lock(&blocked_queue_lock);
                    list_delete(&blocked_queue, &waiting_thread->thread_list_node);
                    mutex_unlock(&blocked_queue_lock);

                    mutex_lock(&waiting_thread -> tcb_mutex);
                    waiting_thread -> state = THREAD_RUNNABLE;
                    mutex_unlock(&waiting_thread -> tcb_mutex);

                    mutex_lock(&runnable_queue_lock);
                    list_insert_last(&runnable_queue, &waiting_thread->thread_list_node);
                    mutex_unlock(&runnable_queue_lock);
                    break;
                }
            }
        }
    }
    // Set the current state to be exit
    current_thread -> state = THREAD_EXIT;
    mutex_unlock(&current_thread -> tcb_mutex);
    // context switch
    schedule(-1);
}

/** @brief Wait for a target process to exit
 *
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int sys_wait(int *status_ptr)
{
    if (status_ptr != NULL && !is_user_addr(status_ptr)) return -1;

    PCB *current_pcb = current_thread -> pcb;
    list *child_pros = &current_pcb -> children;

    // If the process has no children, kernel won't let it wait
    if (current_pcb -> children_count == 0)
    {
        return -1;
    }
    node *n;

    for (n = list_begin (child_pros); n != NULL; n = n -> next)
    {
        PCB *pcb = list_entry(n, PCB, peer_processes_node);
        // Found one already exited child
        if (pcb -> state == PROCESS_EXIT)
        {
            int pid = pcb -> pid;
            // collects the return status
            if (status_ptr != NULL)
            {
                *status_ptr = pcb -> return_state;
            }
            // Reaps all the threads for this child
            list child_threads = pcb -> threads;

            node *temp = NULL;
            for (temp = list_begin (&child_threads); temp != NULL; temp = temp -> next)
            {
                TCB *tcb = list_entry(temp, TCB, peer_threads_node);
                // Free its kernel stack and tcb
                sfree(tcb -> stack_base, tcb -> stack_size);
                free(tcb);
            }
            // Reap this child
            list_delete(child_pros, &pcb -> peer_processes_node);
            current_pcb -> children_count--;

            // Free page directory and pcb
            destroy_page_directory(pcb -> PD);
            sfree(pcb -> PD, PAGE_SIZE);

            free(pcb);
            return pid;
        }
    }
    // If no children process is exited, wait till one of them exits by
    // calling schedule
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> state = THREAD_WAITING;
    mutex_unlock(&current_thread -> tcb_mutex);
    schedule(-1);


    // Do the thing again once one exited child wake me up
    for (n = list_begin (child_pros); n != NULL; n = n -> next)
    {
        PCB *pcb = list_entry(n, PCB, peer_processes_node);
        // Found one already exited child
        if (pcb -> state == PROCESS_EXIT)
        {
            int pid = pcb -> pid;
            // collects the return status
            if (status_ptr != NULL)
            {
                *status_ptr = pcb -> return_state;
            }
            // Reaps all the threads for this child
            list child_threads = pcb -> threads;
            //lprintf("child_threads count %d", child_threads.length);
            node *temp = NULL;
            for (temp = list_begin (&child_threads); temp != NULL; temp = temp -> next)
            {
                TCB *tcb = list_entry(temp, TCB, peer_threads_node);
                // Free its kernel stack and tcb
                sfree(tcb -> stack_base, tcb -> stack_size);
                free(tcb);
            }
            // Reap this child
            list_delete(child_pros, &pcb -> peer_processes_node);
            current_pcb -> children_count--;

            // Free page directory and control block
            destroy_page_directory(pcb -> PD);
            sfree(pcb -> PD, PAGE_SIZE);

            free(pcb);
            return pid;
        }
    }
    return -1;

}
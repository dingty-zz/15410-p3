/** @file _life_cycle.c
 *
 *  @brief This file includes the implementation of the life cycle funcitons.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
#include "control_block.h"
#include "linked_list.h"
#include "seg.h"
#include "cr.h"
#include "simics.h"
#include "malloc.h"
#include <elf/elf_410.h>
#include "common_kern.h"
#include "string.h"
#include "eflags.h"
#include "mutex_type.h"
#include "simics.h"
#include "vm_routines.h"

extern TCB *current_thread;

int sys_thread_fork(void)
{
    unsigned int *kernel_stack = (unsigned int *)(current_thread -> stack_base + current_thread->stack_size - 52);
    lprintf("the kernel_stack is : %p", kernel_stack);
    current_thread -> registers.ss = kernel_stack[12];
    current_thread -> registers.esp = kernel_stack[11];
    current_thread -> registers.eflags = kernel_stack[10];
    current_thread -> registers.cs = kernel_stack[9];
    current_thread -> registers.eip = kernel_stack[8];
    current_thread -> registers.eax = kernel_stack[7];
    current_thread -> registers.ecx = kernel_stack[6];
    current_thread -> registers.edx = kernel_stack[5];
    current_thread -> registers.ebx = kernel_stack[4];
    current_thread -> registers.ebp = kernel_stack[2];
    current_thread -> registers.esi = kernel_stack[1];
    current_thread -> registers.edi = kernel_stack[0];

    PCB* common_pcb = current_thread -> pcb;
    list* threads = common_pcb -> threads;

    TCB *child_tcb = (TCB *)malloc(sizeof(TCB));
    list_insert_last(threads, child_tcb->peer_threads_node);
    list_insert_last(&runnable_queue, child_tcb->thread_list_node);
    child_tcb -> pcb = common_pcb;
    child_tcb -> tid = next_tid;
    next_tid++;

    child_tcb -> state = THREAD_INIT;
    child_tcb -> stack_size = parent_tcb -> stack_size;
    child_tcb -> stack_base = smemalign(4, child_tcb -> stack_size);
    child_tcb -> esp = stack_base+stack_size;
    child_tcb -> registers = parent_tcb -> registers;

    child_tcb -> registers.eax = 0;

    return child_tcb -> tid;
}

void sys_set_status(int status)
{
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> pcb -> return_state = status;
    mutex_unlock(&current_thread -> tcb_mutex);
    return;
}


void sys_vanish(void)
{
    // Get pcb for current process
    mutex_lock(&current_thread -> tcb_mutex);
    PCB *current_pcb = current_thread -> pcb;

    // display to the console by print()....
    // Set the current state to be exit
    current_thread -> state = THREAD_EXIT;


    list *threads = current_pcb -> threads;

    node *n;
    // Count how many peers haven't already exited
    int live_count = 0;
    for (n = list_begin (threads); n != list_end (threads); n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, threads);
        if (tcb -> state != THREAD_EXIT)
        {
            live_count++;
        }
    }

    if (live_count == 1) // if this is the last thread
    {
        for (n = list_begin(threads); n != list_end(threads); n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, threads);
            // We only reap threads that is not the current thread
            if (tcb -> tid != current_thread -> tid)
            {
                list_delete(threads, n);
                list_delete(&blocked_queue, n);
                list_delete(&runnable_queue, n);
                sfree(tcb -> stack_base, tcb -> stack_size);
                free(tcb);
            }
        }

        pcb -> state = PROCESS_EXIT;

        PCB *parent = current_pcb -> parent;

        // If the parent has already exited, wake up init
        if (parent == NULL || parent -> state == PROCESS_EXIT)
        {

            lprintf(" todo report the state to init");
            MAGIC_BREAK;
        }
        else
        {
            // Because the parent is still alive
            // Find a thread that waits to reap this process(thread)
            // because we can ensure that there is only one thread left
            // for this process
            list *parent_threads = parent -> threads;
            TCB *waiting_thread = NULL;
            for (n = list_begin(&parent_threads); n != list_end(&parent_threads); n = n -> next)
            {
                waiting_thread = list_entry(n, TCB, parent_threads);
                // If we find a thread that is waiting, we delete it from the waiting
                // queue and make it runnable
                if (waiting_thread -> state == THREAD_WAITING)
                {
                    mutex_lock(&blocked_queue_lock);
                    list_delete(&blocked_queue, n);
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
    mutex_unlock(&current_thread -> tcb_mutex);
    // context switch it back
    schedule(-1);

}

int sys_wait(int *status_ptr)
{
    // checkout ptr is not in kernel space, writable or not

    PCB *current_pcb = current_thread -> pcb;
    list *children = current_pcb -> children;
    node *n;

    for (n = list_begin (children); n != list_end (children); n = n -> next)
    {
        PCB *pcb = list_entry(n, PCB, children);
        // Found one already exited child
        if (pcb -> state == PROCESS_EXIT)
        {
            int pid = pcb -> pid;
            // collects the return status
            if (status_ptr != NULL)
            {
                *status_ptr = pcb -> return_state;
            }
            // Reap this child
            list_delete(children, n);

            free(pcb -> PD);

            free(pcb);
            return pid;
        }
    }
    // If no children is exited, wait till one of them exits
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> state = THREAD_WAITING;
    mutex_unlock(&current_thread -> tcb_mutex);

    schedule(-1);
    // Do the thing again once one exited child wake me up
    for (n = list_begin (children); n != list_end (children); n = n -> next)
    {
        PCB *pcb = list_entry(n, PCB, children);
        // Found one already exited child
        if (pcb -> state == PROCESS_EXIT)
        {
            int pid = pcb -> pid;
            // collects the return status
            if (status_ptr != NULL)
            {
                *status_ptr = pcb -> return_state;
            }
            // Reap this child
            list_delete(children, n);

            free(pcb -> PD);

            free(pcb);
            return pid;
        }
    }
    return -1;

}

void destroy_pcb(PCB *pcb)
{

}

void destroy_tcb(TCB *tcb)
{

}
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

extern list process_queue;
extern uint32_t next_pid;
extern int process_create(const char *filename, int run);
void allocate_pages(uint32_t *pd, uint32_t virtual_addr, size_t size);

extern TCB *current_thread;


void _set_status(int status)
{
    mutex_lock(&current_thread -> tcb_mutex);
    current_thread -> pcb -> return_state = status;
    mutex_unlock(&current_thread -> tcb_mutex);
    return;
}


void _vanish(void)
{
    // Get pcb for current process
    PCB *current_pcb = current_thread -> pcb;

    // If the parent has already exited
    if (current_pcb -> parent == NULL)
    {
        // TODO report the status to init
    }
    list *threads = current_pcb -> threads;
    node *n;

    // check how many peers have already exited
    int live_count = 0;
    for (n = list_begin (threads); n != list_end (threads); n = n -> next)
    {
        TCB *tcb = list_entry(n, TCB, threads);
        if (tcb -> state != THREAD_EXIT)
        {
            live_count++;
        }
    }

    if (live_count <= 1) // if this is the last thread
    {
        for (n = list_begin (threads); n != list_end (threads); n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, threads);
            if (tcb -> tid != current_thread -> tid)      // We only free tcb that is not the current tcb
            {
                sfree(tcb -> stack_base, tcb -> stack_size);
                free(tcb);
                list_delete(threads, n);
            }

        }

        // Make the exit status available to parent task, or init
        if (current_pcb -> parent -> state = PROCESS_EXIT)
        {
            // todo report the state to init
        }
        pcb -> state = PROCESS_EXIT;
        cond_signal(pcb -> pcb_condvar);    // signal that I am exited, wait for parent to reap me
    }
    else
    {
        // display to the console by print()....
        current_thread -> state = THREAD_EXIT;

    }
        // pick a next thread to run, same thing in context switch lock needed?
    schedule();


}

int _wait(int *status_ptr)
{
    // checkout ptr is not in kernel space, writable or not

    // whether ptr is NULL
    PCB *current_pcb = current_thread -> pcb;
    list *children = current_pcb -> children
    node *n;
    for (n = list_begin (children); n != list_end (children); n = n -> next)
    {
        PCB *pcb = list_entry(n, PCB, children);
        // Found one already exited child
        if (pcb -> state == THREAD_EXIT)
        {
            int pid = pcb -> pid;
            // collects the return status
            if (status_ptr != NULL)
            {
                *status_ptr = pcb -> return_state;
            }
            // Reap this child
            mutex_destory(pcb -> pcb_mutex);
            // cond_destory(pcb -> pcb_condvar);
            free(pcb -> PD);

            // free lists
            free(pcb);
            return pid;
        }
    }
    // If no children is exited, wait till one of them exits
    for (n = list_begin (children); n != list_end (children); n = n -> next) {
        PCB *pcb = list_entry(n, PCB, children);
        // cond_wait(pcb -> pcb_mutex, pcb -> pcb_condvar);
        // deschedule is needed
        schedule(-1);
    }
    // Reap this child
    //free

    
}

void destroy_pcb(PCB *pcb) {

}

void destroy_tcb(TCB *tcb) {

}
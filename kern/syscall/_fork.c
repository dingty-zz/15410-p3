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
/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
/* two more things to do: 1. copy page table 2. iret*/
int _fork(void)
{

    PCB *child_pcb = (PCB *)malloc(sizeof(PCB));
    TCB *child_tcb = (TCB *)malloc(sizeof(TCB));
    PCB *parent_pcb = current_thread -> pcb;
    TCB *parent_tcb = current_thread;
    //step 1: check if multi threaded; then no permission to fork;
    //to be done. We should add count of threads in pcb
    //.........

    //step 2: set up the thread control block;
    child_tcb -> pcb = child_pcb;
    child_tcb -> tid = next_tid;
    next_tid++;
    child_tcb -> state = THREAD_RUNNING;
    child_tcb -> registers = parent_tcb -> registers;
    // child_tcb -> all_threads;

    //step 3: set up the process control block;
    child_pcb -> special = 0;
    child_pcb -> ppid = parent_pcb -> pid;
    child_pcb -> pid = next_pid;
    next_pid++;
    child_pcb -> state = PROCESS_RUNNING;
    child_pcb -> thread = child_tcb;

    //return values are different;
    child_tcb -> registers.eax = 0;
    parent_tcb -> registers.eax = child_pcb -> pid;


    //create a new page directory for the child, which points to the same page tables;
    uint32_t *parent_table = parent_pcb -> PD;
    child_pcb -> PD = (uint32_t *) smemalign(PD_SIZE * 4, PT_SIZE * 4);
    int i;
    //point to same page tables;
    for (i = 0; i < PD_SIZE; i++)
    {
        (child_pcb -> PD)[i] = parent_table[i];
    }


    //insert child to the list of threads and processes
    list_insert_last(&process_queue, &child_pcb->all_processes);
    list_insert_last(&runnable_queue, &child_tcb->all_threads);
    list_insert_last(&runnable_queue, &parent_tcb->all_threads);

    return 0;
}
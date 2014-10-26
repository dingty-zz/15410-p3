/** @file control_block.h
 *  @brief This file defines the type for TCB and PCB.
 */

#ifndef _CONTROL_B_H
#define _CONTROL_B_H
#include "ureg.h"
#include "linked_list.h"

#define THREAD_EXIT -1
#define THREAD_BLOCKED 0
#define THREAD_RUNNING 1
#define THREAD_RUNNABLE 2

#define PROCESS_EXIT -1
#define PROCESS_BLOCKED 0
#define PROCESS_RUNNING 1
#define PROCESS_RUNNABLE 2


typedef struct PCB_t
{
    int special;  // if this process is idle, then never delete out of queue
    int ppid;
    int pid;
    int state; //running, ready, block

    // PCB_t* parent;
    // PCB_t** children;

    // list *threads;  // Now we only care about single threaded
    struct TCB_t *thread;
    node all_processes;
    // PTE;
    // PDE;
} PCB;


typedef struct TCB_t
{

    PCB *pcb; //process the thread belongs to

    int tid;
    int state;

    ureg_t registers;  // USER stack pointer is in here!
    // node peer_threads;  // Now we only care about single threaded
    node all_threads;

    void *stack_base;           // KERNEL stack base
    unsigned int stack_size;  // 4096 (1 page) by default
    // struct TCB* prev;
    // struct TCB* next;
    // struct TCB* wait_next;
    // struct TCB* wait_prev;
} TCB;




#endif /* _CONTROL_B_H */

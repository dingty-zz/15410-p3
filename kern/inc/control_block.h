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

#ifndef _CONTROL_B_H
#define _CONTROL_B_H

#include "ureg.h"
#include "linked_list.h"
#include "vm_type.h"
#include <elf/elf_410.h>
#include "spinlock_type.h"


#define THREAD_EXIT -1
#define THREAD_BLOCKED 0
#define THREAD_RUNNING 1
#define THREAD_RUNNABLE 2
#define THREAD_DESCHEDULED 3     // only set when deschedule() is called
#define THREAD_SLEEPING 4

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
    int return_state;
    // +++++++++++_t* parent;
    // PCB_t** children;
    // list *threads;  

    // Now we only care about single threaded
    struct TCB_t *thread;
    node all_processes;
    uint32_t* PD;
}PCB;


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
}TCB;


// Information about processes/threads in the kernel
spinlock_t tid_lock;
uint32_t next_tid;

spinlock_t pid_lock;
uint32_t next_pid;

spinlock_t thread_queue_lock;
list thread_queue;

spinlock_t blocked_thread_queue_lock;
list blocked_thread_queue;

spinlock_t process_queue_lock;
list process_queue;

TCB *current_thread;

#endif /* _CONTROL_B_H */

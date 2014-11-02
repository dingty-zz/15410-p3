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
#include "mutex_type.h"


#define THREAD_EXIT -2
#define THREAD_BLOCKED -1
#define THREAD_RUNNING 0
#define THREAD_RUNNABLE 1
#define THREAD_DESCHEDULED 2     // only set when deschedule() is called
#define THREAD_SLEEPING 3
#define THREAD_INIT 4

#define PROCESS_EXIT -2
#define PROCESS_BLOCKED -1
#define PROCESS_RUNNING 0
#define PROCESS_RUNNABLE 1
#define PROCESS_IDLE 2



typedef struct PCB_t
{
    int special;  // if this process is idle, then never delete out of queue
    int pid;
    int state; //running, ready, block
    int return_state;
    PCB_t* parent;      // who creates me

    list *threads;      // All threads that this process has, including self thread  

    list *children // saves all forked child
    node all_processes;
    uint32_t* PD;
    mutex_t pcb_mutex;
    cond_t  pcb_condvar;       // for process exit purposes
}PCB;


typedef struct TCB_t
{
    uint32_t esp;

    PCB *pcb; //process the thread belongs to

    int tid;
    int state;

    void *stack_base;           // KERNEL stack base
    unsigned int stack_size;  // 4096 (1 page) by default
    ureg_t registers;  // USER stack pointer is in here!

    node peer_threads;
    node all_threads;
    mutex_t tcb_mutex;

    // struct TCB* prev;
    // struct TCB* next;
    // struct TCB* wait_next;
    // struct TCB* wait_prev;
}TCB;


// Information about processes/threads in the kernel
mutex_t tid_lock;
uint32_t next_tid;

mutex_t pid_lock;
uint32_t next_pid;

mutex_t thread_queue_lock;
list thread_queue;

mutex_t blocked_thread_queue_lock;
list blocked_thread_queue;

mutex_t process_queue_lock;
list process_queue;

TCB *current_thread;

#endif /* _CONTROL_B_H */

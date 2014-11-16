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
#include "datastructure/linked_list.h"
#include <elf/elf_410.h>
#include "locks/mutex_type.h"
#include "mem_internals.h"

// The thread is exited, set by vanish()
#define THREAD_EXIT -2

// Sets when the thread calls deschedule()
#define THREAD_BLOCKED -1

// The thread is running
#define THREAD_RUNNING 0

// The thread is runnable
#define THREAD_RUNNABLE 1

// The thread is waiting for another thread to exit
#define THREAD_WAITING 2 

// The thread is sleeping by sleep()
#define THREAD_SLEEPING 3

// The thread is just created, has never run
#define THREAD_INIT 4

// The thread is calling readline and should block
#define THREAD_READLINE 5

// The process is in exit state, waiting for parent to reap it
#define PROCESS_EXIT -2
#define PROCESS_BLOCKED -1
#define PROCESS_RUNNING 0
#define PROCESS_RUNNABLE 1
#define PROCESS_IDLE 2



typedef struct PCB_t
{
    // Process id
    int pid;
    // Can be the state listed above
    int state;
    // This is the return state for this process, set by set_status sys-call
    int return_state;
    // The parent that creates this process via fork
    struct PCB_t *parent;

    // All threads that this process has, including self thread
    list threads;

    // Saves all forked children
    list children;

    // The inner node that is used for storing peer-process-queue
    node peer_processes_node;

    // The inner node that is used for all process queue
    node all_processes_node;

    // The page directory pointer for this process
    uint32_t *PD;

    // The number of live children that are created by this process via fork
    int children_count;

    //A list of va_info
    list va;

} PCB;


typedef struct s_info
{
    void *esp3;
    void *eip;
    void *arg;
    ureg_t *newureg;
    int installed_flag;
    unsigned int eflags; //eflags when page fault happens;
} swexninfo;

typedef struct TCB_t
{
    // The current kernel stack pointer that is used for context
    // switching
    uint32_t esp;

    // Process the thread belongs to
    PCB *pcb; 

    // Thread id
    int tid;

    // Thread state, can be several as listed above
    int state;

    // The thread sleeps at this time point
    unsigned int start_ticks;

    // The number of ticks that this thread should sleep
    unsigned int duration;

// Thread kernel stack base pointer
    void *stack_base;   

    // 4096 (1 page) by default        
    unsigned int stack_size;  

    // Registers that the thread is used when it's running in user space
    ureg_t registers;  

    // The inner node that belongs to the list of peer threads created by
    // this process via threadfork
    node peer_threads_node;

    // The inner node that belongs to either runnable queue or blocked queue
    node thread_list_node;

    // The inner node that belongs to the wait queue that waits for a mutex
    node mutex_waiting_queue_node;

    // The mutex that protects this tcb
    mutex_t tcb_mutex;

    // The swexn handler information
    swexninfo swexn_info;

} TCB;


// This points the current running thread, and therefore the kernel
// will know which thread is trapping into the kernel space
TCB *current_thread;

// Information about processes/threads in the kernel

uint32_t next_tid;

uint32_t next_pid;

// Thread that has state THREAD_BLOCKED, THREAD_SLEEPING, 
// THREAD_WAITING and THREAD_READLINE should go into this queue
mutex_t blocked_queue_lock;
list blocked_queue;

// Thread that has state THREAD_RUNNABLE or THREAD_INIT should go in this queue
mutex_t runnable_queue_lock;
list runnable_queue;

mutex_t process_queue_lock;
list process_queue;

// print lock
mutex_t print_lock;

// The number of free physical 
int free_frame_num;

int total_num; //total number of chars in a line (to prevent deleting 410 shell phrases)

#endif /* _CONTROL_B_H */

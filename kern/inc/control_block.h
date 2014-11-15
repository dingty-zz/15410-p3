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

#define THREAD_EXIT -2
#define THREAD_BLOCKED -1
#define THREAD_RUNNING 0
#define THREAD_RUNNABLE 1
#define THREAD_WAITING 2     // only set when deschedule() is called
#define THREAD_SLEEPING 3
#define THREAD_INIT 4
#define THREAD_READLINE 5


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
    int return_state;   // This is the return state for this process, set by set_status
    struct PCB_t* parent;      // who creates me

    list threads;      // All threads that this process has, including self thread  

    list children; // saves all forked child
    node all_processes_node;
    node peer_processes_node; 
    uint32_t* PD;
    int children_count;
    list va; //A list of va_info

}PCB;


typedef struct s_info
{
    void* esp3;
    void* eip;
    void* arg;
    ureg_t* newureg;
    int installed_flag; 
    unsigned int eflags; //eflags when page fault happens;
} swexninfo;

typedef struct TCB_t
{
    uint32_t esp;

    PCB *pcb; //process the thread belongs to

    int tid;
    int state;

    // For thread sleeping
    unsigned int start_ticks;
    unsigned int duration;

    void *stack_base;           // KERNEL stack base
    unsigned int stack_size;  // 4096 (1 page) by default
    ureg_t registers;  // USER stack pointer is in here!

    node peer_threads_node;
    node thread_list_node;
    mutex_t tcb_mutex;
    swexninfo swexn_info;
    node readline_node;
}TCB;


// This points the current running thread
TCB *current_thread;

// Information about processes/threads in the kernel
uint32_t next_tid;

uint32_t next_pid;

// Both THREAD_BLOCKED and THREAD_SLEEPING are put in this queue
mutex_t blocked_queue_lock;
list blocked_queue;

// THREAD_RUNNABLE is put in this queue
mutex_t runnable_queue_lock;
list runnable_queue;


// mutex_t readline_queue_lock;
// mutex_t readline_work_lock;
// list readline_queue;
//-1 if no thread is reading
// int next_read_thread = -1;


// Not sure if this is useful
mutex_t process_queue_lock;
list process_queue;

int free_frame_num;


#endif /* _CONTROL_B_H */

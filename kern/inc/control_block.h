/** @file control_block.h
 *  @brief This file defines the type for TCB and PCB.
 */

#ifndef _CONTROL_B_H
#define _CONTROL_B_H
#include "ureg.h"

 
#define THREAD_EXIT -1
#define THREAD_BLOCKED 0
#define THREAD_RUNNING 1
 #define THREAD_RUNNABLE 2
 
#define PROCESS_EXIT -1
#define PROCESS_BLOCKED 0
#define PROCESS_RUNNING 1
  #define PROCESS_RUNNABLE 2

typedef struct PCB_t {
    int special;  // if this process is idle, then never delete out of queue
    int ppid;
    int pid;
       int state; //running, ready, block

    // PCB_t* parent;
    // PCB_t** children;

    list *threads;

    PTE;
    PDE;
} PCB;


typedef struct TCB_t {

    PCB* pcb; //process the thread belongs to

    int tid;
    int state;
    
    ureg_t registers;
    node peer_threads;
    node all_threads;

    // struct TCB* prev;
    // struct TCB* next;
    // struct TCB* wait_next;
    // struct TCB* wait_prev;
} TCB;




#endif /* _CONTROL_B_H */

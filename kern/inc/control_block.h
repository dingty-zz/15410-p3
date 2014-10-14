/** @file control_block.h
 *  @brief This file defines the type for TCB and PCB.
 */

#ifndef _CONTROL_B_H
#define _CONTROL_B_H

typedef struct PCB {
    int pid;
    int uid;
    int gid;
    int euid;
    void* PC;
    void* SP;
    void* ESP;
    void** register_array;
    PCB_t* parent;
    PCB_t** children;
    int state; //running, ready, block
    void* addr; //starting address
    int priority; //the priority of current process
    int* open_files;
} PCB_t;


typedef struct TCB {
    PCB_t* pcb; //process the thread belongs to
    int priority;
    int tid;
    int state;
    void* addr; //current stack pointer of the thread
    struct TCB* prev;
    struct TCB* next;
    struct TCB* wait_next;
    struct TCB* wait_prev;
} TCB_t;


#endif /* _CONTROL_B_H */

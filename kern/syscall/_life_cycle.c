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

extern list process_queue;
extern uint32_t next_pid;
extern int process_create(const char *filename, int run);
void allocate_pages(uint32_t *pd, uint32_t virtual_addr, size_t size);

extern void enter_user_mode(uint32_t ss,
                            uint32_t esp,
                            uint32_t eflags,
                            uint32_t cs,
                            uint32_t eip,
                            uint32_t eax,
                            uint32_t ecx,
                            uint32_t edx,
                            uint32_t ebx,
                            uint32_t ebp,
                            uint32_t esi,
                            uint32_t edi);
extern TCB *thr_create(simple_elf_t *se_hdr, int run);
extern uint32_t *init_pd();
extern TCB *current_thread;

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
    uint32_t* parent_table = parent_pcb -> PD;
    child_pcb -> PD = (uint32_t *) smemalign(PD_SIZE * 4, PT_SIZE * 4);
    int i;
    //point to same page tables;
    for (i = 0; i < PD_SIZE; i++)
    {
      (child_pcb -> PD)[i] = parent_table[i];
    }


    //insert child to the list of threads and processes
    list_insert_last(&process_queue, &child_pcb->all_processes);
    list_insert_last(&thread_queue, &child_tcb->all_threads);
    list_insert_last(&thread_queue, &parent_tcb->all_threads);

    return 0;
}

int _exec(char *execname, char *argvec[])
{
    char *name = (char *)malloc(strlen(execname));
    memcpy(name, execname, strlen(execname));
    lprintf("The execname is %s", name);
    lprintf("char %s, argvec: %p", execname, argvec);
    int argc = 0;
    while (argvec[argc] != 0)
    {

        argc++;
    }
    char *argv[argc];
    int j = 0;
    for (j = 0; j < argc; ++j)
    {
        argv[j] = (char *)malloc(strlen(argvec[j]));
        strcpy(argv[j], argvec[j]);
    }
    int l = 0;
    for (l = 0; l < argc; ++l)
    {
        lprintf("The is %s\n", argv[l]);
    }


    lprintf("The argc==%d", argc);
      simple_elf_t se_hdr;

    // elf_load_helper(&se_hdr, execname);
    // lprintf("%lx", se_hdr.e_entry);
  PCB *pcb = (PCB *)malloc(sizeof(PCB));
  //create a clean page directory
    pcb -> PD = init_pd();
  // set up pcb for this program
    elf_load_helper(&se_hdr, name);
lprintf("dsfsdf");
        allocate_pages(pcb -> PD, 
      (uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
            getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen, 
      (char *)se_hdr.e_txtstart);
            lprintf("Doneonoenoennoneoeneonoe");
            MAGIC_BREAK;
    pcb -> state = PROCESS_RUNNING;
    pcb -> ppid = 0; // who cares this??
    pcb -> pid = next_pid;
    next_pid++;
    

    // list_init(pcb -> threads);
    TCB *thread = thr_create(&se_hdr, 1); // please see thread.c
    pcb -> thread =  thread;


    list_insert_last(&process_queue, &pcb -> all_processes);


    thread -> pcb = pcb;  // cycle reference :)


    if (0)  // if not run ,we return
    {
        MAGIC_BREAK;

        return 0 ;
    }
    MAGIC_BREAK;
    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    // set up kernel stack pointer possibly bugs here
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));  
    lprintf("this is the esp, %x", (unsigned int)get_esp0());
    /* Load the elf program using the helper function */

    // lprintf("\n");

    lprintf("e_txtstart: %lx", se_hdr.e_txtstart);
    lprintf("e_txtoff: %lu", se_hdr.e_txtoff);
    lprintf("e_txtlen: %lu", se_hdr.e_txtlen);


    lprintf("e_datstart: %lx", se_hdr.e_datstart);
    lprintf("e_datoff: %lu", se_hdr.e_datoff);
    lprintf("e_datlen: %lu", se_hdr.e_datlen);


    lprintf("e_rodatstart: %lx", se_hdr.e_rodatstart);
    lprintf("e_rodatoff: %lu", se_hdr.e_rodatoff);
    lprintf("e_rodatlen: %lu", se_hdr.e_rodatlen);


    lprintf("e_bssstart: %lx", se_hdr.e_bssstart);
    lprintf("e_bsslen: %lu", se_hdr.e_bsslen);


    lprintf("before zeroth break");
    // *(int *)0xffffffff=3;

    //MAGIC_BREAK;

    /* Allocate memory for every area */
    allocate_pages(pcb -> PD, 
      (uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
    // MAGIC_BREAK;

    allocate_pages(pcb -> PD, 
      (uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    allocate_pages(pcb -> PD, 
      (uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
    allocate_pages(pcb -> PD, 
      (uint32_t)0xfffff000, 4096); // possibly bugs here

    lprintf("allocate_pages done!");
    // *(int *)0xffffffff=3;

    // MAGIC_BREAK;
    // /* copy data from data field */
    getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen, 
      (char *)se_hdr.e_datstart);

    getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen, 
      (char *)se_hdr.e_rodatstart);
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);


    char *dest = (char *)0xffffffff;
    char *vector[argc + 1];

    int k = 0;
    for (k = 0; k < argc; k++)
    {
        dest -= (strlen(argv[k]) + 1);
        strcpy(dest, argv[k]);
        vector[k] = dest;
    }
    vector[argc] = NULL;
    for (k = 0; k <= argc; k++) lprintf("The vector :%p", vector[k]);
    dest -= sizeof(int) * 5;
    memcpy(dest, vector, sizeof(int) * 5);
    // for (k =0;k<=argc;k++) lprintf("The vector :%s",dest[k]);
    MAGIC_BREAK;

    *((unsigned int *)(thread -> registers.esp)) = 0xffffc000;
    thread -> registers.esp -= 4;

    *(unsigned int *)thread -> registers.esp = 0xffffffff;
    thread -> registers.esp -= 4;


    *((unsigned int *)(thread -> registers.esp)) = (unsigned int)dest;
    thread -> registers.esp -= 4;
    lprintf("The argv %s", ((char **)dest)[0]);
    lprintf("The argc %s", ((char **)dest)[1]);
    lprintf("The argc %s", ((char **)dest)[2]);
    lprintf("The argc %s", ((char **)dest)[3]);
    *(int *)thread -> registers.esp = argc;

    thread -> registers.esp -= 4;


    MAGIC_BREAK;


    lprintf("before second break");

    MAGIC_BREAK;

    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));  // set up kernel stack pointer possibly bugs here
    lprintf("this is the esp, %x", (unsigned int)get_esp0());

    enter_user_mode(thread -> registers.edi,     // let it run, enter ring 3!
                    thread -> registers.esi,
                    thread -> registers.ebp,
                    thread -> registers.ebx,
                    thread -> registers.edx,
                    thread -> registers.ecx,
                    thread -> registers.eax,
                    thread -> registers.eip,
                    thread -> registers.cs,
                    thread -> registers.eflags,
                    thread -> registers.esp,
                    thread -> registers.ss);
    return 0;
}

// void set_status(int status)
// {
//     current_thread -> pcb -> return_state = status;
//     return;

// }


void vanish(void)
{
    // list_delete(&thread_queue, &current_thread -> all_threads);

    // PCB *pcb = current_process = current_thread -> pcb;

    // if (list_length(pcb -> peer_threads) == 1) // if this is the last thread
    // {
    //     destroy_page_directory(pcb -> PD);
    //     sfree(current_thread -> stack_base, current_thread -> stack_size);
    //     // Make the exit status available to parent task, or init
    //     free(tcb);
    //     free(pcb);
    // } else {
    //     // display to the console
    //     // set_status(-2)
    //     // free resources
    //     sfree(current_thread -> stack_base, current_thread -> stack_size);
    //     free(tcb);
    // }
        // pick a next thread to run, same thing in context switch
    while(1) {

    }

}

// int wait(int *status_ptr)
// {

//     return -1;

// }


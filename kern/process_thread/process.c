/** @file process.c
 *
 *  @brief This file defines process manipulation routines
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
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

// list thread_queue;
// uint32_t next_tid = 0 ;

uint32_t next_pid=0;

void allocate_page(uint32_t virtual_addr, size_t size);
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


extern TCB *current_thread;

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
int process_init()
{


    // create a list of run queues based on threads
    list_init(&process_queue);
    next_pid = 1;
    return 0;
}


/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
int process_create(const char *filename, int run)
{

    /* Load the elf program using the helper function */
    simple_elf_t se_hdr;
    lprintf("\n");
    elf_load_helper(&se_hdr, filename);
    lprintf("%lx", se_hdr.e_entry);
    lprintf("%d", machine_phys_frames());
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
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    //create a clean page directory
    pcb -> pd_ptr = smemalign(PD_SIZE, sizeof(PD));
    memset(pcb -> pd_ptr,0,PT_SIZE*sizeof(PT*));

    /* Allocate memory for every area */
    allocate_page((uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
    // MAGIC_BREAK;
    allocate_page((uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    allocate_page((uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    allocate_page((uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
    allocate_page((uint32_t)0xfffff000, 4096); // possibly bugs here

    lprintf("sdfds");
    // *(int *)0xffffffff=3;

    // MAGIC_BREAK;
    // /* copy data from data field */
    getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen, (char *)se_hdr.e_datstart);
    getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen, (char *)se_hdr.e_txtstart);
    getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen, (char *)se_hdr.e_rodatstart);
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);

    // set up pcb for this program
    
    pcb -> state = PROCESS_RUNNING;
    pcb -> ppid = 0; // who cares this??
    pcb -> pid = next_pid;
    next_pid++;
    

    // list_init(pcb -> threads);
    TCB *thread = thr_create(&se_hdr, run); // please see thread.c
    pcb -> thread =  thread;


    list_insert_last(&process_queue, &pcb -> all_processes);


    thread -> pcb = pcb;  // cycle reference :)


    if (!run)  // if not run ,we return
    {
        MAGIC_BREAK;

        return 0 ;
    }
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

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  1. Exit all of the threads
 *  2. Deallocate all of the memory, i.e., free physical frames, unmap user virtual memories
 *  3. Deallocate all of it's source, i.e., free page tables, free page directory except kernel
 *     mappings, free PCB. But where to put the mappings given PCB to be freed?
 *
 *  @param address address must be both physical address and 4KB aligned (really ?)
 **/
 // Possibly be vanish??
int process_exit()
{

    return 0;

}







/** @file process.c
 *
 *  @brief This file defines process manipulation routines
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "control_block.h"
#include "datastructure/linked_list.h"
#include "seg.h"
#include "cr.h"
#include "simics.h"
#include "malloc.h"
#include <elf/elf_410.h>
#include "common_kern.h"
#include "string.h"
#include "eflags.h"
#include "locks/mutex_type.h"
#include "enter_user_mode.h"
#include "thread/thread_basic.h"
#include "memory/vm_routines.h"
#include "process.h"
#include "assert.h"
extern TCB *current_thread;

/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical
 address and 4KB aligned (really ?)
 **/
void process_init()
{
    // create a list of run queues based on threads
    list_init(&process_queue);
    mutex_init(&process_queue_lock);
    next_pid = 1;
}


/** @brief Release a frame frame and mark it as freed only when refcount = 0.
 *         If so, let free_frame point to it.
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param address address must be both physical
 address and 4KB aligned (really ?)
 **/
int process_create(const char *filename, int run)
{

    lprintf("%s", filename);
    PCB *process = (PCB *)malloc(sizeof(PCB));

    //create a clean page directory
    process -> PD = init_pd();
    lprintf("The pd is for this process is %x", (unsigned int)process->PD);
    
    simple_elf_t se_hdr;
    int result = elf_load_helper(&se_hdr, filename);
        lprintf("result is %d", result);

    if (result == NOT_PRESENT || result == ELF_NOTELF)
    {
        // error, the file doesn't exist
        MAGIC_BREAK;
    }
    // set up process for this program
    process -> state = PROCESS_RUNNABLE;   // currently unused
    process -> pid = next_pid;
    next_pid++;
    process -> return_state = 0;
    process -> children_count = 0;
    process -> parent = NULL;

    // lprintf("shabi1");
    // MAGIC_BREAK;

    list_init(&process -> threads);
    list_init(&process -> children);

    list_insert_last(&process_queue, &process -> all_processes_node);

    // Load the program, copy the content to the memory and get the eip
    unsigned int eip = program_loader(se_hdr, process);

        // lprintf("shabi2");
        // MAGIC_BREAK;
    // Create a single thread for this process
    TCB *thread = thr_create(eip, run); // please see thread.c
    list_insert_last(&process -> threads, &thread -> peer_threads_node);


    thread -> pcb = process;  // cycle reference :)

    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    // set up kernel stack pointer possibly bugs here
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));
    
    *((unsigned int *)(current_thread -> registers.esp)) = 0xffffc000;
    current_thread -> registers.esp -= 4;

    *(unsigned int *)current_thread -> registers.esp = 0xffffffff;
    current_thread -> registers.esp -= 12;

    // MAGIC_BREAK;
    if (!run)  // if not run ,we return
    {
        MAGIC_BREAK;
        return 0;
    }
    // MAGIC_BREAK;
    lprintf("let it run, enter ring 3!, thread -> registers.eip%x", (unsigned int)thread->registers.eip);
    enter_user_mode(thread -> registers.edi,
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
 *  2. Deallocate all of the memory, i.e., free physical frames,
  unmap user virtual memories
 *  3. Deallocate all of it's source, i.e., free page tables,
 free page directory except kernel
 *     mappings, free PCB. But where to put the mappings
 given PCB to be freed?
 *
 *  @param address address must be both physical address
 and 4KB aligned (really ?)
 **/
// Possibly be vanish??
int process_exit()
{

    return 0;

}


unsigned int program_loader(simple_elf_t se_hdr, PCB *process) {

    // lprintf("this is the esp, %x", (unsigned int)get_esp0());

    // /* Load the elf program using the helper function */

    // lprintf("\n");
    // lprintf("e_txtstart: %lx", se_hdr.e_txtstart);
    // lprintf("e_txtoff: %lu", se_hdr.e_txtoff);
    // lprintf("e_txtlen: %lu", se_hdr.e_txtlen);


    // lprintf("e_datstart: %lx", se_hdr.e_datstart);
    // lprintf("e_datoff: %lu", se_hdr.e_datoff);
    // lprintf("e_datlen: %lu", se_hdr.e_datlen);


    // lprintf("e_rodatstart: %lx", se_hdr.e_rodatstart);
    // lprintf("e_rodatoff: %lu", se_hdr.e_rodatoff);
    // lprintf("e_rodatlen: %lu", se_hdr.e_rodatlen);


    // lprintf("e_bssstart: %lx", se_hdr.e_bssstart);
    // lprintf("e_bsslen: %lu", se_hdr.e_bsslen);


    /* Allocate memory for every area */
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
    // MAGIC_BREAK;

    allocate_pages(process -> PD,
                   (uint32_t)0xffffe000, 8192); // possibly bugs here

    lprintf("allocate_pages done!");
    // *(int *)0xffffffff=3;

    int result = 0;
    // MAGIC_BREAK;
    // /* copy data from data field */
    result += getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen,
             (char *)se_hdr.e_datstart);
    // MAGIC_BREAK;
    result += getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen,
             (char *)se_hdr.e_txtstart);
    result += getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen,
             (char *)se_hdr.e_rodatstart);
    assert(result > 0);
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);

    // map_readonly(process -> PD,
    //                (uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    // map_readonly(process -> PD,
    //                (uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    // map_readonly(process -> PD,
    //                (uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);


    return se_hdr.e_entry;
}





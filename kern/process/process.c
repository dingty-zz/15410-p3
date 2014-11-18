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

/** @brief initializes the process system
*/
void process_init()
{
    // create a list of run queues based on threads
    list_init(&process_queue);
    mutex_init(&process_queue_lock);
    mutex_init(&print_lock);
    next_pid = 1;
}


/** @brief load a new process
 *
 *  @param filename the filename
 *  @param run whether to run or not
 *  @return 0 on success, -1 otherwise
 **/
int process_create(const char *filename, int run)
{

    PCB *process = (PCB *)malloc(sizeof(PCB));

    //create a clean page directory
    process -> PD = init_pd();
    
    simple_elf_t se_hdr;
    int result = elf_load_helper(&se_hdr, filename);

    if (result == NOT_PRESENT || result == ELF_NOTELF)
    {
        assert("file not exist");
    }
    // set up process for this program
    process -> state = PROCESS_RUNNABLE;   // currently unused
    process -> pid = next_pid;
    next_pid++;
    process -> return_state = 0;
    process -> children_count = 0;
    process -> parent = NULL;

    list_init(&process -> threads);
    list_init(&process -> children);
    list_init(&process -> va);
    list_insert_last(&process_queue, &process -> all_processes_node);

    // Load the program, copy the content to the memory and get the eip
    unsigned int eip = program_loader(se_hdr, process);

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

    if (!run)  // if not run ,we return
    {
        return 0;
    }
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



unsigned int program_loader(simple_elf_t se_hdr, PCB *process) {


    /* Allocate memory for every area */
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
    allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);

    allocate_pages(process -> PD,
                   (uint32_t)0xffffe000, 8192); 


    int result = 0;
    /* copy data from data field */
    result += getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen,
             (char *)se_hdr.e_datstart);
    result += getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen,
             (char *)se_hdr.e_txtstart);
    result += getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen,
             (char *)se_hdr.e_rodatstart);
    assert(result > 0);
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);


    return se_hdr.e_entry;
}
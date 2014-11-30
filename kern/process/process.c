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
 #include <x86/asm.h>   
#include <page.h>
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
    total_num = 0;
}


/** @brief load a new process
 *         When error happens, because only kernel will call this function,
 *         (in kernel.c), there is no way for us to report error to somebody
 *         else, so we choose to panic
 *  @param filename the filename
 *  @param run whether to run or not
 *  @return 0 on success, -1 otherwise
 **/
int process_create(const char *filename, int run)
{
    PCB *process = (PCB *)malloc(sizeof(PCB));

    if (process == NULL)
    {
        panic("Memory allocation fails!");
    }
    //create a clean page directory
    process -> PD = init_pd();
    if (process -> PD == 0)
    {
        panic("Can't creat this process due to allocation fails");
    }
    
    simple_elf_t se_hdr;
    int result = elf_load_helper(&se_hdr, filename);

    if (result == NOT_PRESENT || result == ELF_NOTELF)
    {
        panic("file not exist");
    }
    // set up pcb for this process
    process -> state = PROCESS_RUNNABLE;  
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

    // When the kernel will load a process, it will assume always success
    // unless run out of memory
    assert(eip > 0);
    lprintf("The eip is %x",(unsigned int)eip);
    // Create a single thread for this process
    TCB *thread = thr_create(eip, run); // please see thread_basic.c
    if (thread == NULL)
    {
        panic("Couldn't set up a tcb due to memory allocation error");
    }
    list_insert_last(&process -> threads, &thread -> peer_threads_node);


    thread -> pcb = process;  // cycle reference :)

    /* We need to do this every time for a thread to run */
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
    // Because we know that both idle and init is loaded, we can safely
    // enable interrupts to enable preemption
    enable_interrupts();
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


/* >0 on success, 0 fails  */
unsigned int program_loader(simple_elf_t se_hdr, PCB *process) {


    /* Allocate memory for every area */
    int result = 0;
    result = allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    if (result == -1)
    {
        return 0;
    }
    result = allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
        if (result == -1)
    {
        return 0;
    }
    result = allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
        if (result == -1)
    {
        return 0;
    }
    result = allocate_pages(process -> PD,
                   (uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
        if (result == -1)
    {
        return 0;
    }

    result = allocate_pages(process -> PD,
                   (uint32_t)0xffffe000, 2 * PAGE_SIZE); 
        if (result == -1)
    {
        return 0;
    }


 
    /* copy data from data field */
    result += getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen,
             (char *)se_hdr.e_datstart);
    result += getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen,
             (char *)se_hdr.e_txtstart);
    result += getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen,
             (char *)se_hdr.e_rodatstart);
    if (result < 0)
    {
        return 0;
    }
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);


    return se_hdr.e_entry;
}
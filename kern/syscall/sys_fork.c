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
#include "mutex_type.h"

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
/* two more things to do: 1. copy page table 2. iret*/
int sys_fork(void)
{
    //update current threads' registers
    unsigned int *kernel_stack = (unsigned int *)(current_thread -> stack_base + current_thread->stack_size - 52);
    lprintf("the kernel_stack is : %p", kernel_stack);
    current_thread -> registers.ss = kernel_stack[12];
    current_thread -> registers.esp = kernel_stack[11];
    current_thread -> registers.eflags = kernel_stack[10];
    current_thread -> registers.cs = kernel_stack[9];
    current_thread -> registers.eip = kernel_stack[8];
    current_thread -> registers.eax = kernel_stack[7];
    current_thread -> registers.ecx = kernel_stack[6];
    current_thread -> registers.edx = kernel_stack[5];
    current_thread -> registers.ebx = kernel_stack[4];
    current_thread -> registers.ebp = kernel_stack[2];
    current_thread -> registers.esi = kernel_stack[1];
    current_thread -> registers.edi = kernel_stack[0];

    //start forking
    PCB *child_pcb = (PCB *)malloc(sizeof(PCB));
    TCB *child_tcb = (TCB *)malloc(sizeof(TCB));
    PCB *parent_pcb = current_thread -> pcb;
    TCB *parent_tcb = current_thread;
    //step 1: check if multi threaded; then no permission to fork;
    //to be done.
    //.........
    // if (current_thread -> pcb -> threads -> length != 1)
    //      return -1;

    //step 2: set up the thread control block;
    child_tcb -> pcb = child_pcb;
    child_tcb -> tid = next_tid;
    next_tid++;
    child_tcb -> state = THREAD_INIT;
    child_tcb -> stack_size = parent_tcb -> stack_size;
    child_tcb -> stack_base = smemalign(4, child_tcb -> stack_size);
    child_tcb -> esp = child_tcb -> stack_base + child_tcb -> stack_size;
    /*copy the general user space registers*/
    // push_registers(current_thread -> registers,child_tcb->base);

    child_tcb -> registers = parent_tcb -> registers;
    lprintf("%u",child_tcb->registers.eip);

    //step 3: set up the process control block;
    child_pcb -> special = 0;
    child_pcb -> pid = next_pid;
    next_pid++;
    child_pcb -> state = PROCESS_RUNNING;
    child_pcb -> parent = parent_pcb;
    list_insert_last(child_pcb -> threads, child_tcb->thread_list_node);
    list_insert_last(parent_pcb -> children, child_pcb -> all_processes_node);

    //return values are different;
    child_tcb -> registers.eax = 0;
    parent_tcb -> registers.eax = child_pcb -> pid;

    //create a new page directory for the child, which points to the same page tables;
    uint32_t* parent_directory = parent_pcb -> PD;
    child_pcb -> PD = (uint32_t *) smemalign(PD_SIZE * 4, PT_SIZE * 4);
    int i,j;
    // copy kernel mappings first
    for (i = 0; i < 4; i++)
    {
        (child_pcb->PD)[i] = parent_directory[i];
    }
    // copy user mappings by allocating new frames for each mapping
    for (i = 4; i < PD_SIZE; i++)
    {
        //parent directory entry info
        uint32_t parent_de_raw = parent_directory[i];
        uint32_t parent_de = parent_de_raw & 0xfffff000;
        if (parent_de == 0)  continue;
        //child direcotory entry info
        uint32_t child_de = (uint32_t)smemalign(PT_SIZE * 4, PT_SIZE * 4);
        uint32_t child_de_raw = child_de | ((uint32_t)parent_de_raw & 0xfff);
        child_pcb->PD[i] = child_de_raw;
        //copy page table
        for (j = 0; j < PT_SIZE; j++)
        {
            //page table entry info
            uint32_t parent_pte_raw = ((uint32_t*)parent_de) [j];
            uint32_t parent_pte = parent_pte_raw & 0xfffff000;
            if (parent_pte == 0)  continue;
            lprintf("page table entry: %d",j);
            virtual_map_physical(child_pcb -> PD,i,j);
            lprintf("new physical addr: %u", (unsigned int)((uint32_t*)child_de) [j]);
            uint32_t child_pte = ((uint32_t*)child_de) [j] & 0xfffff000;
            ((uint32_t*)child_de)[j] = child_pte | 
                                      (parent_pte_raw & 0xfff);    
            //turn off paging;
            lprintf("start copying physical addr!");
            set_cr0(get_cr0() & (~CR0_PG));
            //copy all the data;
            memcpy((void*)child_pte, (void*)parent_pte, PAGE_SIZE*4);
            //turn on paging;
            set_cr0(get_cr0() | CR0_PG);
        }
    }
    lprintf("finished!");

    // insert child to the list of threads and processes
    list_insert_last(&process_queue, &child_pcb->all_processes);
    list_insert_last(&runnable_queue, &child_tcb->all_threads);
    // list_insert_last(&thread_queue, &parent_tcb->all_threads);
    lprintf("ready to return! child pid:%d", child_pcb -> pid);
    MAGIC_BREAK;   
    return child_pcb -> pid;
}

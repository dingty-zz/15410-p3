/** @file sys_fork.c
 *
 *  @brief This file includes the implementation fork using ZFOD.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
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
#include "memory/vm_routines.h"
#include "mem_internals.h"

#define DEFLAG_ADDR(x)           (x & 0xfffff000)
#define ADDFLAG(x,flag)          (x | flag)
#define GET_FLAG(x)              (x & 0xfff)

typedef struct entry_info
{
    int pd_index;
    int pt_index;
    uint32_t free_virtual_addr;
} entry_struct;

static entry_struct entry;

/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
int find_free_entry(uint32_t *parent_directory)
{
    int i, j;
    for (i = 4; i < PD_SIZE; ++i)
    {
        uint32_t current_pde = DEFLAG_ADDR(parent_directory[i]);
        if (current_pde == 0) continue;
        for (j = 0; j < PT_SIZE; ++j)
        {
            uint32_t current = DEFLAG_ADDR(((uint32_t *)current_pde)[j]);
            if (current == 0)
            {
                entry.pd_index = i;
                entry.pt_index = j;
                entry.free_virtual_addr = (i << 22) | (j << 12);
                return 1;
                return 1;
            }
        }
    }
    return 0;
}

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

    /* step 0 : check if multi threaded; if so, then no permission to fork */

    // if (current_thread -> pcb -> threads -> length != 1)
    //      return -1;

    /* Step 1: update current threads' registers */
    unsigned int *kernel_stack =
        (unsigned int *)(current_thread -> stack_base +
                         current_thread->stack_size - 60);
    //lprintf("the kernel_stack is : %p", kernel_stack);
    current_thread -> registers.ss = kernel_stack[14];
    current_thread -> registers.esp = kernel_stack[13];
    current_thread -> registers.eflags = kernel_stack[12];
    current_thread -> registers.cs = kernel_stack[11];
    current_thread -> registers.eip = kernel_stack[10];
    current_thread -> registers.ebp = kernel_stack[9];
    current_thread -> registers.edi = kernel_stack[8];
    current_thread -> registers.esi = kernel_stack[7];
    current_thread -> registers.edx = kernel_stack[6];
    current_thread -> registers.ecx = kernel_stack[5];
    current_thread -> registers.ebx = kernel_stack[4];
    //lprintf("The ebp is %x", (unsigned int)current_thread -> registers.ebp);
    //lprintf("The eip is %x", (unsigned int)current_thread -> registers.eip);
    //lprintf("The cs is %x", (unsigned int)current_thread -> registers.cs);
    //lprintf("The ss is %x", (unsigned int)current_thread -> registers.ss);

    /* Step 2: create new task control block */
    PCB *child_pcb = (PCB *)malloc(sizeof(PCB));
    if (child_pcb == NULL)
    {
        return -1;
    }
    TCB *child_tcb = (TCB *)malloc(sizeof(TCB));
    if (child_tcb == NULL)
    {
        return -1;
    }
    PCB *parent_pcb = current_thread -> pcb;
    TCB *parent_tcb = current_thread;
    uint32_t *parent_directory = parent_pcb -> PD;
    //Find a temprorary entry in parent pd for temporary mapping
    //with the new physical frame
    if (!find_free_entry(parent_directory))
    {
        //didn't successfully find a free entry in current pcb;
        return -1;
    }

    /* Step 3: set up the thread control block */
    mutex_init(&child_tcb -> tcb_mutex);
    child_pcb -> children_count=0;
    child_tcb -> pcb = child_pcb;
    child_tcb -> tid = next_tid;
    next_tid++;
    child_tcb -> state = THREAD_INIT;
    child_tcb -> stack_size = parent_tcb -> stack_size;
    child_tcb -> stack_base = memalign(4, child_tcb -> stack_size);
    if (child_tcb -> stack_base == NULL)
    {
        return -1;
    }
    child_tcb -> esp = (uint32_t)child_tcb -> stack_base +
                       (uint32_t)child_tcb -> stack_size;
    child_tcb -> registers = parent_tcb -> registers;
    /* return twice and values are different */
    child_tcb -> registers.eax = 0;
    parent_tcb -> registers.eax = child_pcb -> pid;
    list_init(&child_pcb -> threads);
    list_init(&child_pcb -> va);
    list_insert_last(&child_pcb -> threads, &child_tcb->peer_threads_node);
    lprintf("The length is %d",child_pcb->threads.length);
    /* step 4: set up the process control block */
    list_init(&child_pcb -> children);
    child_pcb -> pid = next_pid;
    next_pid++;
    child_pcb -> state = PROCESS_RUNNING;
    child_pcb -> parent = parent_pcb;
    list_insert_last(&parent_pcb -> children, &child_pcb->peer_processes_node);
    parent_pcb -> children_count++;

    /* step 5: create a new page directory for the child */
    child_pcb -> PD = (uint32_t *) memalign(PD_SIZE * 4, PT_SIZE * 4);
    if (child_pcb -> PD == NULL)
    {
        return -1;
    }
    //lprintf("The child tid is %d, pd is %p", child_tcb->tid, child_tcb->pcb->PD);
    int i, j;
    // copy kernel mappings first
    for (i = 0; i < 4; ++i)
    {
        (child_pcb->PD)[i] = parent_directory[i];
    }
    // copy user mappings by allocating new frames for each mapping
    for (i = 4; i < PD_SIZE; ++i)
    {
        //parent directory entry info
        uint32_t parent_de_raw = parent_directory[i];
        uint32_t pt_addr = DEFLAG_ADDR(parent_de_raw);
        if (pt_addr == 0)  continue;
        //child direcotory entry info
        uint32_t child_de = (uint32_t)memalign(PT_SIZE * 4, PT_SIZE * 4);
        if (child_de ==0)
        {
            return -1;
        }
        uint32_t child_de_raw = ADDFLAG(child_de, (GET_FLAG(parent_de_raw)));
        (child_pcb->PD)[i] = child_de_raw;
        //copy page table enties, and copy frame data
        for (j = 0; j < PT_SIZE; ++j)
        {

            //page table entry info
            uint32_t phys_addr_raw = ((uint32_t *)pt_addr) [j];
            uint32_t phys_addr = DEFLAG_ADDR(phys_addr_raw);
            if (phys_addr == 0)  continue;
            if (j == 1 && i == 1023)
            {
                //lprintf("This is speicial case");
                // MAGIC_BREAK;
            }
            uint32_t parent_entry_v_addr = (i << 22) | (j << 12);
            //lprintf("page table entry: %d",j);
            int result = virtual_map_physical(parent_directory, entry.pd_index,
                                 entry.pt_index);
            if (result == -1)
            {
                return -1;
            }
            uint32_t found_table = DEFLAG_ADDR(parent_directory[entry.pd_index]);
            //new allocated frame address with flags;
            uint32_t new_phys_addr = ((uint32_t *)found_table)[entry.pt_index];
            //copy the physical frame using virtual address
            //so that we don't need to turn off paging
            //lprintf("start copying physical addr!");
            //lprintf("free_virtual_addr: %x",(int)entry.free_virtual_addr);
            //lprintf("i:%d",i);
            //lprintf("j:%d",j);
            //lprintf("pd_index: %d",entry.pd_index);
            //lprintf("pt_index: %d",entry.pt_index);
            //lprintf("parent entry virtual addr: %x",(int)parent_entry_v_addr);
            //lprintf("old physical addr:%x",(unsigned int) phys_addr);
            //lprintf("new physical addr:%x",(unsigned int) DEFLAG_ADDR(new_phys_addr));
            memcpy((void *)entry.free_virtual_addr, (void *)parent_entry_v_addr, 4096);
            //demap this new frame from parent pd
            ((uint32_t *)child_de) [j] = ADDFLAG(DEFLAG_ADDR(new_phys_addr), GET_FLAG(phys_addr_raw)) ;
            bzero( &(((uint32_t *)found_table) [entry.pt_index])  , 4);
            // ((uint32_t*)found_table) [entry.pt_index] = 0;

            //and map this new frame to child pd;

            //lprintf("child_de pte:%x",(unsigned int) ((uint32_t*)child_de) [j]);
            //lprintf("hahah: %x", (int)(((uint32_t*)found_table) [entry.pt_index]));
            set_cr3((uint32_t)parent_directory);
        }
    }

    // //lprintf("finished!");

    // insert child to the list of threads and processes
    list_insert_last(&process_queue, &child_pcb->all_processes_node);
    list_insert_last(&runnable_queue, &child_tcb->thread_list_node);
    // list_insert_last(&thread_queue, &parent_tcb->all_threads);

    // lprintf("ready to return! parent pid:%d", parent_pcb -> pid);
    // lprintf("ready to return! child pid:%d", child_pcb -> pid);
    //lprintf("%u",child_tcb->registers.eax);

    return child_pcb -> pid;
}

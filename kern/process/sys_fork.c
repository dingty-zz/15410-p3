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

typedef struct entry_info
{
    int pd_index;   // the page directory index
    int pt_index;   // the page table index
    uint32_t free_virtual_addr; // Calculated freed virtual address
} entry_struct;

static entry_struct entry;

/** @brief Find and fill in the free entry from the parent page directory
 *
 *  @param parent_directory The pointer to page directory
 *  @return int 1 on success and 0 otherwise
 **/
int find_free_entry(uint32_t *parent_directory)
{
    int i, j;
    for (i = 4; i < PD_SIZE; ++i)
    {
        uint32_t current_pde = DEFLAG_ADDR(parent_directory[i]);
        // continue when the current page directory entry is 0
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
            }
        }
    }
    return 0;
}

/** @brief The fork implementation, ZFOD
 *
 *  use ZFOD, by using a temporary unmapped entry to copy the old physical
 *  frame to the new allocated physical frame for child, so as not to 
 *  turn off paging.
 *
 *  @param nothing
 *  @return -1 on failure, 0 to child, child pid to parent
 **/
int sys_fork(void)
{

    /* step 0 : check if multi threaded; if so, then no permission to fork */
    if (current_thread -> pcb -> threads.length != 1)
         return -1;

    /* Step 1: update current threads' registers */
    unsigned int *kernel_stack =
        (unsigned int *)(current_thread -> stack_base +
                         current_thread->stack_size - 60);
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

    /* Step 2: create new task control block */
    PCB *child_pcb = (PCB *)malloc(sizeof(PCB));
    if (child_pcb == NULL)
    {
        lprintf("Sth bad happend");
        return -1;
    }
    TCB *child_tcb = (TCB *)malloc(sizeof(TCB));
    if (child_tcb == NULL)
    {
        lprintf("Sth bad happend");

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
    child_tcb -> start_ticks = 0;
    child_tcb -> duration = 0;
    child_tcb -> stack_size = PAGE_SIZE;
    child_tcb -> stack_base = smemalign(PAGE_SIZE, child_tcb->stack_size);
    if (child_tcb -> stack_base == NULL)
    {
        lprintf("Sth bad happend");

        return -1;
    }
    child_tcb -> esp = (uint32_t)child_tcb -> stack_base +
                       (uint32_t)child_tcb -> stack_size;
    child_tcb -> registers = parent_tcb -> registers;
    /* return twice and values are different */
    child_tcb -> registers.eax = 0;
    parent_tcb -> registers.eax = child_pcb -> pid;

    child_tcb -> swexn_info = parent_tcb -> swexn_info;
    
    list_init(&child_pcb -> threads);
    list_init(&child_pcb -> va);
    list_insert_last(&child_pcb -> threads, &child_tcb->peer_threads_node);

    /* step 4: set up the process control block */
    list_init(&child_pcb -> children);
    child_pcb -> pid = next_pid;
    next_pid++;
    child_pcb -> state = PROCESS_RUNNING;
    child_pcb -> parent = parent_pcb;
    list_insert_last(&parent_pcb -> children, &child_pcb->peer_processes_node);
    parent_pcb -> children_count++;

    /* step 5: create a new page directory for the child */
    child_pcb -> PD = (uint32_t *) smemalign(PAGE_SIZE, PAGE_SIZE);
    if (child_pcb -> PD == NULL)
    {
        lprintf("Sth bad happend");

        return -1;
    }
    memset((void *)child_pcb -> PD, 0, PAGE_SIZE);

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
        uint32_t child_de = (uint32_t)smemalign(PAGE_SIZE, PAGE_SIZE);
        if (child_de ==0)
        {
        lprintf("Sth bad happend");
            
            return -1;
        }
        memset((void *)child_de, 0, PAGE_SIZE);

        uint32_t child_de_raw = ADDFLAG(child_de, (GET_FLAG(parent_de_raw)));
        (child_pcb->PD)[i] = child_de_raw;
        //copy page table enties, and copy frame data
        for (j = 0; j < PT_SIZE; ++j)
        {

            //page table entry info
            uint32_t phys_addr_raw = ((uint32_t *)pt_addr) [j];
            uint32_t phys_addr = DEFLAG_ADDR(phys_addr_raw);
            if (phys_addr == 0)  continue;

            uint32_t parent_entry_v_addr = (i << 22) | (j << 12);
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
           
            memcpy((void *)entry.free_virtual_addr, (void *)parent_entry_v_addr, 4096);
            //demap this new frame from parent pd
            ((uint32_t *)child_de) [j] = ADDFLAG(DEFLAG_ADDR(new_phys_addr), GET_FLAG(phys_addr_raw)) ;
            bzero( &(((uint32_t *)found_table) [entry.pt_index])  , 4);

            //and map this new frame to child pd;
            //flush tlb
            set_cr3((uint32_t)parent_directory);
        }
    }

    // insert child to the list of threads and processes
    list_insert_last(&process_queue, &child_pcb->all_processes_node);
    list_insert_last(&runnable_queue, &child_tcb->thread_list_node);

    return child_pcb -> pid;
}

/** @file scheduler.c
 *
 *  @brief This file the scheduler and context switch routine.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "linked_list.h"
#include "control_block.h"
#include "simics.h"
#include "linked_list.h"
#include "seg.h"
#include "cr.h"
#include "simics.h"
#include "malloc.h"
#include <elf/elf_410.h>
#include "common_kern.h"
#include "string.h"
#include "eflags.h"
// #include "linked_list.c"

#define offsetoff(TYPE, MEMBER) ((size_t) &((TYPE *) 0)->MEMBER)
#define list_entry(LIST_ELEM, STRUCT, MEMBER)    \
    ((STRUCT *) ((uint8_t *) LIST_ELEM    \
                 - offsetoff (STRUCT, MEMBER)))


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

unsigned int seconds;
extern list thread_queue;
TCB *current_thread;  // indicates the current runnign thread
void schedule();


void tick(unsigned int numTicks)
{
    if (numTicks % 100 == 0)
    {
        ++seconds;
        if (seconds % 5 == 0)
        {
            lprintf("5 seconds, let's do something");
            // MAGIC_BREAK;
            schedule();     // schedule
        }

    }

}

void schedule()
{
  disable_interrupts();
    // save current running thread, such as %ss and stuff
    lprintf("this is the current running thread: %d", current_thread->tid);

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

    // MAGIC_BREAK;
    // pop a thread from the thread_queue
    node *n  = list_delete_first(&thread_queue);
    if (n == 0)
    {
        lprintf("ohohoh");
        MAGIC_BREAK;
    }
    TCB *next_thread = list_entry(n, TCB, all_threads);


    // run this thread by setting registers, and restore it's kernel stack

    unsigned int *next_kernel_stack = (unsigned int *)(next_thread -> stack_base + next_thread->stack_size - 52);
    lprintf("the next_kernel_stack is : %p", next_kernel_stack);
    next_kernel_stack[0] = next_thread -> registers.edi;
    next_kernel_stack[1] = next_thread -> registers.esi;
    next_kernel_stack[2] = next_thread -> registers.ebp;
    next_kernel_stack[4] = next_thread -> registers.ebx;
    next_kernel_stack[5] = next_thread -> registers.edx;
    next_kernel_stack[6] = next_thread -> registers.ecx;
    next_kernel_stack[7] = next_thread -> registers.eax;
    next_kernel_stack[8] = next_thread -> registers.eip;
    next_kernel_stack[9] = next_thread -> registers.cs;
    next_kernel_stack[10] = next_thread -> registers.eflags;
    next_kernel_stack[11] = next_thread -> registers.esp;
    next_kernel_stack[12] = next_thread -> registers.ss;

    set_esp0((uint32_t)(next_thread -> stack_base + next_thread -> stack_size));
    list_insert_last(&thread_queue, &current_thread->all_threads);

    // MAGIC_BREAK;
    current_thread = next_thread;
    enter_user_mode(next_thread -> registers.edi,     // let it run, enter ring 3!
           next_thread -> registers.esi,
           next_thread -> registers.ebp,
           next_thread -> registers.ebx,
           next_thread -> registers.edx,
           next_thread -> registers.ecx,
           next_thread -> registers.eax,
           next_thread -> registers.eip,
           next_thread -> registers.cs,
           next_thread -> registers.eflags,
           next_thread -> registers.esp,
           next_thread -> registers.ss);

}
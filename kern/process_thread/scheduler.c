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
#include <x86/asm.h>
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
extern TCB *current_thread;  // indicates the current runnign thread
void schedule();
void context_switch(TCB *current, TCB *next);
extern void do_switch(TCB *current, TCB *next);
void prepare_init_thread(TCB *next);

void tick(unsigned int numTicks)
{
    if (numTicks % 5 == 0)
    {
        ++seconds;
        if (seconds % 5 == 0)
        {
            lprintf("5 seconds, let's do something\n");
            schedule();     // schedule
        }

    }

}

void schedule()
{
    disable_interrupts();

    // pop a thread from the thread_queue
    node *n  = list_delete_first(&thread_queue);
    if (n == 0)
    {
        lprintf("ohohoh");
        MAGIC_BREAK;
    }
    TCB *next_thread = list_entry(n, TCB, all_threads);

    lprintf("getcr3 %x", (unsigned int)get_cr3());

    list_insert_last(&thread_queue, &current_thread->all_threads);

    // MAGIC_BREAK;
    // lprintf("Switch from current: %p, to next: %p\n", current_thread, next_thread);
    context_switch(current_thread, next_thread);
    current_thread = next_thread;
    enable_interrupts();
}

// note that this implementation is NOT OK because you have to set esp to it's appropriate place

void context_switch(TCB *current, TCB *next)
{
    lprintf("Switch from current: %p, to next: %p\n", current, next);

    // MAGIC_BREAK;
    do_switch(current, next);
    // TCB *temp = next;
    // next = current;
    // current = temp;
   lprintf("(^_^)Switch from current: %p, to next: %p\n", current, next);

    set_cr3((uint32_t)next -> pcb -> PD);
    // MAGIC_BREAK;
    
    
    set_esp0((uint32_t)(next -> stack_base + next -> stack_size));

    return;
}

void prepare_init_thread(TCB *next)
{
    lprintf("%p", next);
    lprintf("105, run this thread");
    set_cr3((uint32_t)next -> pcb -> PD);
    set_esp0((uint32_t)(next -> stack_base + next -> stack_size));
    next -> state = THREAD_RUNNING;
    current_thread = next;
    enable_interrupts();
    enter_user_mode(next -> registers.edi,
                    next -> registers.esi,
                    next -> registers.ebp,
                    next -> registers.ebx,
                    next -> registers.edx,
                    next -> registers.ecx,
                    next -> registers.eax,
                    next -> registers.eip,
                    next -> registers.cs,
                    next -> registers.eflags,
                    next -> registers.esp,
                    next -> registers.ss);
}
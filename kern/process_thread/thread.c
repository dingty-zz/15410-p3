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

uint32_t next_tid = 0;

int thr_init() {
	    list_init(&thread_queue);
    next_tid = 1;
    return 0;
}

TCB *thr_create(simple_elf_t *se_hdr, int run) {
	 // set up tcb for this program
    TCB *tcb = (TCB *)malloc(sizeof(TCB));
    tcb -> tid = next_tid;
    next_tid++;
    tcb -> state = THREAD_RUNNING;

    tcb -> stack_size = 4096;
    tcb -> stack_base = smemalign(4, tcb->stack_size);

    tcb -> registers.ds = SEGSEL_USER_DS;
    tcb -> registers.es = SEGSEL_USER_DS;
    tcb -> registers.fs = SEGSEL_USER_DS;
    tcb -> registers.gs = SEGSEL_USER_DS;

    tcb -> registers.edi = 0;
    tcb -> registers.esi = 0;
    tcb -> registers.ebp = 0xffffffff;  // set up frame pointer
    tcb -> registers.ebx = 0;
    tcb -> registers.edx = 0;
    tcb -> registers.ecx = 0;
    tcb -> registers.eax = 0;

    tcb -> registers.eip = se_hdr -> e_entry;
    tcb -> registers.cs = SEGSEL_USER_CS;
    tcb -> registers.eflags = (get_eflags() | EFL_RESV1) & ~EFL_AC;
    tcb -> registers.esp = 0xffffff10;  // set up user stack pointer
    tcb -> registers.ss = SEGSEL_USER_DS;
    lprintf("The kernel stack is : %p", tcb -> stack_base+4096);
    if (!run)  // if not run, we put it in the run queue
        list_insert_last(&thread_queue, &tcb -> all_threads);
    return tcb;

}	

// int thr_join() {

// }

// int thr_exit() {
	
// }
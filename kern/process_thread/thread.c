#include "control_block.h"

int thr_init() {

}

int thr_create() {
// set up tcb for init 
    TCB *tcb = (TCB *)smemalign(4, sizeof(TCB));
    tcb -> tid = next_tid;
    next_tid++;
    tcb -> state = THREAD_RUNNABLE;

    tcb -> registers.ds = SEGSEL_USER_DS | 0x3;
    tcb -> registers.es = SEGSEL_USER_DS | 0x3;
    tcb -> registers.fs = SEGSEL_USER_DS | 0x3;
    tcb -> registers.gs = SEGSEL_USER_DS | 0x3;    

    tcb -> registers.edi = 0;
    tcb -> registers.esi = 0;
    tcb -> registers.ebp = 0xffffffff;
    tcb -> registers.ebx = 0;
    tcb -> registers.edx = 0;
    tcb -> registers.ecx = 0;
    tcb -> registers.eax = 0;

    tcb -> registers.eip = se_hdr.e_entry;
    tcb -> registers.cs = SEGSEL_USER_CS | 0x3;
    tcb -> registers.eflags = (get_eflags() | EFL_RESV1) & ~EFL_AC;
    tcb -> registers.esp = 0xffffffff;
    tcb -> registers.ss = SEGSEL_USER_DS | 0x3;

    list_insert_last(&thread_queue, tcb -> all_threads);
}

int thr_join() {

}

int thr_exit() {
	
}
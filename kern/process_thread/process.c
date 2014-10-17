
/** @file process.c
 *  @brief This file defines funcitons to control processes
 */





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
list thread_queue;
list process_queue;
uint32_t next_pid = 0;
uint32_t next_tid =0 ;

void allocate_page(uint32_t virtual_addr, size_t size);
extern void set_ss();


int process_init() {


// create a list of run queues based on threads
	list_init(&thread_queue);
	list_init(&process_queue);
	next_tid = 1;
	next_pid = 1;
	return 0;
}



int process_create(const char* filename) {

	/* Load the elf program using the helper function */
	simple_elf_t se_hdr;
	lprintf("\n");
    elf_load_helper(&se_hdr, filename);
    lprintf("%lx",se_hdr.e_entry);
    lprintf("%d",machine_phys_frames());
       lprintf("e_txtstart: %lx",se_hdr.e_txtstart);
    lprintf("e_txtoff: %lu",se_hdr.e_txtoff);
    lprintf("e_txtlen: %lu",se_hdr.e_txtlen);


        lprintf("e_datstart: %lx",se_hdr.e_datstart);
    lprintf("e_datoff: %lu",se_hdr.e_datoff);
    lprintf("e_datlen: %lu",se_hdr.e_datlen);


        lprintf("e_rodatstart: %lx",se_hdr.e_rodatstart);
    lprintf("e_rodatoff: %lu",se_hdr.e_rodatoff);
    lprintf("e_rodatlen: %lu",se_hdr.e_rodatlen);


        lprintf("e_bssstart: %lx",se_hdr.e_bssstart);
    lprintf("e_bsslen: %lu",se_hdr.e_bsslen);


    /* Allocate memory for every area */
    allocate_page((uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
	allocate_page((uint32_t)se_hdr.e_txtstart,se_hdr.e_txtlen);
	allocate_page((uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
	allocate_page((uint32_t)se_hdr.e_bssstart,se_hdr.e_bsslen);
	allocate_page((uint32_t)0xffffefff,4096);
 // *(int *)0xfffffffc=3;


	/* copy data from data field */
	getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen, (char *)se_hdr.e_datstart);
	getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen, (char *)se_hdr.e_txtstart);
	getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen, (char *)se_hdr.e_rodatstart);
	memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);

	// set up tcb for this program
    TCB *tcb = (TCB *)malloc(sizeof(TCB));
    tcb -> tid = next_tid;
    next_tid++;
    tcb -> state = THREAD_RUNNING;

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

    tcb -> registers.eip = se_hdr.e_entry;
    tcb -> registers.cs = SEGSEL_USER_CS;
    tcb -> registers.eflags = (get_eflags() | EFL_RESV1) & ~EFL_AC;
    tcb -> registers.esp = 0xffffffff;  // set up stack pointer
    tcb -> registers.ss = SEGSEL_USER_DS;

    list_insert_last(&thread_queue, &tcb -> all_threads);



	// set up pcb for this program

    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb -> state = PROCESS_RUNNING;
    pcb -> ppid = 0; // who cares this??
    pcb -> pid = next_pid;
    next_pid++;
    // list_init(pcb -> threads);
    pcb -> thread = tcb;

    // pcb -> PD = memcpy(asdfasdf,fsdaf);
    // pcb -> PT = memcpy('sfasdfas'f);
    list_insert_last(&process_queue, &pcb -> all_processes);


    tcb -> pcb = pcb;

    MAGIC_BREAK;
    set_esp0(get_esp0());
    set_ss();  // let it run, enter ring 3!
    return 0;	
}


int process_exit() {
return 0;

}







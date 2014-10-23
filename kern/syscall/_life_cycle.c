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

extern list process_queue;
extern uint32_t next_pid;

void allocate_page(uint32_t virtual_addr, size_t size);
extern void set_ss(uint32_t ss, 
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
extern TCB *thr_create(simple_elf_t *se_hdr, int run);

extern TCB *current_thread;


/* two more things to do: 1. copy page table 2. iret*/
int _fork(void)
{
  // //parent_pcb
  // //COW
  // PCB* child_pcb = (PCB*)malloc(sizeof(PCB));
  // //1. ecopy the parent's page tables
  // pgt* parent_pgt = parent_pcb -> page_table;
  // pgt* child_pgt = copy_pgt(parent_pgt);
  // child_pcb->page_table = child_pgt;
  // //2. the assignment of a unique process 
  // //descriptor struct, task_struct, for the child. 
  // int curTid = gettid();

  // //3. put both processes in the scheduling waiting queue


  //---------------------------------------------------------------
  PCB* child_pcb = (PCB*)malloc(sizeof(PCB));
  TCB* child_tcb = (TCB*)malloc(sizeof(TCB));
  PCB* parent_pcb = current_thread -> pcb;
  //step 1: check if multi threaded; then no permission to fork;
  //to be done. We should add count of threads in pcb
  //.........

  //step 2: set up the thread control block;
  child_tcb -> pcb = child_pcb;
  child_tcb -> tid = next_tid;
  next_tid++;
  child_tcb -> state = THREAD_RUNNING;
  child_tcb -> registers = parent_tcb -> registers;
  child_tcb -> all_threads = {NULL,NULL};

  //step 3: set up the process control block;
  child_pcb -> special = 0;
  child_pcb -> ppid = parent_pcb -> ppid;
  child_pcb -> pid = next_pid;
  next_pid++;
  child_pcb -> state = PROCESS_RUNNING;
  child_pcb -> thread = child_tcb;

  //return values are different;
  child_tcb -> registers.eax = 0;
  current_thread -> registers.eax = child_pcb -> pid;

  //copy page tables
  
  //insert child to the list of threads and processes
  list_insert_last(process_queue,child_pcb);
  list_insert_last(thread_queue,child_tcb);
  list_insert_last(thread_queue,parent_tcb);

  return 0;
}


int _exec(char *execname, char *argvec[])
{
  lprintf("char %s, argvec: %p", execname, argvec);
	int argc = 0;
	while(argvec[argc] != 0) {
		argc++;
	}
    lprintf("The argc==%d",argc);
	/* Load the elf program using the helper function */
    simple_elf_t se_hdr;
    lprintf("\n");
    elf_load_helper(&se_hdr, execname);
    lprintf("%lx", se_hdr.e_entry);
    lprintf("e_txtstart: %lx", se_hdr.e_txtstart);
    lprintf("e_txtoff: %lu", se_hdr.e_txtoff);
    lprintf("e_txtlen: %lu", se_hdr.e_txtlen);


    lprintf("e_datstart: %lx", se_hdr.e_datstart);
    lprintf("e_datoff: %lu", se_hdr.e_datoff);
    lprintf("e_datlen: %lu", se_hdr.e_datlen);


    lprintf("e_rodatstart: %lx", se_hdr.e_rodatstart);
    lprintf("e_rodatoff: %lu", se_hdr.e_rodatoff);
    lprintf("e_rodatlen: %lu", se_hdr.e_rodatlen);


    lprintf("e_bssstart: %lx", se_hdr.e_bssstart);
    lprintf("e_bsslen: %lu", se_hdr.e_bsslen);

   lprintf("before zeroth break");
    // *(int *)0xffffffff=3;

    MAGIC_BREAK;
    /* Allocate memory for every area */
    allocate_page((uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
    // MAGIC_BREAK;
    allocate_page((uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    allocate_page((uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    allocate_page((uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
    allocate_page((uint32_t)0xffffc000, 4096*4);  // possibly bugs here
   lprintf("before first break");
    // *(int *)0xffffffff=3;

    MAGIC_BREAK;
    // /* copy data from data field */
    getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen, (char *)se_hdr.e_datstart);
    getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen, (char *)se_hdr.e_txtstart);
    getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen, (char *)se_hdr.e_rodatstart);
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);

   


    // set up pcb for this program

    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb -> state = PROCESS_RUNNING;
    pcb -> ppid = 0; // who cares this??
    pcb -> pid = next_pid;
    next_pid++;
    // list_init(pcb -> threads);

    TCB *thread = thr_create(&se_hdr, 1); // please see thread.c
    pcb -> thread =  thread;

    // pcb -> PD = memcpy(asdfasdf,fsdaf);
    // pcb -> PT = memcpy('sfasdfas'f);
    list_insert_last(&process_queue, &pcb -> all_processes);


    thread -> pcb = pcb;  // cycle reference :)
    
    MAGIC_BREAK;
	*((unsigned int*)(thread -> registers.esp)) = 0xffffc000;
	thread -> registers.esp -= 4;

	*(unsigned int*)thread -> registers.esp = 0xffffffff;
	thread -> registers.esp -= 4;


	*((unsigned int*)(thread -> registers.esp)) = (unsigned int)argvec;
	thread -> registers.esp -= 4;

        *(int*)thread -> registers.esp = argc;
    thread -> registers.esp -= 4;


    MAGIC_BREAK;

   lprintf("before second break");

MAGIC_BREAK;
    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));  // set up kernel stack pointer possibly bugs here
    lprintf("this is the esp, %x", (unsigned int)get_esp0());

    
    set_ss(thread -> registers.edi,     // let it run, enter ring 3!
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

void set_status(int status)
{
	return;
}

volatile int placate_the_compiler;
void vanish(void)
{
	int blackhole = 867-5309;

	blackhole ^= blackhole;
	blackhole /= blackhole;
	*(int *) blackhole = blackhole; /* won't get here */
	while (1)
		++placate_the_compiler;
}

int wait(int *status_ptr)
{
	return -1;
}


void task_vanish(int status)
{
	status ^= status;
	status /= status;
	while (1)
		continue;
}

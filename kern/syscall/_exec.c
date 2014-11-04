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

extern list process_queue;
extern uint32_t next_pid;
extern int process_create(const char *filename, int run);
void allocate_pages(uint32_t *pd, uint32_t virtual_addr, size_t size);

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
extern TCB *thr_create(simple_elf_t *se_hdr, int run);
extern uint32_t *init_pd();
extern TCB *current_thread;

/* Two things to do:
1. add return -1, exec_non_exist
2. don't increase tid
*/
int _exec(char *execname, char *argvec[])
{
    char *name = (char *)malloc(strlen(execname));
    memcpy(name, execname, strlen(execname));
    lprintf("The execname is %s", name);
    lprintf("char %s, argvec: %p", execname, argvec);
    int argc = 0;
    while (argvec[argc] != 0)
    {

        argc++;
    }
    char *argv[argc];
    int j = 0;
    for (j = 0; j < argc; ++j)
    {
        argv[j] = (char *)malloc(strlen(argvec[j]));
        strcpy(argv[j], argvec[j]);
    }
    int l = 0;
    for (l = 0; l < argc; ++l)
    {
        lprintf("The is %s\n", argv[l]);
    }


    lprintf("The argc==%d", argc);
    simple_elf_t se_hdr;

    // elf_load_helper(&se_hdr, execname);
    // lprintf("%lx", se_hdr.e_entry);
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    //create a clean page directory
    pcb -> PD = init_pd();
    // set up pcb for this program
    elf_load_helper(&se_hdr, name);
    lprintf("dsfsdf");
    allocate_pages(pcb -> PD,
                   (uint32_t)se_hdr.e_txtstart, se_hdr.e_txtlen);
    getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen,
             (char *)se_hdr.e_txtstart);
    lprintf("Doneonoenoennoneoeneonoe");
    MAGIC_BREAK;
    pcb -> state = PROCESS_RUNNING;
    pcb -> ppid = 0; // who cares this??
    pcb -> pid = next_pid;
    next_pid++;


    // list_init(pcb -> threads);
    TCB *thread = thr_create(&se_hdr, 1); // please see thread.c
    pcb -> thread =  thread;


    list_insert_last(&process_queue, &pcb -> all_processes);


    thread -> pcb = pcb;  // cycle reference :)


    if (0)  // if not run ,we return
    {
        MAGIC_BREAK;

        return 0 ;
    }
    MAGIC_BREAK;
    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    // set up kernel stack pointer possibly bugs here
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));
    lprintf("this is the esp, %x", (unsigned int)get_esp0());
    /* Load the elf program using the helper function */

    // lprintf("\n");

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

    //MAGIC_BREAK;

    /* Allocate memory for every area */
    allocate_pages(pcb -> PD,
                   (uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
    // MAGIC_BREAK;

    allocate_pages(pcb -> PD,
                   (uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
    allocate_pages(pcb -> PD,
                   (uint32_t)se_hdr.e_bssstart, se_hdr.e_bsslen);
    allocate_pages(pcb -> PD,
                   (uint32_t)0xfffff000, 4096); // possibly bugs here

    lprintf("allocate_pages done!");
    // *(int *)0xffffffff=3;

    // MAGIC_BREAK;
    // /* copy data from data field */
    getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen,
             (char *)se_hdr.e_datstart);

    getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen,
             (char *)se_hdr.e_rodatstart);
    memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);


    char *dest = (char *)0xffffffff;
    char *vector[argc + 1];

    int k = 0;
    for (k = 0; k < argc; k++)
    {
        dest -= (strlen(argv[k]) + 1);
        strcpy(dest, argv[k]);
        vector[k] = dest;
    }
    vector[argc] = NULL;
    for (k = 0; k <= argc; k++) lprintf("The vector :%p", vector[k]);
    dest -= sizeof(int) * 5;
    memcpy(dest, vector, sizeof(int) * 5);
    // for (k =0;k<=argc;k++) lprintf("The vector :%s",dest[k]);
    MAGIC_BREAK;

    *((unsigned int *)(thread -> registers.esp)) = 0xffffc000;
    thread -> registers.esp -= 4;

    *(unsigned int *)thread -> registers.esp = 0xffffffff;
    thread -> registers.esp -= 4;


    *((unsigned int *)(thread -> registers.esp)) = (unsigned int)dest;
    thread -> registers.esp -= 4;
    lprintf("The argv %s", ((char **)dest)[0]);
    lprintf("The argc %s", ((char **)dest)[1]);
    lprintf("The argc %s", ((char **)dest)[2]);
    lprintf("The argc %s", ((char **)dest)[3]);
    *(int *)thread -> registers.esp = argc;

    thread -> registers.esp -= 4;


    MAGIC_BREAK;


    lprintf("before second break");

    MAGIC_BREAK;

    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));  // set up kernel stack pointer possibly bugs here
    lprintf("this is the esp, %x", (unsigned int)get_esp0());

    enter_user_mode(thread -> registers.edi,     // let it run, enter ring 3!
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

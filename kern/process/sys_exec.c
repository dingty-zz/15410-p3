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
#include "enter_user_mode.h"
#include "process.h"
#include "vm_routines.h"
#include "thread.h"


extern list process_queue;
extern uint32_t next_pid;

extern TCB *current_thread;

/* Two things to do:
1. add return -1, exec_non_exist
2. don't increase tid
*/
int sys_exec(char *execname, char *argvec[])
{
    simple_elf_t se_hdr;
    int result = elf_load_helper(&se_hdr, filename);

    if (result == NOT_PRESENT || result == ELF_NOTELF)
    {
        // error, the file doesn't exist
        return -1;
    }
    char *name = (char *)malloc(strlen(execname));
    memcpy(name, execname, strlen(execname));
    lprintf("The execname is %s", name);
    lprintf("char %s, argvec: %p", execname, argvec);

    // Count the number of arguments
    int argc = 0;
    while (argvec[argc] != 0)
    {
        argc++;
    }

    // Copy the content to the kernel stack
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

    // set up process for this program
    process -> special = 0;
    process -> state = PROCESS_RUNNABLE;   // currently unused
    process -> pid = next_pid;
    next_pid++;
    process -> return_state = 0;
    process -> parent = NULL;

    list_init(process -> threads);
    list_init(process -> children);

    list_insert_last(&process_queue, &process -> all_processes_node);

 // Load the program, copy the content to the memory and get the eip
    unsigned int eip = program_loader(se_hdr, process);

    // Create a single thread for this process
    TCB *thread = thr_create(eip, run); // please see thread.c
    list_insert_last(&process -> thread, &thread -> peer_threads_node);

    thread -> pcb = process;  // cycle reference :)

    /* We need to do this everytime for a thread to run */
    current_thread = thread;
    // set up kernel stack pointer possibly bugs here
    set_esp0((uint32_t)(thread -> stack_base + thread -> stack_size));

    // Copy the content to the new user stack
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

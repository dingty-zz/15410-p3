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
#include "enter_user_mode.h"
#include "process.h"
#include "memory/vm_routines.h"
#include "thread/thread_basic.h"


extern list process_queue;
extern uint32_t next_pid;

extern TCB *current_thread;

/* Two things to do:
1. add return -1, exec_non_exist
2. don't increase tid
*/
int sys_exec(char *execname, char *argvec[])
{
    char *name = (char *)malloc(strlen(execname) + 1);
    strncpy(name, execname,strlen(execname)+1);
    // name = "swexn_uninstall_test";
    lprintf("The execname is %s", name);
    lprintf("char %s, argvec: %p", execname, argvec);
    simple_elf_t se_hdr;
    int result = elf_load_helper(&se_hdr, name);
    lprintf("after elf loader");
    if (result == NOT_PRESENT || result == ELF_NOTELF)
    {
        lprintf("program not present in exec");
        // error, the file doesn't exist
        return -1;
    }

    lprintf("in exec, program found, start to load");
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

    PCB *process = current_thread -> pcb;

    // Unmap current page directory and free all its address space
    process -> PD = init_pd();

    current_thread -> registers.eip = program_loader(se_hdr, process);
    // set up kernel stack pointer possibly bugs here
    set_esp0((uint32_t)(current_thread -> stack_base + current_thread -> stack_size));

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

    dest -= sizeof(int) * 5;
    memcpy(dest, vector, sizeof(int) * 5);
    *((unsigned int *)(current_thread -> registers.esp)) = 0xffffc000;
    current_thread -> registers.esp -= 4;

    *(unsigned int *)current_thread -> registers.esp = 0xffffffff;
    current_thread -> registers.esp -= 4;


    *((unsigned int *)(current_thread -> registers.esp)) = (unsigned int)dest;
    current_thread -> registers.esp -= 4;

    *(int *)current_thread -> registers.esp = argc;

    current_thread -> registers.esp -= 4;


    enter_user_mode(current_thread -> registers.edi,     // let it run, enter ring 3!
                    current_thread -> registers.esi,
                    current_thread -> registers.ebp,
                    current_thread -> registers.ebx,
                    current_thread -> registers.edx,
                    current_thread -> registers.ecx,
                    current_thread -> registers.eax,
                    current_thread -> registers.eip,
                    current_thread -> registers.cs,
                    current_thread -> registers.eflags,
                    current_thread -> registers.esp,
                    current_thread -> registers.ss);
    return 0;
}

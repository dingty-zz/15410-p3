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
/** @brief Determine if the given queue is empty
 *
 *  If top == bottom, we know there are nothing in the queue.
 *
 *  @param q The pointer to the queue
 *  @return int 1 means not empty and 0 otherwise
 **/
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

    // Count the length of total number of char to be copied
    int total_len = 0;
    int i = 0;
    for (i = 0; i < argc; ++i)
    {
        total_len += (strlen(argvec[i]) + 1);
    }

    // Copy the content to the kernel stack
    char string[total_len+1];
    char *kernel_dest = (char *)string;
    char *argv[argc + 1];

    int k = 0;
    for (k = 0; k < argc; k++)
    {
        argv[k] = kernel_dest;
        strcpy(kernel_dest, argvec[k]);
        kernel_dest += (strlen(argvec[k]) + 1);
    }
    argv[argc] = NULL;

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

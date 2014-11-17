 /**
 * @file sys_exec.h
 *
 * @brief This file contains the exec system call implementation
          it first verifies the passed in pointers, if all valid,
          it copies execname and argvec on to the current kernel
          stack, and remap the virtual memory by changing the page
          directory. After loading all the elf sections to the user
          memory, it the copies the content stored in the kernel
          stack to the user stack. It also pushes the stack high
          pointer and stack low pointer as well
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */
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
#include <x86/asm.h>                /* enable_interrupts() */

#define USER_STACK_HIGH 0xffffffff
#define USER_STACK_LOW 0xffffc000
#define PARAMETERS_COUNT 5
extern list process_queue;
extern uint32_t next_pid;

extern TCB *current_thread;


/** @brief Execute a process on top of the calling process
 *
 *  @param execname the new process name
 *  @param argvec argument-string vector
 *  @return int -1 when failed, otherwise never returns
 **/
int sys_exec(char *execname, char *argvec[])
{
    if (!is_user_addr(execname) || !addr_has_mapping(execname)) return -1;
    if (!is_user_addr(argvec) || !addr_has_mapping(argvec)) return -1;
    // Allocate the name pointer to store execname onto kernel stack
    char *name = (char *)malloc(strlen(execname) + 1);
    strncpy(name, execname,strlen(execname)+1);

    // Load the process
    simple_elf_t se_hdr;
    int result = elf_load_helper(&se_hdr, name);
    // If invalid program, we return -1
    if (result == NOT_PRESENT || result == ELF_NOTELF)
    {
        // error, the file doesn't exist
        return -1;
    }

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

    // Get current process pcb
    PCB *process = current_thread -> pcb;

    // Unmap current page directory and free all its address space
    process -> PD = init_pd();

    disable_interrupts();
    current_thread -> registers.eip = program_loader(se_hdr, process);
    enable_interrupts();
    
    // set up kernel stack pointer 
    set_esp0((uint32_t)(current_thread -> stack_base + current_thread -> stack_size));

    // Copy the content to the new user stack
    char *dest = (char *)USER_STACK_HIGH;
    char *vector[argc + 1];

    for (k = 0; k < argc; k++)
    {
        dest -= (strlen(argv[k]) + 1);
        strcpy(dest, argv[k]);
        vector[k] = dest;
    }
    vector[argc] = NULL;

    dest -= sizeof(int) * PARAMETERS_COUNT;
    memcpy(dest, vector, sizeof(int) * PARAMETERS_COUNT);
    *((unsigned int *)(current_thread -> registers.esp)) = USER_STACK_LOW;
    current_thread -> registers.esp -= sizeof(int);

    *(unsigned int *)current_thread -> registers.esp = USER_STACK_HIGH;
    current_thread -> registers.esp -= sizeof(int);


    *((unsigned int *)(current_thread -> registers.esp)) = (unsigned int)dest;
    current_thread -> registers.esp -= sizeof(int);

    *(int *)current_thread -> registers.esp = argc;

    current_thread -> registers.esp -= sizeof(int);

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

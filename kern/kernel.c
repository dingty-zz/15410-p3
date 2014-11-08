/** @file process.c
 *
 *  @brief This file defines process manipulation routines
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <string.h>
#include "cr.h"
#include "seg.h"
#include <common_kern.h>
#include "console.h"
#include <malloc.h>
#include "eflags.h"
/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <elf/elf_410.h>
/* multiboot header file */
#include <multiboot.h>              /* boot_info */
#include "mem_internals.h"
/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */

/* Include all related header files in kern/ */
#include "handler_install.h"
#include "memory/vm_routines.h"
#include "process/process.h"
#include "thread/thread_basic.h"

// In scheduler.c
extern void tick(unsigned int numTicks);

extern int malloc_init();

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{

    // Init all subsystems
    malloc_init();
    handler_install(tick);
    clear_console();
    sys_set_term_color(FGND_GREEN | BGND_BLACK);
    mm_init();
    process_init();
    thr_init();
    enable_interrupts();

    lprintf("Hello from a brand new kernel!");    
    // process_create("ck1", 1);   // we hang this thread

    // process_create("merchant", 0);   // we run this thread
    // process_create("peon", 1);


    process_create("idle", 0);   // we run this thread
    // process_create("getpid_test1", 1);   // we run this thread
    // process_create("coolness", 1);   // we run this thread
    process_create("readline_basic", 1);   // we run this thread
    // process_create("deschedule_hang", 1);   // we run this thread


    // process_create("init");
    // process_create("ck1");




    // For future use
    // process_create("idle", 0);   // we hang this thread

    // process_create("init", 1);   // we run this thread

    while (1) continue;

    return 0;
}
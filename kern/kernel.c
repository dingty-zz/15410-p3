/** @file kernel.c
 *
 *  @brief This file initialize the kernel by following the steps specified
 *         in the handout
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
 * @param
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{

    // Init malloc locks
    malloc_init();

    // Install all exception/system call handlers
    handler_install(tick);

    // Initializing console
    clear_console();
    sys_set_term_color(FGND_GREEN | BGND_BLACK);
    show_cursor();

    // Initialize virtual memory system and enable paging
    mm_init();

    // Initialize process management system
    process_init();

    // Initialize thread management system
    thr_init();

    enable_interrupts();

    lprintf("Hello from a brand new kernel!");
    process_create("idle", 0);   // we hang idle

    // Load and run init process
    process_create("init",1);

    // For future use

    while (1) continue;

    return 0;
}
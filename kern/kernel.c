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





    // process_create("ck1", 1);   // we hang this thread

    // process_create("merchant", 0);   // we run this thread
    // process_create("peon", 1);


    // process_create("idle", 0);   // we hang idle
    // process_create("getpid_test1", 1);   // we run this thread
    // process_create("coolness", 1);   // we run this thread
    // process_create("remove_pages_test2", 1);   // we run this thread

    // process_create("swexn_basic_test",1);
    // process_create("swexn_cookie_monster",1);
    // process_create("swexn_dispatch",0);
    // process_create("swexn_regs",0);
    // process_create("swexn_uninstall_test",1);


    // process_create("yield_desc_mkrun", 1);
    // process_create("slaughter", 1);
    // process_create("actual_wait", 1);
    // process_create("make_crash", 1);    

    // process_create("getpid_test1", 1);   // we run this thread
    // process_create("sleep_test1", 1);   // we run this thread
    // process_create("new_pages", 1);   // we run this thread
    // process_create("deschedule_hang", 1);   // we run this thread
    // process_create("remove_pages_test1",1);

    // process_create("minclone_mem",1);

    // process_create("init",1);
    // process_create("ck1");

    // process_create("cho2",1);


    // For future use


    process_create("init", 1);   // we run this thread

    while (1) continue;

    return 0;
}
/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */
#include "cr.h"
#include <common_kern.h>
#include "console.h"
/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <elf/elf_410.h>
/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */
extern int handler_install(void (*tickback)(unsigned int));
static void tick(unsigned int numTicks);
uint64_t seconds;
/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    /*
     * When kernel_main() begins, interrupts are DISABLED.
     * You should delete this comment, and enable them --
     * when you are ready.
     */
handler_install(tick);
    lprintf( "Hello from a brand new kernel! %lu",get_esp0());
    
    clear_console();
    simple_elf_t se_hdr;
    elf_load_helper(&se_hdr, "init");
    lprintf("%lu",se_hdr.e_entry);
   // void *memspace = smemalign(4, 4096);


    while (1) {
        continue;
    }

    return 0;
}
void tick(unsigned int numTicks)
{
     if (numTicks % 100 == 0) 
         ++seconds;
}

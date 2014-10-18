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
extern int handler_install(void (*tickback)(unsigned int));
extern void set_ss();
extern void set_ds(int i);
extern void set_es(int i);
extern void set_cs(int i);
extern void set_esp(int i);
extern KF *mm_init();
void allocate_page(uint32_t virtual_addr, size_t size);
extern void tick(unsigned int numTicks);


// extern list thread_queue;
// extern list process_queue;
static KF *frame_base = 0;

extern int process_create(const char *filename);
extern int process_init();
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
    lprintf("Hello from a brand new kernel! %lu", get_esp0());
    enable_interrupts();
    clear_console();

    frame_base = mm_init();
    process_init();


    //     /* --- Simplified ELF header --- */
    // typedef struct simple_elf {
    //   const char *  e_fname;       /* filename of binary */
    //   unsigned long e_entry;       /* entry point virtual address */
    //   unsigned long e_txtoff;      /* offset of text segment in file */
    //   unsigned long e_txtlen;      /* length of text segment in bytes */
    //   unsigned long e_txtstart;    /* start of text segment virtual address */
    //   unsigned long e_datoff;      /* offset of data segment in file */
    //   unsigned long e_datlen;      /* length of data segment in bytes */
    //   unsigned long e_datstart;    /* start of data segment in virtual memory */
    //   unsigned long e_rodatoff;    /* offset of rodata segment in file */
    //   unsigned long e_rodatlen;    /* length of rodata segment in bytes */
    //   unsigned long e_rodatstart;  /* start of rodata segment in virtual memory*/
    //   unsigned long e_bsslen;      /* length of bss  segment in bytes */
    //   unsigned long e_bssstart;    /* start of bss  segment in virtual memory */
    // } simple_elf_t;




    // MAGIC_BREAK;

    lprintf("The frame_base is : %p", frame_base);




    // set_eflags((get_eflags() | EFL_RESV1) & ~EFL_AC);




    // process_create("init");
    process_create("idle");










    lprintf("End of the kernel main in line 108.");

    while (1)
    {
        continue;
    }

    return 0;
}


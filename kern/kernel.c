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
extern KF* mm_init();
void allocate_page(uint32_t virtual_addr, size_t size);
static void tick(unsigned int numTicks);

uint64_t seconds;

static KF *frame_base = 0;
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
    
    //clear_console();
    simple_elf_t se_hdr;
    elf_load_helper(&se_hdr, "init");
    lprintf("%lx",se_hdr.e_entry);
    lprintf("%d",machine_phys_frames());
    // MAGIC_BREAK;
    //set_ss(213123);

    // MAGIC_BREAK;
    frame_base = mm_init();


    // f = (void *)(se_hdr.e_entry);
    // lprintf("%p",f);
    // f();
    //int i =0;
    // for (i=0;i<1000;i++) lprintf("%adsf", *((int *)se_hdr.e_entry+(1<<24)+i));
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

    lprintf("e_txtstart: %lx",se_hdr.e_txtstart);
    lprintf("e_txtoff: %lu",se_hdr.e_txtoff);
    lprintf("e_txtlen: %lu",se_hdr.e_txtlen);


        lprintf("e_datstart: %lx",se_hdr.e_datstart);
    lprintf("e_datoff: %lu",se_hdr.e_datoff);
    lprintf("e_datlen: %lu",se_hdr.e_datlen);


        lprintf("e_rodatstart: %lx",se_hdr.e_rodatstart);
    lprintf("e_rodatoff: %lu",se_hdr.e_rodatoff);
    lprintf("e_rodatlen: %lu",se_hdr.e_rodatlen);


        lprintf("e_bssstart: %lx",se_hdr.e_bssstart);
    lprintf("e_bsslen: %lu",se_hdr.e_bsslen);


// MAGIC_BREAK;
lprintf("%p", frame_base);
allocate_page((uint32_t)se_hdr.e_datstart, se_hdr.e_datlen);
allocate_page((uint32_t)se_hdr.e_txtstart,se_hdr.e_txtlen);
allocate_page((uint32_t)se_hdr.e_rodatstart, se_hdr.e_rodatlen);
allocate_page((uint32_t)se_hdr.e_bssstart,se_hdr.e_bsslen);
allocate_page((uint32_t)0xffffefff,4096);
 // *(int *)0xfffffffc=3;
/* copy data from data field */
getbytes(se_hdr.e_fname, se_hdr.e_datoff, se_hdr.e_datlen, (char *)se_hdr.e_datstart);
getbytes(se_hdr.e_fname, se_hdr.e_txtoff, se_hdr.e_txtlen, (char *)se_hdr.e_txtstart);
getbytes(se_hdr.e_fname, se_hdr.e_rodatoff, se_hdr.e_rodatlen, (char *)se_hdr.e_rodatstart);
memset((char *)se_hdr.e_bssstart, 0,  se_hdr.e_bsslen);

MAGIC_BREAK;
// set_eflags((get_eflags() | EFL_RESV1) & ~EFL_AC);
set_ss();


lprintf("nop");

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

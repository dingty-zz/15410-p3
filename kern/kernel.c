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
/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <elf/elf_410.h>
/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */
extern int handler_install(void (*tickback)(unsigned int));
extern void set_ss(int i);
extern void set_ds(int i);
extern void set_es(int i);
extern void set_cs(int i);
extern void set_esp(int i);
static void tick(unsigned int numTicks);
void (*f)(void);
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
    lprintf("%lx",se_hdr.e_entry);
    MAGIC_BREAK;
    set_ss(213123);

    MAGIC_BREAK;



    f = (void *)(se_hdr.e_entry);
    lprintf("%p",f);
    f();
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

   void *memspace = smemalign(4, 4096);

   memcpy(memspace, (void *)se_hdr.e_txtstart, se_hdr.e_txtlen);
   //lprintf("%d",i);
   // int i=0;
   // for (i =0; i < se_hdr.e_txtlen; ++i)
   // {
   //     lprintf("%p", memspace+i);
   //     lprintf("itis %s", (char *)(memspace+i));
   // }
   lprintf("%s",(char *)memspace);
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

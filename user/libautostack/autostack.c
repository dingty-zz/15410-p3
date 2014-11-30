 /**
 * @file autostack.c
 *
 * @brief Autostack function and exception handler function
 *        Please refer to README.dox part I for stack design description
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 * @bugs No known bugs
 */
#include <syscall.h>
#include "autostack.h"
#include "thread.h"
#include "simics.h"
#define NULL 0
#define WORDSIZE 4
/*First address of the exception stack*/
#define EXCEP_STACK 0x1EFFFFC
#define PAGE_ALIGN_TEST ((unsigned int)(PAGE_SIZE-1))
#define PAGE_ALIGN_MASK (~PAGE_ALIGN_TEST)
#define LAST_ADDR (EXCEP_STACK + PAGE_SIZE)
extern int getesp();


/** @brief The actual handler. Kills the thread for the 
 *         multi-threaded program. Allocates more space on to the 
 *         stack for the single threaded program.
 *  @param arg The argument to be passed in
 *  @param ureg The saved registers structure
 */
void myhandler(void *arg, ureg_t *ureg)
{
    lprintf("Inside my handler");
    int ret;
    unsigned int cr2 = ureg->cr2;

    
    /* page fault in multi-threaded, exit all the threads in the task 
     * and the task itself */
    if (!global_stackinfo.is_single_threaded) 
        task_vanish(-1);

    /*not a page fault, exit thread*/
    if ((ureg == NULL) || 
        (ureg->cause != SWEXN_CAUSE_PAGEFAULT) || 
        ((ureg->error_code) & 1) !=0) {
        set_status(-2);
        vanish();
    }
        
    
    /*no more memory to allocate on the stack, exit thread*/
    if (cr2 < LAST_ADDR) {
        set_status(-2);
        vanish();
    }

    /* exception happened not because of automatic stack growth */
    if (cr2 < ureg->esp)
    {
        set_status(-2);
        vanish();
    }

    /*align and then new_pages*/
    unsigned int aligned_addr = cr2 & PAGE_ALIGN_MASK;
    if ((ret = new_pages((void*)aligned_addr,PAGE_SIZE)) <0 ) {
        thr_exit((void *)-1);
    }
        
    //re-install the handler;
    swexn((void*)EXCEP_STACK+WORDSIZE, myhandler, NULL, ureg);
    return;
}


/** @brief Install the exception handler for page fault.
 *         First assume that the program is running in single threaded
 *         program by doing global_stackinfo.is_single_threaded = 1. 
 *         Calling thr_create will set the flag to 0 because the program
 *         must be running in the multithreaded mode.
 *  @param stack_high The highest virtual address
 *  @param stack_low The lowest virtual address
 */
void install_autostack(void *stack_high, void *stack_low)
{
    /*check stack_low is page aligned*/
    if (((unsigned int)stack_low & PAGE_ALIGN_TEST) != 0) {
        thr_exit((void*)-1);
    }
    int ret;
    global_stackinfo.global_root_tid = thr_getid(); 
    global_stackinfo.is_single_threaded = 1;
    if ( (unsigned int)stack_low <= PAGE_SIZE ||
         ((unsigned int)stack_low & (PAGE_SIZE-1)) != 0) {
        thr_exit((void *)-1);
    }
    //allocate the space for root exception handler
    if (new_pages((void*)EXCEP_STACK+WORDSIZE-PAGE_SIZE,PAGE_SIZE) < 0) {
        thr_exit((void *)-1);
    }   
    ret = swexn((void*)EXCEP_STACK+WORDSIZE, myhandler, NULL, NULL);
    // unsuccessful installation of handler
    if (ret<0) {
        thr_exit((void *)-1);
        return;
    }
}
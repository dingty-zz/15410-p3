/** @file exception_handler_real.c
 *
 *  @brief This file defines the function to 
 *         call the user defined exception handler
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */


#include "control_block.h"
#include "cr.h"
#include <stddef.h>
#include <syscall.h>
#include <simics.h>
#include <string.h>
#include <stdio.h>
#include "process/enter_user_mode.h"
#include <malloc.h>
#include "syscall_handler.h"


/** @brief The function to call the user-installed handler if there exists one,
 *         to vanish if there doesn't exist one
 *         
 *  This function will first attmpt to check if there is any user installed 
 *  handler; If so, it will restore the information and build the cur_ureg on 
 *  user stack, then return to user stack and call the handler with 
 *  the built cur_ureg on the userstack
 *
 *  @param cur_ureg, the ureg structure built by the exeption handler wrapper
 *  @return nothing
 */
void get_real_handler(ureg_t* cur_ureg)
{
    lprintf("Ohoh, bad news");
    MAGIC_BREAK;
    cur_ureg->cr2 = get_cr2();
    if (current_thread-> swexn_info.installed_flag==0)
    { 
        sys_set_status(-2);
        printf("Vanish! CPU registers info: cr2: 0x%x, esp0: 0x%x",
                (unsigned int)get_cr2(), (unsigned int)get_esp0());
        /* reap peer threads first before vanishing the thread itself*/
        PCB *current_pcb = current_thread -> pcb;
        list threads = current_pcb -> threads;
        node *n;
        /* display to the console the fault related information */
        for (n = list_begin(&threads); n != NULL; n = n -> next)
        {
            TCB *tcb = list_entry(n, TCB, peer_threads_node);
            /* We only reap threads that is not the current thread */
            if (tcb -> tid != current_thread -> tid)
            {
                list_delete(&threads, n);
                list_delete(&blocked_queue, n);
                list_delete(&runnable_queue, n);
                sfree(tcb -> stack_base, tcb -> stack_size);
                sfree(tcb, sizeof(TCB));
            }
        }
        sys_vanish();
    }

    /* if installed, try to call the real handler; */
    void* new_esp = current_thread -> swexn_info.esp3;
    void* new_eip = current_thread -> swexn_info.eip;
    void* arg = current_thread -> swexn_info.arg;
    void* now_esp;
    
    /* deregister this handler since it will be called;*/
    current_thread -> swexn_info.esp3 = NULL;
    current_thread -> swexn_info.eip = NULL;
    current_thread -> swexn_info.arg = NULL;
    current_thread -> swexn_info.newureg = (ureg_t*)NULL;
    current_thread -> swexn_info.installed_flag = 0;
    current_thread -> swexn_info.eflags = cur_ureg -> eflags;
    
    /* copy the ureg; */
    new_esp = new_esp - (sizeof(ureg_t));
    memcpy(new_esp, cur_ureg, sizeof(ureg_t));
    now_esp = new_esp - 4;
    /* push argument ureg_t* ureg */
    *(uint32_t *)(now_esp) = (uint32_t) new_esp;
    now_esp -= 4;
    /* push void* arg */
    *(uint32_t *)(now_esp) = (uint32_t) arg;
    now_esp -= 4;
    /* let it run, enter ring 3! */
    enter_user_mode(current_thread -> registers.edi,
                    current_thread -> registers.esi,
                    current_thread -> registers.ebp,
                    current_thread -> registers.ebx,
                    current_thread -> registers.edx,
                    current_thread -> registers.ecx,
                    current_thread -> registers.eax,
                    (uint32_t)new_eip,
                    current_thread -> registers.cs,
                    current_thread -> registers.eflags,
                    (uint32_t)now_esp,
                    current_thread -> registers.ss);

    return;  
}
/** @file signal_handler_real.c
 *
 *  @brief This file defines the function to 
 *         call the user defined exception handler
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include <syscall.h>
#include <ureg.h>
#include "control_block.h"
#include "string.h"
#include "datastructure/linked_list.h"
#include "simics.h"
#include <stdint.h>
#include <stddef.h>
#include "process/enter_user_mode.h"
extern void sys_vanish();

 void get_real_signal_handler(ureg_t* cur_ureg) {

 	// If this thread has no swexn handler installed, we simply return
    if (current_thread-> swexn_info.installed_flag==0)
    {
    	return;
    }

	// Get this signal 
    node *node  = list_delete_first(&current_thread -> pending_signals);
    signal_t *s = list_entry(node, signal_t, signal_list_node);

    // Fill in ureg entries
    cur_ureg -> cause = s -> cause;
    cur_ureg -> signaler = s -> signaler;

    // If this signal is SIGKILL, we do ... before vanish
    if (s -> cause == SIGKILL)
    {
    	// do something before we vanish
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
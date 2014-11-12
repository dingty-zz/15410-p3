/** @file exception_handler_real.h
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

extern void sys_vanish(void);


void get_real_handler(ureg_t* cur_ureg)
{
    cur_ureg->cr2 = get_cr2();
    lprintf("getting real handler.........print ureg info that I created:");
    lprintf("eip:%x",(unsigned int)cur_ureg->eip);
    MAGIC_BREAK;
    if (current_thread-> swexn_info.installed_flag==0)
    { 
        lprintf("not registered; fault!!");
        MAGIC_BREAK;
        sys_vanish();
    }
    //if installed, try to call the real handler;
	void* new_esp = current_thread -> swexn_info.esp3;
	void* new_eip = current_thread -> swexn_info.eip;
	void* arg = current_thread -> swexn_info.arg;
    void* now_esp;
    
    //deregister;
    current_thread -> swexn_info.esp3 = NULL;
    current_thread -> swexn_info.eip = NULL;
    current_thread -> swexn_info.arg = NULL;
    current_thread -> swexn_info.newureg = (ureg_t*)NULL;
    current_thread -> swexn_info.installed_flag = 0;
    current_thread -> swexn_info.eflags = cur_ureg -> eflags;
    
    //copy the ureg;
    new_esp = new_esp - (sizeof(ureg_t));
    memcpy(new_esp, cur_ureg, sizeof(ureg_t));
    now_esp = new_esp - 4;
    //push argument ureg_t* ureg
    *(uint32_t *)(now_esp) = (uint32_t) new_esp;
    now_esp -= 4;
    //push void* arg
    *(uint32_t *)(now_esp) = (uint32_t) arg;
    now_esp -= 4;

   	enter_user_mode(current_thread -> registers.edi,     // let it run, enter ring 3!
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
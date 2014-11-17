/**
 * @file enter_user_mode.S
 *
 * @brief This file provids the actual implementation for
 *        a thread entering the user mode by iret
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */
.global enter_user_mode

enter_user_mode:
     

     popl      %eax       # pop return address
     popl      %edi
     popl      %esi
     popl      %ebp
     popl      %ebx
     popl      %edx
     popl      %ecx
     popl      %eax

     pushl     %eax
     movl      $0x2b, %eax
     movl      %eax, %ds
     movl      %eax, %es 
     movl      %eax, %fs 
     movl      %eax, %gs
     popl      %eax         
     
     iret
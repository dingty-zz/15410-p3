.global enter_user_mode

enter_user_mode:
     

     popl      %eax       # return address
     popl      %edi
     popl      %esi
     popl      %ebp
     popl      %ebx
     popl      %edx
     popl      %ecx
     popl      %eax

     pushl     %eax
     movl $0x2b, %eax
     movl %eax, %ds
     movl %eax, %es 
     movl %eax, %fs 
     movl %eax, %gs
     popl      %eax

 
     # pushl $0x2b
     # pushl %ecx  # arbitrary stack pointer
     # pushf     

     # pushl $0x23
     # pushl %ebx             
     
     iret
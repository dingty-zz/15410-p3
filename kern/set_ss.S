.global set_ss
# set_ss:

# 	#mov	0x2b, %ss
# # pushl $0x2e
# pushl 0x2b
# pushl $0xffffffff
	

# pushf
# pushl $0x23
# 	pushl  $0x1000064  # esp


# 	iret

# set_ss:
#       //pushf
# 	//cli
#      //mov $0x2e, %ax 
#      //mov %ax, %ds
#      //mov %ax, %es 
#      //mov %ax, %fs 
#      //mov %ax, %gs 
                    
#      //mov %esp, %eax 
#      //pushl $0x2e 
#      //pushl %eax 
   
#      //pushf 
#      //pushl $0x26
#      //pushl $0x1000064  
#       pushf
#       pushl $0x26
#       pushl $0x1000064
#      ret 
set_ss:
     
     # popl %eax # return address


     movl $0x2b, %eax
     movl %eax, %ds
     movl %eax, %es 
     movl %eax, %fs 
     movl %eax, %gs

 
     pushl $0x2b
     pushl $0xffffff10
     pushf     

     pushl $0x23
     pushl $0x1000064               
          # movl $0xffffffff , %ebp 
     iret
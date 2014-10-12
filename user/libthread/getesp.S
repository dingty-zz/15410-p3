.global getesp
getesp:
	movl	%esp, 	%eax # Return the %esp of the current function 
	ret
#include "push_pop_helper.h"

.global gettid
.global	deschedule
.global yield
.global make_runnable
.global sleep
.global sys_swexn_wrapper

.extern sys_gettid
.extern sys_deschedule
.extern sys_yield
.extern sys_make_runnable
.extern sys_sleep
.extern sys_swexn

yield:

	PUSHREGS

	pushl 	%esi
	call 	sys_yield

	POPREGS

	iret	

gettid:

	PUSHREGS

	call 	sys_gettid

	POPREGS

	iret	

deschedule:

	PUSHREGS

	pushl 	%esi
	call 	sys_deschedule
	popl 	%esi

	POPREGS

	iret	

make_runnable:

	PUSHREGS

	pushl 	%esi
	call 	sys_make_runnable
	popl 	%esi

	POPREGS

	iret	

sleep:

	PUSHREGS

	pushl 	%esi
	call 	sys_sleep
	popl 	%esi

	POPREGS

	iret







//beginning of kernel mode, right after INT
sys_swexn_wrapper:
	PUSHREGS

	pushl 	12(%esi)
	pushl 	8(%esi)
	pushl 	4(%esi)
	pushl 	(%esi)

	call sys_swexn

	popl	%esi
	popl	%esi
	popl 	%esi
	popl	%esi

	//%esi is now newureg
	//either installation unsuccessful or newureg invalid
	test 	%eax, %eax
	jne		end
	//newureg is null
	test 	%esi, %esi
	je 		end

	//swexn succeded to register or deregister and 
	//newureg is also non null
	//use newureg to specify registers
	POPREGS2

	movl 	4(%esi),  %eax
	movl	%eax, 	  %cr2
	movl 	8(%esi),  %eax
	movl	%eax, 	  %ds
	movl 	12(%esi), %eax
	movl	%eax, 	  %es
	movl 	16(%esi), %eax
	movl	%eax, 	  %fs
	movl 	20(%esi), %eax
	movl	%eax, 	  %gs
	movl 	24(%esi), %edi
	movl 	32(%esi), %ebp

	movl 	40(%esi), %ebx
	movl 	44(%esi), %edx
	movl 	48(%esi), %ecx

	//eip, cs, eflags, esp, ss
	movl 	60(%esi), %eax
	movl	%eax,	  (%esp)
	movl 	64(%esi), %eax
	movl	%eax,	  4(%esp)
	movl 	68(%esi), %eax
	movl	%eax,	  8(%esp)
	movl 	72(%esi), %eax
	movl	%eax,	  12(%esp)
	movl 	76(%esi), %eax
	movl	%eax,	  16(%esp)
	
	movl 	52(%esi), %eax
	movl 	28(%esi), %esi

	iret 


	end: 
	POPREGS
	iret
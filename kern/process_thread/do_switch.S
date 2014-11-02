
.extern prepare_init_thread
.global do_switch

do_switch:

	pushl	%ebp
	movl 	%esp,	%ebp
	pusha

	movl	8(%ebp),	%ebx		# This is current TCB
	movl	12(%ebp),	%ecx		# This is next TCB
	movl	16(%ebp),	%edx
	movl	%esp, 		(%ebx)		# current -> esp = %esp

	movl	(%ecx),		%esp		# switch stack to the next TCB's esp
	cmp		$5,			%edx		# if current -> state = THREAD_INIT
	je	 	when_init
	popa
	popl	%ebp
	ret

when_init:
	pushl	%ecx
	call	prepare_init_thread
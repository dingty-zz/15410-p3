/** @file do_switch.S
 *
 *  @brief This file includes the implementation of the life cycle funcitons.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#define THREAD_INIT 	4
.extern prepare_init_thread
.global do_switch

do_switch:

	pushl	%ebp
	movl 	%esp,	%ebp
	pusha
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	movl	8(%ebp),	%ebx		# This is current TCB
	movl	12(%ebp),	%ecx		# This is next TCB
	movl	16(%ebp),	%edx
	movl	%esp, 		(%ebx)		# current -> esp = %esp

	movl	(%ecx),		%esp		# switch stack to the next TCB esp
	cmp		$THREAD_INIT,	%edx	# if current -> state = THREAD_INIT
	je	 	when_init

	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa
	popl	%ebp
	ret

when_init:
	pushl	%ecx
	call	prepare_init_thread
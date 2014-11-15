#include <syscall_int.h>

.global thread_fork
thread_fork:

	# Push old ebp
	pushl	%ebp
	movl 	%esp,	%ebp

	# Save callee saves registers
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	
	# Get arguments for t_fork
	movl	12(%ebp),	%esi		# Get func argument
	movl	16(%ebp),	%edi		# Get arg argument

	# Set ebp and esp to new stack
	movl	%esp,		%ebx		# save old %esp
	movl	8(%ebp),	%esp
	movl	%esp,		%ebp

	# Call thread_fork
	INT 	$THREAD_FORK_INT

	# We know that the child will receive 0 and parent will receive child tid, so 
	# we compare the result and do the branching work
	cmp		$0, 	%eax 			
	jne		parent_end

	# Push arguments on stack
	pushl	%edi
	pushl	%esi

	# Once the stack is created, run the child with func and arg pushed on stack
	# run_child will wait until the child's thread_t data structure is inserted 
	# into the tcb and run child with func(arg)
	call 	run_child	

parent_end:  # Return back to the parent
	movl	%ebx,	%esp	
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret

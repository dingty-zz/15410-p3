
.global exec

.extern _exec

exec:
	# call get_esp0
	# movl %eax, %esp

	pushl 	4(%esi)
	pushl 	(%esi)
	
	movl 	$0x18,	%eax
	movl	%eax,	%ds
	movl	%eax,	%es
	movl	%eax,	%fs
	movl	%eax,	%gs

	# movl	%esp, 	%eax
	# pushl 	60(%eax)
	# pushl	%ebp
	# movl	%esp,	%ebp
	# pushl	%eax

	call _exec

	addl	$0xc, 	%esp
	addl	$0x8,	%esp
	popl	%gs	
	popl	%fs	
	popl	%es	
	popl	%ds
	
	popl	%ebp
	popl	%edi
	popl	%esi
	popl 	%edx
	popl 	%ecx 
	popl 	%ebx

	addl 	$0x4,	%esp

	iret	
.global gettid

.extern _gettid

gettid:
	# call get_esp0
	# movl %eax, %esp

	pushl 	$0

	pusha
	pushl 	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	pushl	$0
	pushl	$72
	movl 	$0x18,	%eax
	movl	%eax,	%ds
	movl	%eax,	%es
	movl	%eax,	%fs
	movl	%eax,	%gs

	movl	%esp, 	%eax
	pushl 	60(%eax)
	pushl	%ebp
	movl	%esp,	%ebp
	pushl	%eax

	call _gettid
	
	addl	$0xc, 	%esp
	addl	$0x8,	%esp
	popl	%gs	
	popl	%fs	
	popl	%es	
	popl	%ds
	popa
	addl 	$0x4,	%esp

	iret	

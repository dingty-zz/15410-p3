

.global gettid

.extern _gettid

gettid:


	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl 	%edx
	pushl 	%ecx 
	pushl 	%ebx

	pushl 	%ds
	pushl 	%es
	pushl 	%fs
	pushl 	%gs

	call 	_gettid

	popl	%gs	
	popl	%fs	
	popl	%es	
	popl	%ds
	
	popl 	%ebx
	popl 	%ecx 
	popl 	%edx
	popl	%esi
	popl	%edi
	popl	%ebp

	iret	

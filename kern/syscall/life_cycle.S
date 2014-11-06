.global fork

.extern _fork

fork:

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

	call 	_fork

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



.global exec

.extern _exec

exec:

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

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	_exec

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

.global thread_fork

.extern sys_thread_fork
	
thread_fork:

	pushl 	%ebx
	pushl 	%ecx 
	pushl 	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	pushl 	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	call sys_thread_fork

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
	iret
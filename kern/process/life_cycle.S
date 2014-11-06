#define 	PUSHREGS	\
			pushl	%ebp	\
			pushl	%edi	\
			pushl	%esi	\
			pushl 	%edx	\
			pushl 	%ecx	\ 
			pushl 	%ebx	\

			pushl 	%ds		\
			pushl 	%es		\
			pushl 	%fs		\
			pushl 	%gs		\

#define 	POPREGS		\
			popl	%gs 	\
			popl	%fs		\
			popl	%es		\
			popl	%ds		\
			
			popl 	%ebx	\
			popl 	%ecx 	\
			popl 	%edx	\
			popl	%esi	\
			popl	%edi	\
			popl	%ebp	\

.global fork
.global exec
.global thread_fork
.global wait
.global vanish
.global sleep

.extern sys_fork
.extern sys_exec
.extern sys_thread_fork
.extern sys_wait
.extern sys_vanish
.extern sys_sleep

fork:
	PUSHREGS

	call 	sys_fork

	POPREGS
	iret	

exec:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_exec

	POPREGS

	iret	
	
thread_fork:

	PUSHREGS
	
	call sys_thread_fork

	POPREGS
	
	iret

wait:

	PUSHREGS

	pushl 	%esi
	call 	sys_wait

	POPREGS

	iret	

vanish:

	PUSHREGS
	call 	sys_vanish

sleep:

	PUSHREGS

	pushl 	%esi
	call 	sys_sleep

	POPREGS

	iret	
#include "push_pop_helper.h"	

.global fork
.global exec
.global thread_fork
.global wait
.global vanish

.extern sys_fork
.extern sys_exec
.extern sys_thread_fork
.extern sys_wait
.extern sys_vanish

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
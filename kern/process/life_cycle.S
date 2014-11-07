#include "push_pop_helper.h"	

.global fork
.global exec
.global thread_fork
.global wait
.global vanish
.global set_status

.extern sys_fork
.extern sys_exec
.extern sys_thread_fork
.extern sys_wait
.extern sys_vanish
.extern sys_set_status

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
	popl 	%esi
	POPREGS

	iret

set_status:

	PUSHREGS

	pushl 	%esi
	call 	sys_set_status
	pushl 	%esi

	POPREGS

	iret	

vanish:

	PUSHREGS
	call 	sys_vanish
#include "push_pop_helper.h"

.global gettid
.global	deschedule
.global yield
.global make_runnable
.global sleep
.global get_ticks

.extern sys_gettid
.extern sys_deschedule
.extern sys_yield
.extern sys_make_runnable
.extern sys_sleep
.extern sys_get_ticks

yield:

	PUSHREGS

	pushl 	%esi
	call 	sys_yield

	POPREGS

	iret	

gettid:

	PUSHREGS

	call 	sys_gettid

	POPREGS

	iret
		
get_ticks:

	PUSHREGS

	call 	sys_get_ticks

	POPREGS

	iret	

deschedule:

	PUSHREGS

	pushl 	%esi
	call 	sys_deschedule

	POPREGS

	iret	

make_runnable:

	PUSHREGS

	pushl 	%esi
	call 	sys_make_runnable

	POPREGS

	iret	

sleep:

	PUSHREGS

	pushl 	%esi
	call 	sys_sleep

	POPREGS

	iret
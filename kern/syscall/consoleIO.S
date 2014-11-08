#include "push_pop_helper.h"

.global readline
.global print
.global get_cursor_pos
.global set_cursor_pos
.global set_term_color

.extern sys_readline
.extern sys_print
.extern sys_get_cursor_pos
.extern sys_set_cursor_pos
.extern sys_set_term_color

readline:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_readline
	popl 	%esi
	popl 	%esi


	POPREGS

	iret	

print:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_print
	popl 	%esi
	popl 	%esi


	POPREGS

	iret	



get_cursor_pos:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_get_cursor_pos
	popl 	%esi
	popl 	%esi


	POPREGS

	iret	




set_cursor_pos:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_set_cursor_pos
	popl 	%esi
	popl 	%esi

	POPREGS

	iret	




set_term_color:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_set_term_color
	popl 	%esi
	popl 	%esi

	POPREGS

	iret	


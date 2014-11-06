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

	POPREGS

	iret	

print:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_print

	POPREGS

	iret	



get_cursor_pos:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_get_cursor_pos

	POPREGS

	iret	




set_cursor_pos:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_set_cursor_pos

	POPREGS

	iret	




set_term_color:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_set_term_color

	POPREGS

	iret	


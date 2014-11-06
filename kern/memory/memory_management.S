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

.global new_pages
.global remove_pages

.extern sys_new_pages
.extern sys_remove_pages

new_pages:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_new_pages

	POPREGS

	iret	



remove_pages:

	PUSHREGS

	pushl 	%esi
	call 	sys_remove_pages

	POPREGS

	iret	
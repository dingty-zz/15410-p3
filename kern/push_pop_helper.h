#ifndef _PUSH_POP_HELPER_H
#define _PUSH_POP_HELPER_H

#define 	PUSHREGS	;\
			pushl	%ebp	;\
			pushl	%edi	;\
			pushl	%esi	;\
			pushl 	%edx	;\
			pushl 	%ecx	;\
			pushl 	%ebx	;\
			pushl 	%ds		;\
			pushl 	%es		;\
			pushl 	%fs		;\
			pushl 	%gs		
#define 	POPREGS		;\
			popl	%gs 	;\
			popl	%fs		;\
			popl	%es		;\
			popl	%ds		;\
			popl 	%ebx	;\
			popl 	%ecx 	;\
			popl 	%edx	;\
			popl	%esi	;\
			popl	%edi	;\
			popl	%ebp	


//pop all the registers without needing them
#define		POPREGS2	;\
			popl	%ebp 	;\
			popl	%ebp	;\
			popl	%ebp	;\
			popl	%ebp	;\
			popl 	%ebp	;\
			popl 	%ebp 	;\
			popl 	%ebp	;\
			popl	%ebp	;\
			popl	%ebp	;\
			popl	%ebp	


#endif /* _PUSH_POP_HELPER_H */
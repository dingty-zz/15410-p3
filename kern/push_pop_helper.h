#ifndef _PUSH_POP_HELPER_H
#define _PUSH_POP_HELPER_H
#include <seg.h>

#define 	PUSHREGS	;\
			pushl	%ebp	;\
			pushl	%edi	;\
			pushl	%esi	;\
			pushl 	%edx	;\
			pushl 	%ecx	;\
			pushl 	%ebx	;\
			pushl   %eax	;\
     		movl $SEGSEL_KERNEL_DS, %eax ;\
     		movl %eax, %ds	;\
     		movl %eax, %es  ;\
     		movl %eax, %fs	;\
     		movl %eax, %gs	;\
     		popl      %eax
#define 	POPREGS		;\
     		pushl   %eax	;\
     		movl $SEGSEL_USER_DS, %eax ;\
     		movl %eax, %ds	;\
     		movl %eax, %es  ;\
     		movl %eax, %fs	;\
     		movl %eax, %gs	;\
     		popl      %eax	;\
			popl 	%ebx	;\
			popl 	%ecx 	;\
			popl 	%edx	;\
			popl	%esi	;\
			popl	%edi	;\
			popl	%ebp	


//pop all the registers without needing them
#define		POPREGS2	;\
			popl	%ebp	;\
			popl 	%ebp	;\
			popl 	%ebp 	;\
			popl 	%ebp	;\
			popl	%ebp	;\
			popl	%ebp	;\
			popl	%ebp	


#endif /* _PUSH_POP_HELPER_H */
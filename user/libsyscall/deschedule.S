#include <syscall_int.h>

.global deschedule

deschedule:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$DESCHEDULE_INT
popl	%esi
popl	%ebp
ret
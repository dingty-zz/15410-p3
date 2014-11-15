#include <syscall_int.h>

.global misbehave

misbehave:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$MISBEHAVE_INT
popl	%esi
popl	%ebp
ret
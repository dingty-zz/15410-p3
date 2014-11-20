#include <syscall_int.h>

.global amask

amask:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
leal	8(%ebp), %esi
INT 	$AMASK_INT
popl	%esi
popl	%ebp
ret
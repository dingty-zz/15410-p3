#include <syscall_int.h>

.global atimer

atimer:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
leal	8(%ebp), %esi
INT 	$ATIMER_INT
popl	%esi
popl	%ebp
ret
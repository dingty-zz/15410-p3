#include <syscall_int.h>

.global yield

yield:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$YIELD_INT
popl	%esi
popl	%ebp
ret
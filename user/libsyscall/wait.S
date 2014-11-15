#include <syscall_int.h>

.global wait

wait:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$WAIT_INT
popl	%esi
popl	%ebp
ret
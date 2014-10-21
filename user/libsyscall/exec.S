#include <syscall_int.h>

.global exec

exec:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$EXEC_INT
popl	%esi
popl	%ebp
ret

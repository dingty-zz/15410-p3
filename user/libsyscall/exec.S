#include <syscall_int.h>

.global exec

exec:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
add		$8,	%ebp
movl	%ebp, %esi
sub		$8, %ebp
INT 	$EXEC_INT
popl	%esi
popl	%ebp
ret
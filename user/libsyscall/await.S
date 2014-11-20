#include <syscall_int.h>

.global await

await:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$AWAIT_INT
popl	%esi
popl	%ebp
ret
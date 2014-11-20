#include <syscall_int.h>

.global asignal

asignal:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
leal	8(%ebp), %esi
INT 	$ASIGNAL_INT
popl	%esi
popl	%ebp
ret

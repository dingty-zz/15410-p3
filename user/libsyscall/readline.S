#include <syscall_int.h>

.global readline

readline:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
add		$8,	%ebp
movl	%ebp, %esi
sub		$8, %ebp
INT 	$READLINE_INT
popl	%esi
popl	%ebp
ret
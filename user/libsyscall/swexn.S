#include <syscall_int.h>

.global swexn

swexn:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
add		$8,	%ebp
movl	%ebp, %esi
sub		$8, %ebp
INT 	$SWEXN_INT
popl	%esi
popl	%ebp
ret
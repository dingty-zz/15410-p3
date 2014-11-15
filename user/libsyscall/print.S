#include <syscall_int.h>

.global print

print:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
add		$8,	%ebp
movl	%ebp, %esi
sub		$8, %ebp
INT 	$PRINT_INT
popl	%esi
popl	%ebp
ret
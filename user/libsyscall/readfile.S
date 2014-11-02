#include <syscall_int.h>

.global readfile

readfile:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
add		$8,	%ebp
movl	%ebp, %esi
sub		$8, %ebp
INT 	$READFILE_INT
popl	%esi
popl	%ebp
ret
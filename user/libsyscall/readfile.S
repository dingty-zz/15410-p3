#include <syscall_int.h>

.global readfile

readfile:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
leal	8(%ebp), %esi
INT 	$READFILE_INT
popl	%esi
popl	%ebp
ret
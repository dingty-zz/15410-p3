#include <syscall_int.h>

.global sleep

sleep:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$SLEEP_INT
popl	%esi
popl	%ebp
ret
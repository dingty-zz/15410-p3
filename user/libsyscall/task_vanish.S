#include <syscall_int.h>

.global task_vanish

task_vanish:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$TASK_VANISH_INT
popl	%esi
popl	%ebp
ret
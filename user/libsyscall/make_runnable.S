#include <syscall_int.h>

.global make_runnable

make_runnable:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$MAKE_RUNNABLE_INT
popl	%esi
popl	%ebp
ret
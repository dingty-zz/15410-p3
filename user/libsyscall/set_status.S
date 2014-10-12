#include <syscall_int.h>

.global set_status

set_status:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$SET_STATUS_INT
popl	%esi
popl	%ebp
ret
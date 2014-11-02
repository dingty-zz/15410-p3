#include <syscall_int.h>

.global set_cursor_pos

set_cursor_pos:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
leal	8(%ebp), %esi
INT 	$SET_CURSOR_POS_INT
popl	%esi
popl	%ebp
ret

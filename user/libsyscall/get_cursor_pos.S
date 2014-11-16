#include <syscall_int.h>

.global get_cursor_pos

get_cursor_pos:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
leal	8(%ebp), %esi
INT 	$GET_CURSOR_POS_INT
popl	%esi
popl	%ebp
ret
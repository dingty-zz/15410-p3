#include <syscall_int.h>

.global set_term_color

set_term_color:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$SET_TERM_COLOR_INT
popl	%esi
popl	%ebp
ret
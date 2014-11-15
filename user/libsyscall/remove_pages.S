#include <syscall_int.h>

.global remove_pages

remove_pages:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
movl	8(%ebp), %esi
INT 	$REMOVE_PAGES_INT
popl	%esi
popl	%ebp
ret
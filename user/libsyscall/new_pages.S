#include <syscall_int.h>

.global new_pages

new_pages:
pushl	%ebp
movl	%esp, %ebp
pushl	%esi
add		$8,	%ebp
movl	%ebp, %esi
sub		$8, %ebp
INT 	$NEW_PAGES_INT
popl	%esi
popl	%ebp
ret